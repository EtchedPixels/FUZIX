/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <kernel.h>
#include <printf.h>
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

static struct console_t usb_console_table[NUM_DEV_TTY_USB];
int usb_host_connected;
critical_section_t critical_section;

static void console_buf_write(struct console_buf_t *buf, uint8_t b)
{
	critical_section_enter_blocking(&critical_section);
	if (buf->len < CONFIG_CONSOLE_BUF_LEN)
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
	uint8_t b = 0;
	if (buf->len != 0)
	{
		b = buf->buffer[buf->rindex];
		buf->rindex = (buf->rindex + 1) % CONFIG_CONSOLE_BUF_LEN;
		buf->len--;
	}
	critical_section_exit(&critical_section);
	return b;
}

int usbconsole_getc(uint8_t usb_num)
{
	int c = -1;
	struct console_buf_t *buf = &usb_console_table[usb_num - 1].rbuf;
	if (buf->len > 0)
	{
		c = console_buf_read(buf);
	}
	return c;
}

extern ttyready_t usbconsole_ready(uint8_t usb_num)
{
	if (usb_console_table[usb_num - 1].wbuf.len < CONFIG_CONSOLE_BUF_LEN)
	{
		return TTY_READY_NOW;
	}
	return TTY_READY_SOON;
}

extern bool usbconsole_is_available(uint8_t usb_num)
{
	return usb_host_connected && usb_num != 0 && usb_num <= NUM_DEV_TTY_USB;
}

extern void usbconsole_putc(uint8_t usb_num, uint8_t c)
{
	while(usbconsole_ready(usb_num) != TTY_READY_NOW)
	{
		tight_loop_contents();
	}
	struct console_buf_t *buf = &usb_console_table[usb_num - 1].wbuf;
	if (buf->len >= CONFIG_CONSOLE_BUF_LEN)
	{
		return;
	}

	console_buf_write(buf, c);
}

extern void usbconsole_sleeping(uint8_t usb_num)
{
	usb_console_table[usb_num - 1].sleeping = true;
}


void tud_mount_cb(void)
{
	usb_host_connected = 1;
}

void tud_umount_cb(void)
{
	usb_host_connected = 0;
}

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

		for (int i = 0; i < CFG_TUD_CDC; i++)
		{
			if (tud_cdc_n_write_available(i) && usb_console_table[i].wbuf.len)
			{
				uint8_t b = console_buf_read(&usb_console_table[i].wbuf);
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
			struct console_buf_t *buf = &usb_console_table[i].rbuf;
			if (buf->len < CONFIG_CONSOLE_BUF_LEN)
			{
				uint8_t b = tud_cdc_n_read_char(i);
				console_buf_write(buf, b);
			}
		}
	}
}

void core1_init(void)
{
	//multicore_reset_core1();
	critical_section_init(&critical_section);
#if NUM_DEV_TTY_USB > 0
	multicore_launch_core1(core1_main);
#endif
}
