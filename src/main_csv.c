#include "headers/elimac.h"
#include "headers/utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <x86intrin.h>

double test_elimac_csv(FILE *output_file, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, uint32_t message_length,
                       int tag_bits, int precompute, uint32_t max_blocks, uint8_t *tag, uint32_t msg_len, int parallel, int random_keys, int encoding)
{
    const int iterations = 1000;
    uint64_t start, end, total_cycles = 0;
    int ret;

    for (int i = 0; i < iterations; i++)
    {
        _mm_lfence(); // Ensure memory operations are complete before timing
        start = __rdtsc();
        ret = elimac(key1, key2, message, message_length, tag, tag_bits, precompute, max_blocks, parallel, encoding);
        _mm_lfence(); // Ensure memory operations are complete after timing
        end = __rdtsc();
        if (ret != 0)
        {
            fprintf(stderr, "EliMAC failed in iteration %d\n", i);
            return -1.0;
        }
        total_cycles += (end - start);
    }

    double cycles_per_byte = (double)total_cycles / (iterations * message_length);
    double time_us = cycles_per_byte * message_length / (3.2e9 / 1e6); // Ryzen 5800H base 3.2 GHz

    fprintf(output_file, "%u;%d;%d;%d;%d;%s;", msg_len, tag_bits, precompute, parallel, random_keys, encoding ? "Compact" : "Naive");
    print_tag(output_file, tag, tag_bits, 0);
    fprintf(output_file, ";%.2f;%.2f\n", time_us, cycles_per_byte);

    return cycles_per_byte;
}

void run_test_suite_csv(FILE *fp, int parallel, int encoding)
{
    srand(42);
    uint8_t fixed_key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t fixed_key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                                    0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    uint32_t lengths[] = {16, 128, 1024, 10000, 100000, 1000000};
    int num_lengths = 6;
    int tag_bits[] = {128, 96, 64, 32};
    int num_tags = 4;
    uint8_t tag[BLOCK_SIZE];

    // Determine encoding range (0, 1, or both)
    int start_encoding = (encoding == 2) ? 0 : encoding;
    int end_encoding = (encoding == 2) ? 2 : encoding + 1;

    for (int enc = start_encoding; enc < end_encoding; enc++)
    {
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

            for (int t = 0; t < num_tags; t++)
            {
                int tag_len = tag_bits[t];

                if (test_elimac_csv(fp, fixed_key1, fixed_key2, message, len, tag_len, 0, 0, tag, len, parallel, 0, enc) < 0)
                {
                    free(message);
                    return;
                }

                if (test_elimac_csv(fp, fixed_key1, fixed_key2, message, len, tag_len, 1, max_blocks, tag, len, parallel, 0, enc) < 0)
                {
                    free(message);
                    return;
                }

                uint8_t random_key1[KEY_SIZE], random_key2[KEY_SIZE];
                generate_random_message(random_key1, KEY_SIZE);
                generate_random_message(random_key2, KEY_SIZE);

                if (test_elimac_csv(fp, random_key1, random_key2, message, len, tag_len, 0, 0, tag, len, parallel, 1, enc) < 0)
                {
                    free(message);
                    return;
                }

                if (test_elimac_csv(fp, random_key1, random_key2, message, len, tag_len, 1, max_blocks, tag, len, parallel, 1, enc) < 0)
                {
                    free(message);
                    return;
                }
            }

            free(message);
        }
    }
}

void run_single_message_csv(FILE *fp, const char *message, int random_keys, int precompute, int tag_bits, int parallel, int encoding)
{
    srand(42);
    uint8_t key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                              0xa6, 0xd2, 0xae, 0x28, 0x16, 0x7e, 0x2b};
    if (random_keys)
    {
        generate_random_message(key1, KEY_SIZE);
        generate_random_message(key2, KEY_SIZE);
    }

    uint32_t len = (uint32_t)strlen(message);
    uint32_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint8_t *tag = NULL;

    // Test single message
    int start_encoding = (encoding == 2) ? 0 : encoding;
    int end_encoding = (encoding == 2) ? 2 : encoding + 1;

    for (int enc = start_encoding; enc < end_encoding; enc++)
    {
        if (test_elimac_csv(fp, key1, key2, (uint8_t *)message, len, tag_bits, precompute, max_blocks, tag, len, parallel, random_keys, enc) < 0)
        {
            fprintf(stderr, "EliMAC failed for single message\n");
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    char *message = "Hello, EliMAC!";
    int random_keys = 0;
    int precompute = 0;
    int tag_bits = 128;
    int parallel = 0;
    int run_test = 0;
    int run_single = 0;
    int encoding = 2; // Default: test both encodings

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
        else if (strcmp(argv[i], "--tag_bits") == 0 && i + 1 < argc)
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
                fprintf(stderr, "Invalid encoding. Use 0 (naive), 1 (Compact), or 2 Both).\n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [--test | --run [--message <text>] [--random-keys] [--precompute] [--parallel] [--tag-bits <32|64|96|128>] [--encoding <0|1|2>]]\n", argv[0]);
            return 1;
        }
    }

    if (run_test + run_single != 1)
    {
        fprintf(stderr, "Error: Specify exactly one of --test or --run\n");
        return -1;
    }

    FILE *fp = fopen("out/elimac_results.csv", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to open output file out/elimac_results.csv\n");
        return -1;
    }

    fprintf(fp, "MessageLength;TagBits;Precompute;Parallel;RandomKeys;Encoding;Tag;TimeUs;CyclesPerByte\n");

    if (run_test)
    {
        run_test_suite_csv(fp, parallel, encoding);
    }
    else
    {
        run_single_message_csv(fp, message, random_keys, precompute, tag_bits, parallel, encoding);
    }

    fclose(fp);
    return 0;
}