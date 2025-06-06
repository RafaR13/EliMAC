#include "headers/elimac.h"
#include "headers/utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

double test_elimac_csv(FILE *output_file, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t message_length,
                       int tag_bits, int precompute, size_t max_blocks, uint8_t *tag, size_t msg_len, int parallel)
{
    const int iterations = 100;
    clock_t start_time = clock();

    for (int i = 0; i < iterations; i++)
    {
        elimac(key1, key2, message, message_length, tag, tag_bits, precompute, max_blocks, parallel);
    }

    clock_t end_time = clock();
    double time_us = ((double)(end_time - start_time) * 1000000.0) / (CLOCKS_PER_SEC * iterations);

    fprintf(output_file, "%zu,%d,%d,", msg_len, tag_bits, precompute);
    print_tag(output_file, tag, tag_bits);
    fprintf(output_file, ",%.2f\n", time_us);

    return time_us;
}

int main()
{
    // Open output file
    FILE *output_file = fopen("out/elimac_results.csv", "w");
    if (!output_file)
    {
        fprintf(stderr, "Failed to open out/elimac_results.csv\n");
        return 1;
    }

    // CSV header
    fprintf(output_file, "MessageLength,TagBits,Precompute,Tag,TimeUs\n");

    // Seed random
    srand(42);

    // Keys
    uint8_t key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                              0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};

    // Message lengths
    size_t lengths[] = {32, 1024, 100 * 1024};
    int num_lengths = 3;

    // Tag lengths
    int tag_bits[] = {128, 64, 32};
    int num_tags = 3;

    uint8_t tag[BLOCK_SIZE];

    for (int l = 0; l < num_lengths; l++)
    {
        size_t len = lengths[l];
        uint8_t *message = malloc(len);
        generate_random_message(message, len);
        size_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

        for (int t = 0; t < num_tags; t++)
        {
            int tag_len = tag_bits[t];

            // No precomputation
            test_elimac_csv(output_file, key1, key2, message, len, tag_len, 0, 0, tag, len);

            // Precomputation
            test_elimac_csv(output_file, key1, key2, message, len, tag_len, 1, max_blocks, tag, len);
        }

        free(message);
    }

    fclose(output_file);
    return 0;
}