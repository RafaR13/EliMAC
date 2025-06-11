#include "headers/elihash.h"

// 7-round AES-128
void hash_h(const uint8_t *key, uint32_t counter, uint8_t *output,
            const uint8_t *round_keys, const uint8_t *subkeys, int precompute, int variant)
{
    if (precompute && subkeys)
    {
        memcpy(output, subkeys + (counter - 1) * BLOCK_SIZE, BLOCK_SIZE);
    }
    else
    {
        uint8_t counter_bytes[BLOCK_SIZE];
        encode_counter(counter, counter_bytes, variant);
        aes_encrypt(counter_bytes, round_keys, output, 7);
    }
}

// 4-round AES-128
void hash_i(const uint8_t *h_output, const uint8_t *message_block, uint8_t *output,
            const uint8_t *round_keys)
{
    uint8_t input[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        input[i] = h_output[i] ^ message_block[i];
    }
    aes_encrypt(input, round_keys, output, 4);
}

void elihash(uint8_t *state, const uint8_t *key1, size_t num_blocks, uint8_t *round_keys_7,
             uint8_t *subkeys, int precompute, uint8_t *padded, uint8_t *round_keys_4, int parallel, int variant)
{
    if (parallel)
    {
#ifdef _OPENMP
#pragma omp parallel
        {
            uint8_t local_state[BLOCK_SIZE] = {0};
#pragma omp for
            for (size_t i = 0; i < num_blocks - 1; i++)
            {
                uint8_t h_output[BLOCK_SIZE], i_output[BLOCK_SIZE];
                hash_h(key1, i + 1, h_output, round_keys_7, subkeys, precompute, variant);
                hash_i(h_output, padded + i * BLOCK_SIZE, i_output, round_keys_4);
                for (int j = 0; j < BLOCK_SIZE; j++)
                {
                    local_state[j] ^= i_output[j];
                }
            }
#pragma omp critical
            {
                for (int j = 0; j < BLOCK_SIZE; j++)
                {
                    state[j] ^= local_state[j];
                }
            }
        }
#else
        for (size_t i = 0; i < num_blocks - 1; i++)
        {
            uint8_t h_output[BLOCK_SIZE], i_output[BLOCK_SIZE];
            hash_h(key1, i + 1, h_output, round_keys_7, subkeys, precompute, variant);
            hash_i(h_output, padded + i * BLOCK_SIZE, i_output, round_keys_4);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                state[j] ^= i_output[j];
            }
        }
#endif
    }
    else
    {
        for (size_t i = 0; i < num_blocks - 1; i++)
        {
            uint8_t h_output[BLOCK_SIZE], i_output[BLOCK_SIZE];
            hash_h(key1, i + 1, h_output, round_keys_7, subkeys, precompute, variant);
            hash_i(h_output, padded + i * BLOCK_SIZE, i_output, round_keys_4);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                state[j] ^= i_output[j];
            }
        }
    }
}

void precompute_subkeys(const uint8_t *key, uint8_t *subkeys, size_t max_blocks,
                        const uint8_t *round_keys, int variant)
{
    for (uint32_t i = 1; i <= max_blocks; i++)
    {
        uint8_t counter[BLOCK_SIZE];
        encode_counter(i, counter, variant);
        aes_encrypt(counter, round_keys, subkeys + (i - 1) * BLOCK_SIZE, 7);
    }
}