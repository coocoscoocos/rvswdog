#include "hex.h"

char nibble_to_hex(uint8_t n)
{
    n &= 0x0F;
    return (n < 10) ? ('0' + n) : ('A' + n - 10);
}

void byte_to_hex(uint8_t byte, char* hex)
{
    hex[0] = nibble_to_hex(byte >> 4);
    hex[1] = nibble_to_hex(byte);
}

uint8_t hex_to_uint32(char* hex, uint32_t *result) {
    *result = 0;
    char* p = hex;
    while (*p) {
        unsigned int digit;
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else if (*p >= 'a' && *p <= 'f') {
            digit = *p - 'a' + 10;
        } else if (*p >= 'A' && *p <= 'F') {
            digit = *p - 'A' + 10;
        } else {
            return 1;
        }
        *result = *result << 4 | digit;
        p++;
    }
    return 0;
}