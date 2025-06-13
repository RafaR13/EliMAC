#include "elimac.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <x86intrin.h>

#define CPU_FREQ 3.2e9 / 1e6
#define ITERATIONS 1000
#define DEFAULT_OUTPUT_FORMAT "csv"
#define SEED 42
#define DEFAULT_MESSAGE "Hello, EliMAC!"

#ifndef MAIN_H
#define MAIN_H

double test_elimac(FILE *output_file, const char *output_format, const uint8_t *key1,
                   const uint8_t *key2, const uint8_t *message, size_t message_length,
                   int tag_bits, int parallel, int precompute, size_t max_blocks, uint8_t *tag, int verbose, int variant);

void run_test_suite(FILE *fp, const char *output_format, int encoding);

void run_single_message(FILE *fp, const char *output_format, const char *message,
                        int random_keys, int precompute, int tag_bits, int parallel, int encoding);

int main(int argc, char *argv[]);

#endif