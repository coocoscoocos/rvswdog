#include "stm8s.h"
#include "uart.h"
#include "rvswd.h"
#include "util.h"
#include "shell.h"

// HSI 16MHz
void clock_init(void) {
    // wait HSI stable
    while (!(CLK_ICKR & (1 << CLK_ICKR_HSIRDY)));
    CLK_CKDIVR = 0;
}

void main(void) {
    clock_init();
    uart_init(115200, 16000000);
    rvswd_init();

    enable_interrupts();
    print_string("\r\nRVSWDOG\r\n");

    while (1) {
        shell_process();
    }
}
