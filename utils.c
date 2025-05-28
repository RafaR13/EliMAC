#include "utils.h"
#include <immintrin.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const uint8_t rcon[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
static const uint8_t sbox[256] = {/* Fill with standard AES S-box */};

void aes_key_schedule(const uint8_t *key, uint8_t *round_keys, int rounds) {
    memcpy(round_keys, key, 16);
    uint8_t temp[4];
    for (int i = 4; i < 4 * (rounds + 1); i++) {
        memcpy(temp, round_keys + (i - 1) * 4, 4);
        if (i % 4 == 0) {
            uint8_t t = temp[0];
            temp[0] = sbox[temp[1]] ^ rcon[i / 4 - 1];
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[t];
        }
        for (int j = 0; j < 4; j++) {
            round_keys[i * 4 + j] = round_keys[(i - 4) * 4 + j] ^ temp[j];
        }
    }
}

void aes_encrypt(const uint8_t *input, const uint8_t *round_keys, uint8_t *output, int rounds) {
    __m128i state = _mm_loadu_si128((__m128i *)input);
    state = _mm_xor_si128(state, _mm_loadu_si128((__m128i *)round_keys));
    for (int i = 1; i < rounds; i++) {
        state = _mm_aesenc_si128(state, _mm_loadu_si128((__m128i *)(round_keys + i * 16)));
    }
    state = _mm_aesenclast_si128(state, _mm_loadu_si128((__m128i *)(round_keys + rounds * 16)));
    _mm_storeu_si128((__m128i *)output, state);
}

void encode_counter(uint32_t counter, uint8_t *output) {
    if (counter < 1 || counter > MAX_BLOCKS) {
        fprintf(stderr, "Counter out of range\n");
        exit(1);
    }
    uint8_t counter_bytes[4] = {
        (counter >> 24) & 0xFF, (counter >> 16) & 0xFF, (counter >> 8) & 0xFF, counter & 0xFF
    };
    for (int i = 0; i < 4; i++) {
        memcpy(output + i * 4, counter_bytes, 4);
    }
}

void pad_message(const uint8_t *message, size_t len, uint8_t **padded, size_t *padded_len) {
    size_t padding_len = BLOCK_SIZE - (len % BLOCK_SIZE);
    if (padding_len == BLOCK_SIZE) padding_len = 0;
    *padded_len = len + padding_len;
    *padded = malloc(*padded_len);
    memcpy(*padded, message, len);
    if (padding_len > 0) {
        (*padded)[len] = 0x80;
        memset((*padded) + len + 1, 0, padding_len - 1);
    }
}