#ifndef HEX_H
#define HEX_H

#include <stdint.h>

char nibble_to_hex(uint8_t n);
void byte_to_hex(uint8_t byte, char* hex);
uint8_t hex_to_uint32(char* hex, uint32_t *result);

#endif // HEX_H
