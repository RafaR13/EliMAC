#include "elimac.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifndef MAIN_H
#define MAIN_H

#define ITERATIONS 100

double test_elimac(FILE *fp, const uint8_t *key1, const uint8_t *key2, const uint8_t *message, size_t len,
                   int tag_bits, int precompute, size_t max_blocks, uint8_t *tag, int verbose, int parallel, int variant);
void run_test_suite(FILE *fp, int parallel, int variant);
void run_single_message(FILE *fp, const char *message, int random_keys, int precompute, int tag_bits, int parallel, int variant);

int main(int argc, char *argv[]);

#endif