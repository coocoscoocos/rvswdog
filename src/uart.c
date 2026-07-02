#include "uart.h"
#include "stm8s.h"
#include "buffer.h"

CIRCLE_BUFFER(tx_buffer, UART_BUFFER_SIZE)
CIRCLE_BUFFER(rx_buffer, UART_BUFFER_SIZE)

void uart_init(const unsigned long baud_rate, const unsigned long f_master)
{
    // unsigned long brr;

    //Настраиваем TX на выход, а RX на вход
    PD_DDR |= 1 << 5; //TX
    PD_DDR &= ~(1 << 6); //RX

    //RX - плавающий вход
    PD_CR1 &= ~(1 << 6);
    //Отключает внешние прерывания для RX
    PD_CR2 &= ~(1 << 6);

    //Настройка скорости передачи
    const unsigned long brr = 138;
    // const unsigned long brr = f_master / baud_rate;

    UART1_BRR2 = brr & 0x000F;
    UART1_BRR2 |= brr >> 12;
    UART1_BRR1 = (brr >> 4) & 0x00FF;

    //Четность отключена
    //Контроль четности отключен
    //8-битный режим
    //Включить UART
    UART1_CR1 = 0;
    UART1_CR2 = 1 << UART1_CR2_RIEN | 1 << UART1_CR2_TEN | 1 << UART1_CR2_REN;

    //Один стоп-бит
    UART1_CR3 = 0;
}

void uart_tx_byte(unsigned char data)
{
    buffer_putc(&tx_buffer, data);
    if (!(UART1_CR2 & (1 << UART1_CR2_TIEN))) {
        UART1_CR2 |= (1 << UART1_CR2_TIEN);
    }
}

// Отправка массива данных
void uart_tx_data(unsigned char* data, unsigned char len)
{
    while (len--)
    {
        uart_tx_byte(*data++);
    }
}

int uart_getc(void)
{
    return buffer_getc(&rx_buffer);
}

void UART1_RX_IRQHandler(void) __interrupt(18)
{
    if (UART1_SR & 1 << UART1_SR_RXNE)
    {
        uint8_t data = UART1_DR;
        buffer_putc(&rx_buffer, data);
        uart_tx_byte(data);
    }
}

void UART1_TX_IRQHandler(void) __interrupt(17) {
    int c = buffer_getc(&tx_buffer);
    if (-1 == c)
    {
        UART1_CR2 &= ~(1 << UART1_CR2_TIEN);
        return;
    }
    UART1_DR = c;
}
