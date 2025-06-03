#include "elimac.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifndef MAIN_H
#define MAIN_H

double test_elimac(FILE *fp, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
                   int tag_bits, int precompute, size_t max_blocks, uint8_t *tag);

int main();

#endif