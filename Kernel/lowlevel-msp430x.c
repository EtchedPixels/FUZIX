#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdarg.h>
#include "iomacros.h"
#include "intrinsics.h"
#include "msp430fr5969.h"

__interrupt void interrupt_handler(void)
{
	platform_interrupt();
}

void doexec(uaddr_t start_addr)
{
	di();
	udata.u_insys = 0;

	asm volatile(
		"mov %0, sp\n"
		"eint\n"
		"br %1\n"
		:
		: "g" (udata.u_isp),
		  "g" (start_addr));
}

