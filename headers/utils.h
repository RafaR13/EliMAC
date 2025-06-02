#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include "elimac.h"
#include <stdio.h>

void aes_key_schedule(const uint8_t *key, uint8_t *round_keys, int rounds);
void aes_encrypt(const uint8_t *input, const uint8_t *round_keys, uint8_t *output, int rounds);
void encode_counter(uint32_t counter, uint8_t *output);
void pad_message(const uint8_t *message, size_t len, uint8_t **padded, size_t *padded_len);
void print_tag(FILE *fp, const uint8_t *tag, int t);
void generate_random_message(uint8_t *message, size_t len);

#endif