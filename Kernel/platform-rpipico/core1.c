/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tusb.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/binary_info.h>
#include <pico/multicore.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include "core1.h"

bool usbconsole_is_readable(void)
{
	return multicore_fifo_rvalid();
}

bool usbconsole_is_writable(void)
{
	return multicore_fifo_wready();
}

uint8_t usbconsole_getc_blocking(void)
{
	return multicore_fifo_pop_blocking();
}

void usbconsole_putc_blocking(uint8_t b)
{
	multicore_fifo_push_blocking(b);
}

static void core1_main(void)
{
    uart_init(uart_default, PICO_DEFAULT_UART_BAUD_RATE);
    gpio_set_function(PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PICO_DEFAULT_UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_translate_crlf(uart_default, false);
    uart_set_fifo_enabled(uart_default, true);

    tusb_init();

	for (;;)
	{
		tud_task();

		if (multicore_fifo_rvalid()
			&& (!tud_cdc_connected() || tud_cdc_write_available())
			&& uart_is_writable(uart_default))
		{
			int b = multicore_fifo_pop_blocking();

			if (tud_cdc_connected())
			{
				tud_cdc_write(&b, 1);
				tud_cdc_write_flush();
			}
			
			uart_putc(uart_default, b);
		}

		if (multicore_fifo_wready()
			&& ((tud_cdc_connected() && tud_cdc_available())
				|| uart_is_readable(uart_default)))
		{
			/* Only service a byte from CDC *or* the UART, in case two show
			 * up at the same time and we end up blocking. No big loss, the
			 * next one will be read the next time around the loop. */

			if (tud_cdc_available())
			{
				uint8_t b;
				int count = tud_cdc_read(&b, 1);
				if (count)
					multicore_fifo_push_blocking(b);
			}
			else if (uart_is_readable(uart_default))
			{
				uint8_t b = uart_get_hw(uart_default)->dr;
				multicore_fifo_push_blocking(b);
			}
		}
	}
}

void core1_init(void)
{
	multicore_launch_core1(core1_main);
}

