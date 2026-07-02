#ifndef UART_H
#define UART_H

#define UART_BUFFER_SIZE 64

void uart_init(unsigned long baud_rate, unsigned long f_master);
void uart_tx_byte(unsigned char data);
void uart_tx_data(unsigned char * data, unsigned char len);
int uart_getc(void);

void UART1_RX_IRQHandler(void) __interrupt(18);
void UART1_TX_IRQHandler(void) __interrupt(17);

#endif // UART_H
