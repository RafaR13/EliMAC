#include "elimac.h"
#include <stdio.h>
#include <string.h>

void print_tag(const uint8_t *tag, int t)
{
    for (int i = 0; i < t / 8; i++)
    {
        printf("%02x", tag[i]);
    }
    printf("\n");
}

int main()
{
    // Sample keys and message
    uint8_t key1[KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key2[KEY_SIZE] = {0x3c, 0x4f, 0xcf, 0x09, 0x88, 0x15, 0xf7, 0xab,
                              0xa6, 0xd2, 0xae, 0x28, 0x16, 0x15, 0x7e, 0x2b};
    uint8_t message[] = "Hello, this is a test message!";
    size_t len = strlen((char *)message);
    uint8_t tag[BLOCK_SIZE];
    int tag_bits = 128;
    size_t max_blocks = 10; // For precomputation

    // Test 1: EliMAC without precomputation
    printf("EliMAC without precomputation:\n");
    elimac(key1, key2, message, len, tag, tag_bits, 0, 0);
    printf("Tag: ");
    print_tag(tag, tag_bits);

    // Test 2: EliMAC with precomputation
    printf("\nEliMAC with precomputation (%zu blocks):\n", max_blocks);
    elimac(key1, key2, message, len, tag, tag_bits, 1, max_blocks);
    printf("Tag: ");
    print_tag(tag, tag_bits);

    // Test 3: EliMAC with shorter tag (64 bits)
    tag_bits = 64;
    printf("\nEliMAC without precomputation (64-bit tag):\n");
    elimac(key1, key2, message, len, tag, tag_bits, 0, 0);
    printf("Tag: ");
    print_tag(tag, tag_bits);

    return 0;
}