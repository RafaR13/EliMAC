#include "headers/main.h"

double test_elimac(FILE *fp, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
                   int tag_bits, int precompute, size_t max_blocks, uint8_t *tag, int verbose, int parallel)
{
    clock_t start = clock();

    for (int i = 0; i < ITERATIONS; i++)
    {
        elimac(key1, key2, message, len, tag, tag_bits, precompute, max_blocks, parallel);
    }

    clock_t end = clock();
    double time_us = ((double)(end - start) * 1000000.0) / (CLOCKS_PER_SEC * ITERATIONS);

    if (verbose)
    {
        fprintf(fp, "Tag: ");
        print_tag(fp, tag, tag_bits);
        fprintf(fp, "Time: %.2f us\n", time_us);
    }
    else
    {
        fprintf(fp, "Tag: ");
        print_tag(fp, tag, tag_bits);
    }

    return time_us;
}

void run_test_suite(FILE *fp, int parallel)
{
    srand(42);
    uint8_t fixed_key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t fixed_key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                                    0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    size_t lengths[] = {16, 128, 1024, 10000, 100000, 1000000};
    int num_lengths = 6;
    int tag_bits[] = {128, 96, 64, 32};
    int num_tags = 4;
    uint8_t tag[BLOCK_SIZE];

    for (int l = 0; l < num_lengths; l++)
    {
        size_t len = lengths[l];
        uint8_t *message = malloc(len);
        generate_random_message(message, len);
        size_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

        fprintf(fp, "\nTesting message length: %zu bytes (%zu blocks), Parallel: %s\n",
                len, max_blocks, parallel ? "Yes" : "No");

        for (int t = 0; t < num_tags; t++)
        {
            int tag_len = tag_bits[t];

            fprintf(fp, "\nFixed keys, no precomputation, %d-bit tag:\n", tag_len);
            double time_no_precomp = test_elimac(fp, fixed_key1, fixed_key2, message, len, tag_len, 0, 0, tag, 0, parallel);
            fprintf(fp, "Avg time: %.2f us\n", time_no_precomp);

            fprintf(fp, "\nFixed keys, precomputation (%zu blocks), %d-bit tag:\n", max_blocks, tag_len);
            double time_precomp = test_elimac(fp, fixed_key1, fixed_key2, message, len, tag_len, 1, max_blocks, tag, 0, parallel);
            fprintf(fp, "Avg time: %.2f us\n", time_precomp);
            fprintf(fp, "Speedup: %.2fx\n", time_no_precomp / time_precomp);

            uint8_t random_key1[KEY_SIZE], random_key2[KEY_SIZE];
            generate_random_message(random_key1, KEY_SIZE);
            generate_random_message(random_key2, KEY_SIZE);

            fprintf(fp, "\nRandom keys, no precomputation, %d-bit tag:\n", tag_len);
            time_no_precomp = test_elimac(fp, random_key1, random_key2, message, len, tag_len, 0, 0, tag, 0, parallel);
            fprintf(fp, "Avg time: %.2f us\n", time_no_precomp);

            fprintf(fp, "\nRandom keys, precomputation (%zu blocks), %d-bit tag:\n", max_blocks, tag_len);
            time_precomp = test_elimac(fp, random_key1, random_key2, message, len, tag_len, 1, max_blocks, tag, 0, parallel);
            fprintf(fp, "Avg time: %.2f us\n", time_precomp);
            fprintf(fp, "Speedup: %.2fx\n", time_no_precomp / time_precomp);
        }

        free(message);
    }
}

void run_single_message(FILE *fp, const char *message, int random_keys, int precompute, int tag_bits, int parallel)
{
    srand(42);
    uint8_t key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                              0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    if (random_keys)
    {
        generate_random_message(key1, KEY_SIZE);
        generate_random_message(key2, KEY_SIZE);
    }

    size_t len = strlen(message);
    size_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint8_t tag[BLOCK_SIZE];

    fprintf(fp, "\nRunning EliMAC on message: \"%s\" (%zu bytes, %zu blocks)\n", message, len, max_blocks);
    fprintf(fp, "Random keys: %s, Precomputation: %s, Tag size: %d bits, Parallel: %s\n",
            random_keys ? "Yes" : "No", precompute ? "Yes" : "No", tag_bits, parallel ? "Yes" : "No");
    printf("Tag: ");
    double time_us = test_elimac(fp, key1, key2, (uint8_t *)message, len, tag_bits, precompute, max_blocks, tag, 1, parallel);
    print_tag(stdout, tag, tag_bits);
    printf("Time: %.2f us\n", time_us);
}

int main(int argc, char *argv[])
{
    // Default options
    char *message = "Hello, EliMAC!";
    int random_keys = 0;
    int precompute = 0;
    int tag_bits = 128;
    int parallel = 0;
    int run_test = 0;
    int run_single = 0;

    // Parse arguments
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
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [--test | --run [--message <text>] [--random-keys] [--precompute] [--parallel] [--tag-bits <32|64|96|128>]]\n", argv[0]);
            return 1;
        }
    }

    // Validate mode
    if (run_test + run_single != 1)
    {
        fprintf(stderr, "Specify exactly one of --test or --run\n");
        return 1;
    }

    // Open output file
    FILE *fp = fopen("out/elimac_results.txt", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to open out/elimac_results.txt\n");
        return 1;
    }

    if (run_test)
    {
        fprintf(fp, "Running full test suite\n");
        run_test_suite(fp, parallel);
    }
    else
    {
        run_single_message(fp, message, random_keys, precompute, tag_bits, parallel);
    }

    fclose(fp);
    return 0;
}