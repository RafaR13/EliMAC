#include "headers/main.h"

double test_elimac(FILE *output_file, const char *output_format, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
                   int tag_bits, int parallel, int precompute, size_t max_blocks, uint8_t *tag, int verbose, int variant)
{
    int ret;
    double result = 0.0;

    // Precompute subkeys and round_keys_7 outside timing loop
    uint8_t *subkeys = NULL;
    uint8_t round_keys_7[KEY_SIZE * 8] = {0};
    if (precompute && max_blocks > 0)
    {
        subkeys = malloc(max_blocks * BLOCK_SIZE);
        if (!subkeys)
        {
            fprintf(stderr, "Memory allocation failed\n");
            return -1.0;
        }
        aes_key_schedule(key1, round_keys_7, 7);
        precompute_subkeys(subkeys, max_blocks, round_keys_7, variant);
    }

    // Warm-up run
    ret = elimac(key1, key2, message, len, tag, tag_bits, precompute, max_blocks, parallel, variant, subkeys, precompute ? round_keys_7 : NULL);
    if (ret != 0)
    {
        if (subkeys)
            free(subkeys);
        fprintf(stderr, "EliMAC warm-up failed\n");
        return -1.0;
    }

    // Timing with rdtsc
    uint64_t start, end, total_cycles = 0;
    for (int i = 0; i < ITERATIONS; i++)
    {
        _mm_lfence();
        start = __rdtsc();
        ret = elimac(key1, key2, message, len, tag, tag_bits, precompute, max_blocks, parallel, variant, subkeys, precompute ? round_keys_7 : NULL);
        _mm_lfence();
        end = __rdtsc();
        if (ret != 0)
        {
            if (subkeys)
                free(subkeys);
            fprintf(stderr, "EliMAC failed in iteration %d\n", i);
            return -1.0;
        }
        total_cycles += (end - start);
    }

    double cycles_per_byte = (double)total_cycles / (ITERATIONS * len);
    double time_us = cycles_per_byte * len / (CPU_FREQ * 1e3); // Convert to µs

    if (strcmp(output_format, "csv") == 0)
    {
        fprintf(output_file, "%zu;%d;%d;%d;%d;%s;0;", len, tag_bits, precompute, parallel, verbose, variant ? "Compact" : "Naive");
        print_tag(output_file, tag, tag_bits, 0);
        fprintf(output_file, ";%.2f;%.2f\n", time_us, cycles_per_byte);
        result = cycles_per_byte;
    }
    else
    {
        fprintf(output_file, "Tag: ");
        print_tag(output_file, tag, tag_bits, 0);
        if (verbose)
            fprintf(output_file, " Time: %.2f us\n", time_us);
        else
            fprintf(output_file, "\n");
        result = time_us;
    }

    if (subkeys)
        free(subkeys);
    return result;
}

void run_test_suite(FILE *output_file, const char *output_format, int parallel, int encoding)
{
    srand(SEED);
    uint8_t fixed_key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t fixed_key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                                    0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    uint32_t lengths[] = {16, 128, 1024, 10000, 100000, 1000000};
    int num_lengths = 6;
    int tag_bits[] = {128, 96, 64, 32};
    int num_tags = 4;
    uint8_t tag[BLOCK_SIZE];

    int start_encoding = (encoding == 2) ? 0 : encoding;
    int end_encoding = (encoding == 2) ? 2 : encoding + 1;

    for (int enc = start_encoding; enc < end_encoding; enc++)
    {
        if (strcmp(output_format, "txt") == 0)
            fprintf(output_file, "\nEncoding: %s\n", enc ? "Compact" : "Naive");
        for (int l = 0; l < num_lengths; l++)
        {
            uint32_t len = lengths[l];
            uint8_t *message = malloc(len);
            if (!message)
            {
                fprintf(stderr, "Memory allocation failed for message\n");
                return;
            }
            generate_random_message(message, len);
            uint32_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

            if (strcmp(output_format, "txt") == 0)
                fprintf(output_file, "\nTesting message length: %u bytes (%u blocks), Parallel: %s\n",
                        len, max_blocks, parallel ? "Yes" : "No");

            for (int t = 0; t < num_tags; t++)
            {
                int tag_len = tag_bits[t];

                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "\nFixed keys, no precomputation, %d-bit tag:\n", tag_len);
                double time_no_precomp = test_elimac(output_file, output_format, fixed_key1, fixed_key2, message, len, tag_len, parallel, 0, 0, tag, 0, enc);
                if (time_no_precomp < 0)
                {
                    free(message);
                    return;
                }
                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "Avg time: %.2f us\n", time_no_precomp);

                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "\nFixed keys, precomputation (%u blocks), %d-bit tag:\n", max_blocks, tag_len);
                double time_precomp = test_elimac(output_file, output_format, fixed_key1, fixed_key2, message, len, tag_len, parallel, 1, max_blocks, tag, 0, enc);
                if (time_precomp < 0)
                {
                    free(message);
                    return;
                }
                if (strcmp(output_format, "txt") == 0)
                {
                    fprintf(output_file, "Avg time: %.2f us\n", time_precomp);
                    fprintf(output_file, "Speedup: %.2fx\n", time_no_precomp / time_precomp);
                }

                uint8_t random_key1[KEY_SIZE], random_key2[KEY_SIZE];
                generate_random_message(random_key1, KEY_SIZE);
                generate_random_message(random_key2, KEY_SIZE);

                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "\nRandom keys, no precomputation, %d-bit tag:\n", tag_len);
                time_no_precomp = test_elimac(output_file, output_format, random_key1, random_key2, message, len, tag_len, parallel, 0, 0, tag, 0, enc);
                if (time_no_precomp < 0)
                {
                    free(message);
                    return;
                }
                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "Avg time: %.2f us\n", time_no_precomp);

                if (strcmp(output_format, "txt") == 0)
                    fprintf(output_file, "\nRandom keys, precomputation (%u blocks), %d-bit tag:\n", max_blocks, tag_len);
                time_precomp = test_elimac(output_file, output_format, random_key1, random_key2, message, len, tag_len, parallel, 1, max_blocks, tag, 0, enc);
                if (time_precomp < 0)
                {
                    free(message);
                    return;
                }
                if (strcmp(output_format, "txt") == 0)
                {
                    fprintf(output_file, "Avg time: %.2f us\n", time_precomp);
                    fprintf(output_file, "Speedup: %.2fx\n", time_no_precomp / time_precomp);
                }
            }
            free(message);
        }
    }
}

void run_single_message(FILE *output_file, const char *output_format, const char *message, int random_keys, int precompute, int tag_bits, int parallel, int encoding)
{
    srand(SEED);
    uint8_t key1[KEY_SIZE], key2[KEY_SIZE];

    if (random_keys)
    { // random keys
        generate_random_message(key1, KEY_SIZE);
        generate_random_message(key2, KEY_SIZE);
    }
    else
    { // fixed keys
        memcpy(key1, (uint8_t[]){0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c}, KEY_SIZE);
        memcpy(key2, (uint8_t[]){0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab, 0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b}, KEY_SIZE);
    }

    uint32_t len = (uint32_t)strlen(message);
    uint32_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint8_t tag[BLOCK_SIZE];

    int start_encoding = (encoding == 2) ? 0 : encoding;
    int end_encoding = (encoding == 2) ? 2 : encoding + 1;

    for (int enc = start_encoding; enc < end_encoding; enc++)
    {
        if (strcmp(output_format, "txt") == 0)
        {
            fprintf(output_file, "\nRunning EliMAC on message: \"%s\" (%u bytes, %u blocks), Encoding: %s\n",
                    message, len, max_blocks, enc ? "Compact" : "Naive");
            fprintf(output_file, "Random keys: %s, Precomputation: %s, Tag size: %d bits, Parallel: %s\n",
                    random_keys ? "Yes" : "No", precompute ? "Yes" : "No", tag_bits, parallel ? "Yes" : "No");
            printf("Tag: ");
        }
        double result = test_elimac(output_file, output_format, key1, key2, (uint8_t *)message, len, tag_bits, parallel, precompute, max_blocks, tag, 1, enc);
        if (result < 0)
        {
            fprintf(stderr, "EliMAC failed for single message\n");
            return;
        }
        if (strcmp(output_format, "txt") == 0)
        {
            print_tag(stdout, tag, tag_bits, 1);
            printf("Time: %.2f us\n", result);
        }
    }
}

int main(int argc, char *argv[])
{
    char *message = DEFAULT_MESSAGE;
    int random_keys = 0;
    int precompute = 0;
    int tag_bits = 128;
    int parallel = 0;
    int run_test = 0;
    int run_single = 0;
    int encoding = 2;
    char *output_format = "csv"; // Default: CSV

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--test") == 0)
        {
            run_test = 1;
        }
        else if (strcmp(argv[i], "--run") == 0)
        {
            run_single = 1;
        }
        else if (strcmp(argv[i], "--message") == 0 && i + 1 < argc)
        {
            message = argv[++i];
        }
        else if (strcmp(argv[i], "--random-keys") == 0)
        {
            random_keys = 1;
        }
        else if (strcmp(argv[i], "--precompute") == 0)
        {
            precompute = 1;
        }
        else if (strcmp(argv[i], "--parallel") == 0)
        {
            parallel = 1;
        }
        else if (strcmp(argv[i], "--tag-bits") == 0 && i + 1 < argc)
        {
            tag_bits = atoi(argv[++i]);
            if (tag_bits != 32 && tag_bits != 64 && tag_bits != 96 && tag_bits != 128)
            {
                fprintf(stderr, "Invalid tag size. Use 32, 64, 96, or 128.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--encoding") == 0 && i + 1 < argc)
        {
            encoding = atoi(argv[++i]);
            if (encoding != 0 && encoding != 1 && encoding != 2)
            {
                fprintf(stderr, "Invalid encoding. Use 0 (Naive), 1 (Compact), or 2 (Both).\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--output-format") == 0 && i + 1 < argc)
        {
            output_format = argv[++i];
            if (strcmp(output_format, "txt") != 0 && strcmp(output_format, "csv") != 0)
            {
                fprintf(stderr, "Invalid output format. Use 'txt' or 'csv'.\n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [--test | --run [--message <text>] [--random-keys] [--precompute] [--parallel] [--tag-bits <32|64|96|128>] [--encoding <0|1|2>] [--output-format <txt|csv>]]\n", argv[0]);
            return 1;
        }
    }

    if (run_test + run_single != 1)
    {
        fprintf(stderr, "Error: Specify exactly one of --test or --run\n");
        return -1;
    }

    char *output_file_name = strcmp(output_format, "txt") == 0 ? "out/elimac_results.txt" : "out/elimac_results.csv";
    FILE *output_file = fopen(output_file_name, "w");
    if (!output_file)
    {
        fprintf(stderr, "Failed to open output file %s\n", output_file_name);
        return -1;
    }

    if (strcmp(output_format, "csv") == 0)
        fprintf(output_file, "MessageLength;TagBits;Precompute;Parallel;RandomKeys;Encoding;TimePrecompute;Tag;TimeUs;CyclesPerByte\n");

    if (run_test)
    {
        printf("Running test suite...\n");
        printf("Output format: %s\n", output_format);
        printf("Parallel: %s\n", parallel ? "Yes" : "No");
        printf("Encoding: %s\n", encoding == 0 ? "Naive" : (encoding == 1 ? "Compact" : "Both"));
        run_test_suite(output_file, output_format, parallel, encoding);
    }
    else
    {
        printf("Running single message test...\n");
        printf("Message: \"%s\"\n", message);
        printf("Random keys: %s\n", random_keys ? "Yes" : "No");
        printf("Precompute: %s\n", precompute ? "Yes" : "No");
        printf("Tag bits: %d\n", tag_bits);
        printf("Parallel: %s\n", parallel ? "Yes" : "No");
        printf("Encoding: %s\n", encoding == 0 ? "Naive" : (encoding == 1 ? "Compact" : "Both"));
        run_single_message(output_file, output_format, message, random_keys, precompute, tag_bits, parallel, encoding);
    }

    fclose(output_file);
    return 0;
}