#include "buffer.h"

uint8_t buffer_get_next_head(struct circle_buffer *buffer)
{
    uint8_t next_head = buffer->head + 1;
    if (next_head >= buffer->size)
    {
        next_head = 0;
    }
    return next_head;
}

uint8_t buffer_get_next_tail(struct circle_buffer *buffer)
{
    uint8_t next_tail = buffer->tail + 1;
    if (next_tail >= buffer->size)
    {
        next_tail = 0;
    }
    return next_tail;
}

void buffer_putc(struct circle_buffer *buffer, uint8_t c)
{
    buffer->buffer[buffer->head] = c;
    buffer->head = buffer_get_next_head(buffer);
}

int buffer_getc(struct circle_buffer *buffer)
{
    if (buffer->tail == buffer->head)
    {
        return -1;
    }
    uint8_t data = buffer->buffer[buffer->tail];
    buffer->tail = buffer_get_next_tail(buffer);
    return data;
}