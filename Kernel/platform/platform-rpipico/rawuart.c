#include <kernel.h>
#include "picosdk.h"
#include "config.h"
#include "core1.h"

#include <pico/critical_section.h>
#include <pico/multicore.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include "rawuart.h"

#if NUM_DEV_TTY_UART != 1
#error "Only one UART is currently supported"
#endif

void rawuart_init(void)
{
    uart_init(uart_default, PICO_DEFAULT_UART_BAUD_RATE);
    gpio_set_function(PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PICO_DEFAULT_UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_translate_crlf(uart_default, false);
    uart_set_fifo_enabled(uart_default, true);
}

void rawuart_putc(uint8_t devn, uint8_t c)
{
    while (!uart_is_writable(uart_default))
    {
        tight_loop_contents();
    }
    uart_putc(uart_default, c);
}

void rawuart_sleeping(uint8_t devn){}

ttyready_t rawuart_ready(uint8_t devn)
{
    return uart_is_writable(uart_default) ? TTY_READY_NOW : TTY_READY_SOON;
}

int rawuart_getc(uint8_t devn)
{
    if (uart_is_readable(uart_default))
    {
        return (int)uart_get_hw(uart_default)->dr;
    }
    return -1;
}