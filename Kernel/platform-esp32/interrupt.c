#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <blkdev.h>
#include <tty.h>
#include <devtty.h>
#include <dev/devsd.h>
#include <printf.h>
#include "globals.h"

/*
 *	Synchronous exceptions. We display the fault details and for user space then dequeue any
 *	pending asynchronous signal and return a signal to deliver. Some of the handling here
 *	is the same in several ports (eg lib/68000exception.c) so probably wants to be extracted
 *	in a more generic form.
 */
unsigned int exception_handler(void *ef, uint32_t cause)
{
	panic("exception");
}

void timer_init(void)
{
}

static uint32_t irqmask;	/* A set bit is an enable */

static void irq_clear(uint32_t irq)
{
	asm volatile ("wsr.intclear %0; esync" :: "a" (irq));
}

void irq_enable(uint32_t irq)
{
	irqmask |= 1 << irq;
	//irq_set_mask(irqmask);
}

void irq_disable(uint32_t irq)
{
	irqmask &= ~(1 << irq);
	//irq_set_mask(irqmask);
}

void interrupt_handler(uint32_t interrupt)
{
}
