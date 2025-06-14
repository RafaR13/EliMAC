#include "elihash.h"
#include "utils.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef ELIMAC_H
#define ELIMAC_H

#define BLOCK_SIZE 16
#define KEY_SIZE 16
#define MAX_BLOCKS (1ULL << 32)

int elimac(const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
           uint8_t *tag, int t, int precompute, size_t max_blocks, int parallel, int variant, const uint8_t *subkeys, uint8_t *round_keys_7);

#endif