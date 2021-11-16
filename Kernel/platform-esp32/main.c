#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "soc/gpio_struct.h"
#include "soc/timer_group_struct.h"
#include "soc/rtc_cntl_struct.h"
#include "soc/uart_struct.h"

#define LED 2

extern unsigned int _sbss, _ebss, _sidata, _sdata, _edata;

volatile int b = '0';
volatile int v;

static void putc(char c)
{
	uart_dev_t* uart = &UART0;
	while (!uart->int_raw.txfifo_empty)
		;
	uart->fifo.val = c;
	while (!uart->int_raw.txfifo_empty)
		;
	
}

void __attribute__((noreturn)) __pro_cpu_main(void) {
	bool state = true;
	TIMERG0.wdtconfig0.val = 0;
	TIMERG1.wdtconfig0.val = 0;
	RTCCNTL.wdt_config0.val = 0;

	gpio_dev_t* gpio = &GPIO;

	gpio->func_out_sel_cfg[LED].func_sel = 0x100;
	gpio->enable_w1ts = 1<<LED;
	
	for (;;)
	{
		putc(b + state);
		if (state)
			gpio->out_w1ts = 1<<LED;
		else
			gpio->out_w1tc = 1<<LED;

		state = !state;

		
		v = 1000000;
		while (v--)
			;
	}
}
