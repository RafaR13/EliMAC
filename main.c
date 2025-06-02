#include "main.h"

double test_elimac(FILE *fp, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
                   int tag_bits, int precompute, size_t max_blocks, uint8_t *tag)
{
    const int iterations = 100;
    clock_t start = clock();

    for (int i = 0; i < iterations; i++)
    {
        elimac(key1, key2, message, len, tag, tag_bits, precompute, max_blocks);
    }

    clock_t end = clock();
    double time_us = ((double)(end - start) * 1000000.0) / (CLOCKS_PER_SEC * iterations);

    fprintf(fp, "Tag: ");
    print_tag(fp, tag, tag_bits);

    return time_us;
}

int main()
{
    // Open output file
    FILE *fp = fopen("out/elimac_results.txt", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to open out/elimac_results.txt\n");
        return 1;
    }

    // Seed random for reproducible messages
    srand(42);

    // Sample keys
    uint8_t key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                              0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};

    // Message lengths to test
    size_t lengths[] = {32, 1024, 100 * 1024};
    int num_lengths = 3;

    // Tag lengths to test
    int tag_bits[] = {128, 64, 32};
    int num_tags = 3;

    uint8_t tag[BLOCK_SIZE];

    for (int l = 0; l < num_lengths; l++)
    {
        size_t len = lengths[l];
        uint8_t *message = malloc(len);
        generate_random_message(message, len);
        size_t max_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

        fprintf(fp, "\nTesting message length: %zu bytes\n", len);

        for (int t = 0; t < num_tags; t++)
        {
            int tag_len = tag_bits[t];

            // Test without precomputation
            fprintf(fp, "\nNo precomputation, %d-bit tag:\n", tag_len);
            double time_no_precomp = test_elimac(fp, key1, key2, message, len, tag_len, 0, 0, tag);
            fprintf(fp, "Avg time: %.2f us\n", time_no_precomp);

            // Test with precomputation
            fprintf(fp, "\nPrecomputation (%zu blocks), %d-bit tag:\n", max_blocks, tag_len);
            double time_precomp = test_elimac(fp, key1, key2, message, len, tag_len, 1, max_blocks, tag);
            fprintf(fp, "Avg time: %.2f us\n", time_precomp);
            fprintf(fp, "Speedup: %.2fx\n", time_no_precomp / time_precomp);
        }

        free(message);
    }

    fclose(fp);
    return 0;
}