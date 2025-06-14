#include "headers/utils.h"

static const uint8_t rcon[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

int aes_key_schedule(const uint8_t *key, uint8_t *round_keys, int rounds)
{
    if (rounds < 4 || rounds > 10)
    {
        fprintf(stderr, "Rounds must be between 4 and 10\n");
        return -1;
    }
    if (!key || !round_keys)
    {
        fprintf(stderr, "Null pointer in key schedule\n");
        return -1;
    }
    memcpy(round_keys, key, 16);
    uint8_t temp[4];
    for (int i = 4; i < 4 * (rounds + 1); i++)
    {
        memcpy(temp, round_keys + (i - 1) * 4, 4);
        if (i % 4 == 0)
        {
            uint8_t t = temp[0];
            temp[0] = sbox[temp[1]] ^ rcon[i / 4 - 1];
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[t];
        }
        for (int j = 0; j < 4; j++)
        {
            round_keys[i * 4 + j] = round_keys[(i - 4) * 4 + j] ^ temp[j];
        }
    }
    return 0;
}

void aes_encrypt(const uint8_t *input, const uint8_t *round_keys, uint8_t *output, int rounds)
{
    if (!input || !round_keys || !output)
    {
        fprintf(stderr, "Null pointer in AES encrypt\n");
        return;
    }
    __m128i state = _mm_loadu_si128((__m128i *)input);
    state = _mm_xor_si128(state, _mm_loadu_si128((__m128i *)round_keys));
    for (int i = 1; i < rounds; i++)
    {
        state = _mm_aesenc_si128(state, _mm_loadu_si128((__m128i *)(round_keys + i * 16)));
    }
    state = _mm_aesenclast_si128(state, _mm_loadu_si128((__m128i *)(round_keys + rounds * 16)));
    _mm_storeu_si128((__m128i *)output, state);
}

int encode_counter(uint32_t counter, uint8_t *output, int variant)
{
    if (counter < 1 || counter > 0xFFFFFFFF) // TODO: check this counter range
    {
        fprintf(stderr, "Counter out of range [1, 2^32]\n");
        return -1;
    }
    if (!output)
    {
        fprintf(stderr, "Null output in encode_counter\n");
        return -1;
    }

    uint8_t counter_bytes[4] = {
        (counter >> 24) & 0xFF, (counter >> 16) & 0xFF, (counter >> 8) & 0xFF, counter & 0xFF};

    switch (variant)
    {
    case 0: // Naive: Repeat 32-bit counter four times
        for (int i = 0; i < 4; i++)
        {
            memcpy(output + i * 4, counter_bytes, 4);
        }
        break;
    case 1: // Compact: 32-bit counter in last 4 bytes, 96 zero bits
        memset(output, 0, 12);
        memcpy(output + 12, counter_bytes, 4);
        break;
    case 2:
        memset(output, 0, 12);
        memcpy(output + 12, &counter_bytes, 4);
        break;
    case 3:
        memset(output, 0, 12);
        *((uint32_t *)(output + 12)) = counter; // Assumes little-endian + aligned
        break;
    default:
        fprintf(stderr, "Invalid variant %d in encode_counter\n", variant);
        return -1;
    }

    return 1;
}

int pad_message(const uint8_t *message, size_t len, uint8_t **padded, size_t *padded_len)
{
    if (!message || !padded || !padded_len)
    {
        fprintf(stderr, "Null pointer in pad_message\n");
        return -1;
    }
    size_t remainder = len % BLOCK_SIZE;
    *padded_len = len + (remainder == 0 ? BLOCK_SIZE : BLOCK_SIZE - remainder);
    *padded = malloc(*padded_len);
    if (!*padded)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    memcpy(*padded, message, len);
    (*padded)[len] = 0x80;
    memset((*padded) + len + 1, 0, *padded_len - len - 1);
    return 0;
}

void print_tag(FILE *fp, const uint8_t *tag, int t, int isOutput)
{
    if (!fp || !tag)
        return;
    for (int i = 0; i < t / 8; i++)
    {
        fprintf(fp, "%02x", tag[i]);
    }
    if (isOutput)
        fprintf(fp, "\n");
}

void generate_random_message(uint8_t *message, size_t len)
{
    if (!message || len == 0)
        return;
    for (size_t i = 0; i < len; i++)
    {
        message[i] = (uint8_t)(rand() & 0xFF);
    }
}