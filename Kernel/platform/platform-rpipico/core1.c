/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <kernel.h>
#include "picosdk.h"
#include "config.h"
#include "core1.h"

#define ssize_t __ssize_t
#define time_t __time_t
#include <tusb.h>
#undef ssize_t
#undef time_t

#include <pico/critical_section.h>
#include <pico/multicore.h>
#include <hardware/uart.h>
#include <hardware/irq.h>

#define CONFIG_CONSOLE_BUF_LEN 16

#if CONFIG_CONSOLE_BUF_LEN > 255
#error "Invalid console buffer length"
#endif

struct console_buf_t
{
	uint8_t buffer[CONFIG_CONSOLE_BUF_LEN];
	uint8_t rindex;
	uint8_t windex;
	uint8_t len;
};

struct console_t
{
	struct console_buf_t rbuf;
	struct console_buf_t wbuf;
	bool sleeping;
};

static struct console_t console_table[NUM_DEV_TTY_USB];
critical_section_t critical_section;

static void console_buf_write(struct console_buf_t *buf, uint8_t b)
{
	/*
	can deadlock core1 thread
	while(buf->len >= CONFIG_CONSOLE_BUF_LEN)
		tight_loop_contents();
		*/
	critical_section_enter_blocking(&critical_section);
	if(buf->len < CONFIG_CONSOLE_BUF_LEN)
	{
		buf->buffer[buf->windex] = b;
		buf->windex = (buf->windex + 1) % CONFIG_CONSOLE_BUF_LEN;
		buf->len++;
	}
	critical_section_exit(&critical_section);
}

static uint8_t console_buf_read(struct console_buf_t *buf)
{
	critical_section_enter_blocking(&critical_section);
	if(buf->len == 0)
	{
		panic("rd undrf");
	}
	uint8_t b = buf->buffer[buf->rindex];
	buf->rindex = (buf->rindex + 1) % CONFIG_CONSOLE_BUF_LEN;
	buf->len--;
	critical_section_exit(&critical_section);
	return b;
}

int usbconsole_read(uint8_t *buffer, int size)
{
	if(size & 0x01)
	{
		panic("buf size");
	}
	int written = 0;
	for(int i = 0; i < NUM_DEV_TTY && written < size; i++)
	{
		struct console_buf_t *buf = &console_table[i].rbuf;
		if (buf->len > 0)
		{
			buffer[0] = USB_TO_TTY(i);
			buffer[1] = console_buf_read(buf);
			buffer += 2;
			written += 2;
		}
	}
	return written;
}

extern bool usbconsole_is_writable(uint8_t minor)
{
	minor = TTY_TO_USB(minor);
	return console_table[minor].wbuf.len < CONFIG_CONSOLE_BUF_LEN;
}

extern bool usbconsole_is_available(uint8_t minor)
{
	minor = TTY_TO_USB(minor);
	return minor <= NUM_DEV_TTY_USB;
}

extern void usbconsole_putc(uint8_t minor, uint8_t b)
{
	minor = TTY_TO_USB(minor);
	if (minor >= NUM_DEV_TTY_USB)
	{
		panic("ttydev");
	}
	struct console_buf_t *buf = &console_table[minor].wbuf;
	if (buf->len >= CONFIG_CONSOLE_BUF_LEN)
	{
		panic("ovf");
	}

	console_buf_write(buf, b);
}

extern void usbconsole_setsleep(uint8_t minor, bool sleeping)
{
	console_table[TTY_TO_USB(minor)].sleeping = sleeping;
}

extern bool usbconsole_is_sleeping(uint8_t minor)
{
	return console_table[TTY_TO_USB(minor)].sleeping;
}

#if NUM_DEV_TTY_USB > 0
static void core1_main(void)
{
	tusb_init();

	for (;;)
	{
		tud_task();

		/*
		We ignore tud_cdc_n_connected because if DTR is not set by the host
		all tty's will appear disconnected, which seems to be the case for pi pico
		*/

		for(int i = 0; i < CFG_TUD_CDC; i++)
		{
			if (tud_cdc_n_write_available(i)
				&& console_table[i].wbuf.len)
			{
				uint8_t b = console_buf_read(&console_table[i].wbuf);
				tud_cdc_n_write_char(i, b);
				tud_cdc_n_write_flush(i);
			}
		}

		for (int i = 0; i < CFG_TUD_CDC; i++)
		{
			if (!tud_cdc_n_available(i))
			{
				continue;
			}
			struct console_buf_t *buf = &console_table[i].rbuf;
			if (buf->len < CONFIG_CONSOLE_BUF_LEN)
			{
				uint8_t b = tud_cdc_n_read_char(i);
				console_buf_write(buf, b);
			}
		}
	}
}
#endif

void core1_init(void)
{
	multicore_reset_core1();
	critical_section_init(&critical_section);
#if NUM_DEV_TTY_USB > 0
	multicore_launch_core1(core1_main);
#endif
}
