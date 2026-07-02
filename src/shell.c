#include <string.h>

#include "shell.h"
#include "rvswd.h"
#include "uart.h"
#include "util.h"
#include "hex.h"

//todo добавить shell в название
uint8_t error = 0;
uint8_t current_token = 0;
int cmd_count = 0;
int arg1_count = 0;
int arg2_count = 0;
int* current_count;
int current_limit = 1;
uint8_t cmd_buf[1];
uint8_t* arg_bufs[2];
uint8_t* current_buf;
uint8_t arg1_buf[9];
uint8_t arg2_buf[9];

void cmd_scan(uint8_t *argv[]) {
    //todo ch32v перестает отвечать после прошивки stm8
    print_string("Scan\r\n");
    // uart_tx_data("Scan\r\n", 6);
    ch32v20x_reset_microprocessor_and_run();
    ch32v20x_halt_microprocessor();
    uint32_t mem = 0;
    ch32v20x_read_memory_word(0x1FFFF704, &mem);
    print_string("Chip ID=");
    print_uint32(mem);
    print_string("\r\n");
}

void cmd_read(uint8_t *argv[]) {
    // todo Максимальная и минимальная длина аргумента
    uint32_t addr;
    uint8_t r = hex_to_uint32(argv[0], &addr);
    if (1 == r) {
        print_string("\r\nerror\r\n");
        return;
    }
    uint32_t mem = 0;
    ch32v20x_read_memory_word(addr, &mem);
    print_string("=");
    print_uint32(mem);
}

void cmd_write(uint8_t *argv[]) {
    uint32_t addr;
    uint32_t value;
    uint8_t r = hex_to_uint32(argv[0], &addr);
    if (1 == r) {
        print_string("\r\nerror\r\n");
        return;
    }
    r = hex_to_uint32(argv[1], &value);
    if (1 == r) {
        print_string("\r\nerror\r\n");
        return;
    }
    ch32v20x_write_memory_word(addr, value);
    print_string("ok");
}

struct cmd {
    const uint8_t* cmd;
    uint8_t arg_count;
    void (*func)(uint8_t *argv[]);
};

const struct cmd cmds[] = {
    {"s", 0, cmd_scan},
    {"r", 1, cmd_read},
    {"w", 2, cmd_write},
    {nullptr, 0, nullptr}
};

void reset_shell() {
    error = 0;
    current_token = 0;
    cmd_count = 0;
    arg1_count = 0;
    arg2_count = 0;
    current_count = &cmd_count;
    current_limit = 1;
    cmd_buf[0] = 0;
    current_buf = cmd_buf;
    arg_bufs[0] = arg1_buf;
    arg_bufs[1] = arg2_buf;
    memset(arg1_buf, 0, sizeof(arg1_buf));
    memset(arg2_buf, 0, sizeof(arg2_buf));
}

void shell_process(void) {
    uart_tx_byte('>');
    reset_shell();
    while (true) {
        int c = uart_getc();
        switch (c) {
        case -1:
            continue;
        case '\r':
        case '\n':
            if (error) {
                uart_tx_data("\r\nerror\r\n", 9);
            } else if (cmd_count > 0) {
                uint8_t found_cmd = 0;
                for (const struct cmd *cmd = cmds; cmd->cmd != nullptr; cmd++) {
                    if (cmd->cmd[0] == cmd_buf[0]) {
                        uart_tx_data("\r\n", 2);
                        cmd->func(arg_bufs);
                        found_cmd = 1;
                    }
                }
                if (!found_cmd) {
                    uart_tx_data("\r\nerror\r\n", 9);
                }
            }
            uart_tx_data("\r\n", 2);
            uart_tx_byte('>');
            reset_shell();
            break;
        case 0x20:
            if (error) {
                break;
            }
            if (current_token == 0 && *current_count > 0) {
                current_token = 1;
                current_count = &arg1_count;
                current_buf = arg_bufs[0];
                current_limit = 8;
            } else if (current_token == 1 && *current_count > 0) {
                current_token = 2;
                current_count = &arg2_count;
                current_buf = arg_bufs[1];
            }
            break;
        default:
            if (error) {
                break;
            }
            if (*current_count < current_limit) {
                current_buf[*current_count] = (uint8_t)c;
                *current_count += 1;
            } else {
                error = 1;
            }
        }
    }
}