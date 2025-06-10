#include "headers/elimac.h"
#include "headers/utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

double test_elimac_csv(FILE *output_file, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, uint32_t message_length,
                       int tag_bits, int precompute, uint32_t max_blocks, uint8_t *tag, uint32_t msg_len, int parallel, int random_keys)
{
    const int iterations = 1000; // Match main.c ITERATIONS
    clock_t start_time = clock();
    int ret;

    for (int i = 0; i < iterations; i++)
    {
        ret = elimac(key1, key2, message, message_length, tag, tag_bits, precompute, max_blocks, parallel);
        if (ret != 0)
        {
            fprintf(stderr, "EliMAC failed in iteration %d\n", i);
            return -1.0; // Error handling
        }
    }

    clock_t end_time = clock();
    double time_us = ((double)(end_time - start_time) * 1000000.0) / (CLOCKS_PER_SEC * iterations);

    fprintf(output_file, "%u;%d;%d;%d;%d;", msg_len, tag_bits, precompute, parallel, random_keys);
    print_tag(output_file, tag, tag_bits, 0);
    fprintf(output_file, ";%.2f\n", time_us);

    return time_us;
}

void run_test_suite_csv(FILE *fp, int parallel)
{
    srand(42);
    uint8_t fixed_key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t fixed_key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                                    0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    uint32_t lengths[] = {16, 128, 1024, 10000, 100000, 1000000}; // Match main.c test suite
    int num_lengths = 6;
    int tag_bits[] = {128, 96, 64, 32}; // Match main.c
    int num_tags = 4;
    uint8_t tag[BLOCK_SIZE];

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

            // Fixed keys, no precomputation
            if (test_elimac_csv(fp, fixed_key1, fixed_key2, message, len, tag_len, 0, 0, tag, len, parallel, 0) < 0)
            {
                free(message);
                return;
            }

            // Fixed keys, precomputation
            if (test_elimac_csv(fp, fixed_key1, fixed_key2, message, len, tag_len, 1, max_blocks, tag, len, parallel, 0) < 0)
            {
                free(message);
                return;
            }

            // Random keys
            uint8_t random_key1[KEY_SIZE], random_key2[KEY_SIZE];
            generate_random_message(random_key1, KEY_SIZE);
            generate_random_message(random_key2, KEY_SIZE);

            // Random keys, no precomputation
            if (test_elimac_csv(fp, random_key1, random_key2, message, len, tag_len, 0, 0, tag, len, parallel, 1) < 0)
            {
                free(message);
                return;
            }

            // Random keys, precomputation
            if (test_elimac_csv(fp, random_key1, random_key2, message, len, tag_len, 1, max_blocks, tag, len, parallel, 1) < 0)
            {
                free(message);
                return;
            }
        }

        free(message);
    }
}

void run_single_message_csv(FILE *fp, const char *message, int random_keys, int precompute, int tag_bits, int parallel)
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

    uint32_t len = (uint32_t)strlen(message);
    uint32_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint8_t tag[BLOCK_SIZE];

    double time_us = test_elimac_csv(fp, key1, key2, (uint8_t *)message, len, tag_bits, precompute, max_blocks, tag, len, parallel, random_keys);
    if (time_us < 0)
    {
        fprintf(stderr, "EliMAC failed for single message\n");
        return;
    }
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
    FILE *fp = fopen("out/elimac_results.csv", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to open out/elimac_results.csv\n");
        return 1;
    }

    // CSV header
    fprintf(fp, "MessageLength;TagBits;Precompute;Parallel;RandomKeys;Tag;TimeUs\n");

    if (run_test)
    {
        run_test_suite_csv(fp, parallel);
    }
    else
    {
        run_single_message_csv(fp, message, random_keys, precompute, tag_bits, parallel);
    }

    fclose(fp);
    return 0;
}