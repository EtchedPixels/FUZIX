#include <kernel.h>
#include <printf.h>
#include "picosdk.h"
#include "config.h"
#include "core1.h"

#include <pico/critical_section.h>
#include <pico/multicore.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include "rawuart.h"

#if NUM_DEV_TTY_UART > 2
#error "Only two UARTs are supported"
#endif

#ifndef DEV_UART_0_CTS_PIN
#define DEV_UART_0_CTS_PIN 0
#endif
#ifndef DEV_UART_0_RTS_PIN
#define DEV_UART_0_RTS_PIN 0
#endif

#ifndef DEV_UART_1_CTS_PIN
#define DEV_UART_1_CTS_PIN 0
#endif
#ifndef DEV_UART_1_RTS_PIN
#define DEV_UART_1_RTS_PIN 0
#endif

static uint clocks[] = {
    0,      /* B0 */
    50,     /* B50 */
    75,     /* B75 */
    110,    /* B110 */
    134,    /* B134 */
    150,    /* B150 */
    300,    /* B300 */
    600,    /* B600 */
    1200,   /* B1200 */
    2400,   /* B2400 */
    4800,   /* B4800 */
    9600,   /* B9600 */
    19200,  /* B19200 */
    38400,  /* B38400 */
    57600,  /* B57600 */
    115200, /* B115200 */
};

static uart_inst_t * rawuart_init_one(int num, int tx, int rx, int cts, int rts)
{
    uart_inst_t *uart = uart_get_instance(num);
    uart_init(uart, PICO_DEFAULT_UART_BAUD_RATE);
    gpio_set_function(tx, GPIO_FUNC_UART);
    gpio_set_function(rx, GPIO_FUNC_UART);
    uart_set_translate_crlf(uart, false);
    uart_set_fifo_enabled(uart, true);
    if (cts > 0)
    {
        gpio_set_function(DEV_UART_0_CTS_PIN, GPIO_FUNC_UART);
    }
    if (rts > 0)
    {
        gpio_set_function(DEV_UART_0_RTS_PIN, GPIO_FUNC_UART);
    }
    uart_set_hw_flow(uart, cts > 0, rts > 0);
    return uart;
}

static uart_inst_t * rawuart_init(uint_fast8_t uart)
{
    if (uart == 0)
    {
        return rawuart_init_one(0,
                                DEV_UART_0_TX_PIN,
                                DEV_UART_0_RX_PIN,
                                DEV_UART_0_CTS_PIN,
                                DEV_UART_0_RTS_PIN);
    }
    else
    {
        return rawuart_init_one(1,
                                DEV_UART_1_TX_PIN,
                                DEV_UART_1_RX_PIN,
                                DEV_UART_1_CTS_PIN,
                                DEV_UART_1_RTS_PIN);
    }
}

static void rawuart_deinit(uart_inst_t * uart){
    uart_deinit(uart);
}

void rawuart_early_init(void)
{
    // init first uart for kprint
    rawuart_init(0);
}

void rawuart_putc(uint8_t devn, uint8_t c)
{
    uart_inst_t *uart = uart_get_instance(devn - 1);
    while (!uart_is_writable(uart))
    {
        tight_loop_contents();
    }
    uart_putc(uart, c);
}

void rawuart_sleeping(uint8_t devn) {}

ttyready_t rawuart_ready(uint8_t devn)
{
    uart_inst_t *uart = uart_get_instance(devn - 1);
    return uart_is_writable(uart) ? TTY_READY_NOW : TTY_READY_SOON;
}

int rawuart_getc(uint8_t devn)
{
    uart_inst_t *uart = uart_get_instance(devn - 1);
    if (uart_is_readable(uart))
    {
        return (int)uart_get_hw(uart)->dr;
    }
    return -1;
}

static inline bool uart_tx_is_empty(uart_inst_t *uart)
{
    return (uart_get_hw(uart)->fr & UART_UARTFR_TXFE_BITS) != 0;
}

void rawuart_setup(uint_fast8_t minor, uint_fast8_t devn, uint_fast8_t flags)
{
    struct termios *t = &ttydata[minor].termios;

    uart_inst_t *uart = rawuart_init(devn - 1);

    /* Wait for output to finish */
    if (flags)
    {
        while (!uart_tx_is_empty(uart))
            _sched_yield();
    }
    uint baud_rate;
    uint data_bits;
    uint stop_bits = 1;
    uart_parity_t parity = UART_PARITY_NONE;

    speed_t speed = t->c_cflag & CBAUD;
    baud_rate = clocks[speed];
    if (baud_rate == 0) // Hangup if speed is B0 as per stty.1
    {
        rawuart_deinit(uart);
        return;
    }

    if (t->c_cflag & CSTOPB)
    {
        stop_bits = 2;
    }

    data_bits = ((t->c_cflag & CSIZE) >> 4) + 5;
    if (t->c_cflag & PARENB)
    {
        parity = UART_PARITY_EVEN;
    }
    if (t->c_cflag & PARODD)
    {
        parity = UART_PARITY_ODD;
    }
    uart_set_baudrate(uart, baud_rate);
    uart_set_format(uart, data_bits, stop_bits, parity);
}
