#include "util.h"
#include "hex.h"
#include "uart.h"

void print_uint32(uint32_t value) {
    unsigned char str[8];
    byte_to_hex((unsigned char)(value >> 24) & 0xff, str);
    byte_to_hex((unsigned char)(value >> 16) & 0xff, str + 2);
    byte_to_hex((unsigned char)(value >> 8) & 0xff, str + 4);
    byte_to_hex((unsigned char)(value & 0xff), str + 6);
    uart_tx_data(str, sizeof(str));
}

void print_uint32_binary(uint32_t value) {
    for (uint8_t position = 31; position > 0; position--) {
        bool bit = (value >> position) & 1;
        if (bit) {
            uart_tx_byte('1');
        } else {
            uart_tx_byte('0');
        }
    }
}

void print_byte(unsigned char value) {
    unsigned char str[2];
    byte_to_hex(value, str);
    uart_tx_data(str, sizeof(str));
}

void print_nibble(unsigned char nibble) {
    char c = nibble_to_hex(nibble);
    uart_tx_byte(c);
    uart_tx_data("\r\n", 2);
}

void print_string(char *str) {
    while (*str != 0) {
        uart_tx_byte(*str);
        str++;
    }
}


void delay(unsigned long cnt) {
    for (unsigned long i = 0; i <= cnt; i++) {
        __asm__("nop");
    }
}