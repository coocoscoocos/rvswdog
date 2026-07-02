#include <string.h>

#include "rvswd.h"
#include "util.h"
#include "uart.h"
#include "stm8s.h"

static uint8_t const ch32v20x_readmem[] = {0x90, 0x02, 0x41, 0x88};
static uint8_t const ch32v20x_writemem[] = {0x90, 0x02, 0xc1, 0x88};

void rvswd_io_in(void) {
    PC_DDR &= ~(1 << RVSWD_IO_PIN);
    PC_CR1 &= ~(1 << RVSWD_IO_PIN);
    PC_CR2 &= ~(1 << RVSWD_IO_PIN);
}

void rvswd_io_out(void) {
    PC_DDR |= (1 << RVSWD_IO_PIN);
    // PC_CR1 |= (1 << RVSWD_IO_PIN);
    PC_CR1 &= ~(1 << RVSWD_IO_PIN);
}

void rvswd_set_pin(uint8_t pin, uint8_t value) {
    if (value > 0) {
        PC_ODR |= (1 << pin);
    } else {
        PC_ODR &= ~(1 << pin);
    }
}

void rvswd_init(void) {
    PC_DDR |= (1 << RVSWD_CLK_PIN);
    PC_CR1 |= (1 << RVSWD_CLK_PIN);
}

void rvswd_start(void) {
    rvswd_io_out();
    // Start with both lines high
    rvswd_set_pin(RVSWD_IO_PIN, 1);
    rvswd_set_pin(RVSWD_CLK_PIN, 1);
    delay(10);

    // Pull data low
    rvswd_set_pin(RVSWD_IO_PIN, 0);
    rvswd_set_pin(RVSWD_CLK_PIN, 1);
    delay(10);

    // Pull clock low
    rvswd_set_pin(RVSWD_IO_PIN, 0);
    rvswd_set_pin(RVSWD_CLK_PIN, 0);
    delay(10);
}

void rvswd_stop(void) {
    // Pull data low
    rvswd_set_pin(RVSWD_IO_PIN, 0);
    delay(10);
    rvswd_set_pin(RVSWD_CLK_PIN, 1);
    delay(10);
    // Let data float high
    rvswd_set_pin(RVSWD_IO_PIN, 1);
    delay(10);
}

void rvswd_reset(void) {
    rvswd_set_pin(RVSWD_IO_PIN, 1);
    delay(10);
    for (uint8_t i = 0; i < 100; i++) {
        rvswd_set_pin(RVSWD_CLK_PIN, 0);
        delay(10);
        rvswd_set_pin(RVSWD_CLK_PIN, 1);
        delay(10);
    }
    rvswd_stop();
}

void rvswd_write_bit(uint8_t value) {
    rvswd_set_pin(RVSWD_IO_PIN, value);
    rvswd_set_pin(RVSWD_CLK_PIN, 0);
    rvswd_set_pin(RVSWD_CLK_PIN, 1); // Data is sampled on rising edge of clock
}

uint8_t rvswd_read_bit(void) {
    // rvswd_set_pin(RVSWD_IO_PIN, 1);
    // rvswd_io_in();
    rvswd_set_pin(RVSWD_CLK_PIN, 0);
    delay(10);
    rvswd_set_pin(RVSWD_CLK_PIN, 1); // Data is output on rising edge of clock
    uint8_t value = (PC_IDR >> RVSWD_IO_PIN) & 0x01;
    // rvswd_io_out();
    return value;
}

void rvswd_write(uint8_t reg, uint32_t value) {
    rvswd_start();

    // ADDR HOST
    bool parity = false; // This time it's odd parity?
    for (uint8_t position = 0; position < 7; position++) {
        bool bit = (reg >> (6 - position)) & 1;
        rvswd_write_bit(bit);
        if (bit) parity = !parity;
    }

    // Operation: write
    rvswd_write_bit(1);
    parity = !parity;

    // Parity bit (even)
    rvswd_write_bit(parity);

    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);

    // Data
    parity = false; // This time it's even parity?
    for (uint8_t position = 0; position < 32; position++) {
        bool bit = (value >> (31 - position)) & 1;
        rvswd_write_bit(bit);
        if (bit) parity = !parity;
    }

    // Parity bit
    rvswd_write_bit(parity);

    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);
    rvswd_write_bit(1);
    rvswd_write_bit(1);

    rvswd_stop();
}

bool rvswd_read(uint8_t reg, uint32_t* value) {
    bool parity;

    rvswd_start();

    // ADDR HOST
    parity = false;
    for (uint8_t position = 0; position < 7; position++) {
        bool bit = (reg >> (6 - position)) & 1;
        rvswd_write_bit(bit);
        if (bit) parity = !parity;
    }

    // Operation: read
    rvswd_write_bit(false);

    // Parity bit (even)
    rvswd_write_bit(parity);

    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);

    *value = 0;

    // Data
    rvswd_io_in();
    parity = false;
    uint32_t v = 0;
    for (uint8_t position = 0; position < 32; position++) {
        bool bit = rvswd_read_bit();
        if (bit) {
            v |= ((uint32_t)1) << (31 - position);
        }
        if (bit) parity = !parity;
    }

    *value = v;

    // Parity bit
    bool parity_read = rvswd_read_bit();

    rvswd_io_out();

    rvswd_write_bit(1);
    rvswd_write_bit(0);
    rvswd_write_bit(1);
    rvswd_write_bit(1);
    rvswd_write_bit(1);

    rvswd_stop();

    return parity == parity_read;
}

void ch32v20x_reset_microprocessor_and_run(void) {
    // rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000000);
    // delay(500000);

    // Активируем DM (dmactive=1, без haltreq)
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000001);
    delay(5000);
    // Убеждаемся, что DM отвечает (прочитать любой регистр)
    // Затем подаём haltreq

    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x80000001); // Make the debug module work properly
    delay(50000);
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x80000001); // Initiate a halt request
    delay(50000);
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000001); // Clear the halt request
    delay(50000);
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000003); // Initiate a core reset request
    delay(50000);

    uint8_t timeout = 5;
    while (1) {
        uint32_t value;
        bool r = rvswd_read(CH32_REG_DEBUG_DMSTATUS, &value);
        if (r) {
            uart_tx_data("READ OK\r\n", 9);
        } else {
            uart_tx_data("READ FAIL\r\n", 11);
        }
        uart_tx_data("\r\n", 2);
        print_uint32(value);
        uart_tx_data("\r\n", 2);

        if (((value >> 18) & 0b11) == 0b11) {
            // Check that processor has been reset
            break;
        }
        if (timeout == 0) {
            uart_tx_data("Fail\r\n", 6);
            return;
        }
        timeout--;
        delay(5000);
    }

    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000001); // Clear the core reset request
    delay(5000);
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x10000001); // Clear the core reset status signal
    delay(5000);
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000001); // Clear the core reset status signal clear request
    delay(5000);
    uart_tx_data("done\r\n", 6);
}

void ch32v20x_halt_microprocessor() {
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x80000001); // Make the debug module work properly
    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x80000001); // Initiate a halt request

    // Get the debug module status information, check rdata[9:8], if the value is 0b11,
    // it means the processor enters the halt state normally. Otherwise try again.
    uint8_t timeout = 5;
    while (1) {
        uint32_t value;
        rvswd_read(CH32_REG_DEBUG_DMSTATUS, &value);
        if (((value >> 8) & 0b11) == 0b11) {
            // Check that processor has entered halted state
            break;
        }
        if (timeout == 0) {
            uart_tx_data("Failed to halt microprocessor, DMSTATUS=%", 41);
            print_uint32(value);
            uart_tx_data("\r\n", 2);
            return;
            // return false;
        }
        timeout--;
    }

    rvswd_write(CH32_REG_DEBUG_DMCONTROL, 0x00000001); // Clear the halt request
    uart_tx_data("Microprocessor halted\r\n", 23);
}

void ch32v20x_write_cpu_reg(uint16_t regno, uint32_t value) {
    uint32_t command = (uint32_t)regno // Register to access.
        | ((uint32_t)1 << 16) // Write access.
        | ((uint32_t)1 << 17) // Perform transfer.
        | ((uint32_t)2 << 20) // 32-bit register access.
        | ((uint32_t)0 << 24); // Access register command.

    rvswd_write(CH32_REG_DEBUG_DATA0, value);
    delay(1000);
    rvswd_write(CH32_REG_DEBUG_COMMAND, command);
}

bool ch32v20x_read_cpu_reg(uint16_t regno, uint32_t* value_out) {
    uint32_t command = (uint32_t)regno // Register to access.
        | ((uint32_t)0 << 16) // Read access.
        | ((uint32_t)1 << 17) // Perform transfer.
        | ((uint32_t)2 << 20) // 32-bit register access.
        | ((uint32_t)0 << 24); // Access register command.


    rvswd_write(CH32_REG_DEBUG_COMMAND, command);
    delay(1000);
    rvswd_read(CH32_REG_DEBUG_DATA0, value_out);
    return true;
}

void ch32v20x_run_debug_code(void const* code, size_t code_size) {
    uint32_t status = 0;
    // Copy into program buffer.
    uint32_t tmp[8] = {0};
    memcpy(tmp, code, code_size);
    rvswd_read(CH32_REG_DEBUG_ABSTRACTCS, &status);
    for (size_t i = 0; i < 8; i++) {
        rvswd_write(CH32_REG_DEBUG_PROGBUF0 + i, tmp[i]);
        delay(5000);
    }
    rvswd_read(CH32_REG_DEBUG_ABSTRACTCS, &status);
    // Run program buffer.
    uint32_t command = ((uint32_t)0 << 17) // Do not perform transfer.
        | ((uint32_t)1 << 18) // Run program buffer afterwards.
        | ((uint32_t)2 << 20) // 32-bit register access.
        | ((uint32_t)0 << 24); // Access register command.
    rvswd_write(CH32_REG_DEBUG_COMMAND, command);
    delay(5000);
    rvswd_read(CH32_REG_DEBUG_ABSTRACTCS, &status);
}


void ch32v20x_read_memory_word(uint32_t address, uint32_t* value_out) {
    ch32v20x_write_cpu_reg(CH32_REGS_GPR + 11, address);
    delay(5000);
    ch32v20x_run_debug_code(ch32v20x_readmem, sizeof(ch32v20x_readmem));
    delay(5000);
    ch32v20x_read_cpu_reg(CH32_REGS_GPR + 10, value_out);
}

bool ch32v20x_write_memory_word(uint32_t address, uint32_t value) {
    ch32v20x_write_cpu_reg(CH32_REGS_GPR + 10, value);
    delay(5000);
    ch32v20x_write_cpu_reg(CH32_REGS_GPR + 11, address);
    delay(5000);
    ch32v20x_run_debug_code(ch32v20x_writemem, sizeof(ch32v20x_writemem));
    return true;
}