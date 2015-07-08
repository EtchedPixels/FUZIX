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
	if (!udata.u_insys)
	{
		uint8_t cursig = udata.u_cursig;
		if (cursig)
		{
			typedef int (*sigvec_fn)(int);
			sigvec_fn fn = udata.u_sigvec[cursig];

			/* Semantics for now: signal delivery clears handler. */
			udata.u_sigvec[cursig] = 0;
			udata.u_cursig = 0;

			/* Run the handler with interrupts *on* (because the user code
			 * might want to longjmp out). Yes, this means we have reentrant
			 * interrupts. */
			udata.u_insys = 0;
			ei();
			fn(cursig);
		}
	}
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

