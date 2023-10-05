#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdarg.h>
#include "iomacros.h"
#include "intrinsics.h"
#include "msp430fr5969.h"

/* Call with interrupts off and on the user stack, and then return straight
 * into the user program. A signal may longjmp out so don't rely on this
 * returning! */
void deliver_signals(void)
{
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
	ei();
}

__interrupt void interrupt_handler(void)
{
	udata.u_ininterrupt = 1;
	plt_interrupt();
	udata.u_ininterrupt = 0;
	deliver_signals();
	/* deliver_signals() leaves interrupts *on*. */
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

