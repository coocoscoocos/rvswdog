#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

struct circle_buffer
{
    volatile uint8_t *buffer;
    uint8_t size;
    volatile uint8_t head;
    volatile uint8_t tail;
};

#define CIRCLE_BUFFER(name, size)              \
    static volatile uint8_t name##_data[size]; \
    static struct circle_buffer name = {       \
        name##_data,                           \
        size,                                  \
        0,                                     \
        0                                      \
    };

uint8_t buffer_get_next_head(struct circle_buffer *buffer);
uint8_t buffer_get_next_tail(struct circle_buffer *buffer);
void buffer_putc(struct circle_buffer *buffer, uint8_t c);
int buffer_getc(struct circle_buffer *buffer);

#endif // BUFFER_H
