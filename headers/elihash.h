#ifndef ELIHASH_H
#define ELIHASH_H

#include <stdint.h>
#include <stddef.h>
#include "utils.h"
#include "elimac.h"

void hash_h(const uint8_t *key, uint32_t counter, uint8_t *output,
            const uint8_t *round_keys, const uint8_t *subkeys, int precompute);

void hash_i(const uint8_t *h_output, const uint8_t *message_block, uint8_t *output,
            const uint8_t *round_keys);

void elihash(uint8_t *state, const uint8_t *key1, size_t num_blocks, uint8_t *round_keys_7,
             uint8_t *subkeys, int precompute, uint8_t *padded, uint8_t *round_keys_4);

void precompute_subkeys(const uint8_t *key, uint8_t *subkeys, size_t max_blocks,
                        const uint8_t *round_keys);

#endif