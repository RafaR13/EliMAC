#include "headers/elimac.h"

int elimac(const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
           uint8_t *tag, int t, int precompute, size_t max_blocks, int parallel, int variant)
{
    if (t > 128 || t < 0)
    {
        fprintf(stderr, "Tag length must be <= 128 bits\n");
        return -1;
    }

    // Pad message
    uint8_t *padded;
    size_t padded_len;
    pad_message(message, len, &padded, &padded_len);
    size_t num_blocks = padded_len / BLOCK_SIZE;
    if (num_blocks > MAX_BLOCKS)
    {
        free(padded);
        fprintf(stderr, "Message too long\n");
        return -1;
    }

    // Key schedules (basically generates round keys for AES)
    uint8_t round_keys_7[KEY_SIZE * 8], round_keys_4[KEY_SIZE * 5], round_keys_10[KEY_SIZE * 11];
    aes_key_schedule(key1, round_keys_7, 7);
    aes_key_schedule(key1, round_keys_4, 4);
    aes_key_schedule(key2, round_keys_10, 10);

    // Precompute subkeys
    uint8_t *subkeys = NULL;
    if (precompute && max_blocks > 0)
    {
        subkeys = malloc(max_blocks * BLOCK_SIZE);
        if (!subkeys)
        {
            free(padded);
            fprintf(stderr, "Memory allocation failed\n");
            return -1;
        }
        precompute_subkeys(key1, subkeys, max_blocks, round_keys_7, variant);
    }

    // Initialize state
    uint8_t state[BLOCK_SIZE] = {0};

    // process blocks 1 to l-1
    elihash(state, key1, num_blocks, round_keys_7, subkeys, precompute, padded, round_keys_4, parallel, variant);

    // Last block: S = S XOR M_l
    if (num_blocks > 0)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            state[j] ^= padded[(num_blocks - 1) * BLOCK_SIZE + j];
        }
    }

    // Finally: T = E_K2(S)
    // 10-round AES-128
    uint8_t final[BLOCK_SIZE];
    aes_encrypt(state, round_keys_10, final, 10);

    // copy tag (and truncate if necessary)
    memcpy(tag, final, t / 8);

    free(padded);
    if (subkeys)
        free(subkeys);
    return 0;
}