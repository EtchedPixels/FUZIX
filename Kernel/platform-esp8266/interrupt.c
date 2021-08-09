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
#include "rom.h"

/*
 *	Synchronous exceptions. We display the fault details and for user space then dequeue any
 *	pending asynchronous signal and return a signal to deliver. Some of the handling here
 *	is the same in several ports (eg lib/68000exception.c) so probably wants to be extracted
 *	in a more generic form.
 */
unsigned int exception_handler(struct exception_frame *ef, uint32_t cause)
{
	uint_fast8_t s = 0;
	uint16_t m;
	ptptr proc = udata.u_ptab;

	kprintf(" _---,_   FATAL EXCEPTION %d @ %p with %p:\n", cause, ef->epc1, ef->excvaddr);
	kprintf("|     ||  a0=%p  sp=%p  a2=%p  a3=%p\n", ef->a0, ef + 1, ef->a2, ef->a3);
	kprintf("| RIP ||  a4=%p  a5=%p  a6=%p  a7=%p\n", ef->a4, ef->a5, ef->a6, ef->a7);
	kprintf("|     ||  a8=%p  a9=%p a10=%p a11=%p\n", ef->a8, ef->a9, ef->a10, ef->a11);
	kprintf("|.,~||/|. a12=%p a13=%p a14=%p a15=%p\n", ef->a12, ef->a13, ef->a14, ef->a15);

	if (udata.u_insys)
		panic("fatal");
	/*
	 *	Synchronous user exception
	 */
	switch (cause) {
		case 0:	/* Illegal instruction usually */
			s = SIGILL;
			break;
		case 2:	/* Physical address/data fetch/store error - instruction */
		case 3: /* Physical address/data fetch/store error - data */
			s = SIGSEGV;
			break;
		case 6:	/* Divide by zero */
			s = SIGFPE;
			break;
		case 9:	/* Alignment trap */
			s = SIGBUS;
			break;
		case 20: /* Instruction load from data space */
			s = SIGSEGV;
			break;
		case 28: /* Invalid address traps */
		case 29:
			s = SIGSEGV;
			break;
		case 5:	/* Window trap - can't happen */
		case 8:	/* Privilege trap - can't happen */
		default: /* Should not happen */
			panic("badex");
			break;
	}
	/* Cheating here .. all our exceptions are low 16 signals */
	m = 1 << s;
	/*
	 *	The caller is ignoring our signal. In some cases this is fine
	 *	but in others it's less clear (eg division by zero) and we
	 *	may need to take different action.
	 */
	if (proc->p_sig[0].s_ignored & m)
		return 0;
	/* Weird case - we took a sync signal and the caller wants us to
	   report it later. */
	if (proc->p_sig[0].s_held & m) {
		/* TODO: if it's not meaningfully restartable we should
		   probably treat this as a kill */
		ssig(proc, s);
		return 0;
	}
	/* Requeue any asynchronous pending signal */
	recalc_cursig();
	if (udata.u_sigvec[s] == SIG_DFL)
		/* Will not return */
		doexit(dump_core(s));
        /* Cause a signal delivery to the CPU on the return path so it happens before we re-execute
           the faulting instruction */
	return s;
}

/*
 *	The ESP8266 timer, less than ideal in some ways as we have to keep reloading so it will
 *	drift over time. We should probably also use FRC2 to work out what to bump the counter
 *	by.
 *
 *	TODO - Move to FRC1  as it seems to have an auto-reload
 */
static void timer_isr(void)
{
	const uint32_t clocks_per_tick = (CPU_CLOCK*1000000) / TICKSPERSEC;
	uint32_t ccount;

	asm volatile ("rsr.ccount %0" : "=a" (ccount));
	asm volatile ("wsr.ccompare0 %0" :: "a" (ccount + clocks_per_tick));
	asm volatile ("esync");

	timer_interrupt();
}

/*
 *	Interrupts
 *	0: WDEV FIQ
 *	1: SLC
 *	2: SPI
 *	3: RTC
 *	4: GPIO
 *	5: UART			(used for TX interrupt only)
 *	6: TICK			(used for our clock)
 *	7: SOFT
 *	8: WDT
 *	9: FRC1			(needs different handling as it is edge triggered)
 *	10: FRC2		(ditto)
 */

static uint32_t irqmask;	/* A set bit is an enable */

static void irq_clear(uint32_t irq)
{
	asm volatile ("wsr.intclear %0; esync" :: "a" (irq));
}

void irq_enable(uint32_t irq)
{
	irqmask |= 1 << irq;
	irq_set_mask(irqmask);
}

void irq_disable(uint32_t irq)
{
	irqmask &= ~(1 << irq);
	irq_set_mask(irqmask);
}

void interrupt_handler(uint32_t interrupt)
{
	if (interrupt & (1 << 6)) {
		timer_isr();
		irq_clear(1 << 6);
	}
	if (interrupt & (1 << 5)) {
		tty_interrupt();
		irq_clear(1 << 5);
	}
}
