#ifndef ELIHASH_H
#define ELIHASH_H

#include <stdint.h>
#include <stddef.h>
#include "utils.h"
#include "elimac.h"
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

void hash_h(uint32_t counter, uint8_t *output,
            const uint8_t *round_keys, const uint8_t *subkeys, int precompute, int variant);

void hash_i(const uint8_t *h_output, const uint8_t *message_block, uint8_t *output,
            const uint8_t *round_keys);

void elihash(uint8_t *state, size_t num_blocks, uint8_t *round_keys_7,
             const uint8_t *subkeys, int precompute, uint8_t *padded, uint8_t *round_keys_4, int parallel, int variant);

void precompute_subkeys(uint8_t *subkeys, size_t max_blocks,
                        const uint8_t *round_keys, int variant);

#endif