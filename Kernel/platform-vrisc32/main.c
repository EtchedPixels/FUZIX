#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

void plt_idle(void)
{
	/* FIXME: disable IRQ, run tty interrupt, re-enable ? */
	irqflags_t irq = di();
	tty_poll();
	irqrestore(irq);
}

/*
 *	MMU initialize
 */

void map_init(void)
{
}

uaddr_t ramtop;
uint8_t need_resched;

uint8_t plt_param(char *p)
{
	return 0;
}

void plt_discard(void)
{
}

void program_vectors(uint16_t *pptr)
{
}

/* TODO */
void deliver_signals(void)
{
}

void pagemap_init(void)
{
	/* Linker provided end of kernel */
	/* TODO: create a discard area at the end of the image and start
	   there */
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;
	/* Allocate the rest of memory to the userspace */
	/* This will do for our test code but a lot of real RiscV is split I and D spaces so will need
	   us to add some kind of split allocator */
	kmemaddblk((void *)e, 0x3FCE0000 - e);
//	enable_icache();
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

/* Live udata and kernel stack */
u_block udata_block[PTABSIZE];
uint16_t irqstack[128];	/* Used for swapping only */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x20);
}

uint8_t plt_udata_set(ptptr p)
{
	p->p_udata = &udata_block[p - ptab].u_d;
	return 0;
}

extern uint8_t kernel_flag;

/* Invoked whenever an exception occurs (except syscall at the moment) */
void exception(uint32_t mepc, uint32_t mcause, uint32_t mvalue)
{
	ptptr proc = udata.u_ptab;
	static const uint8_t sigtable[] = {
		SIGBUS,		/* Instruction alignment */
		SIGSEGV,	/* Access fault */
		SIGILL,		/* Illegal instruction */
		SIGIOT,		/* Breakpoint */
		SIGBUS,		/* Load alignment */
		SIGBUS,		/* Store/AMO alignment */
		SIGSEGV,	/* Store/AMO access */
		0,		/* Syscall from user */
		0,		/* Syscall from S */
		SIGILL,		/* Unused */
		0,		/* Syscall from kernel mode */
		SIGSEGV,	/* Instruction page fault */
		SIGSEGV,	/* Load page fault */
		SIGILL,		/* Unused */
		SIGSEGV,	/* Store/AMO page fault */
	};
	uint8_t sig;

	/* Interrupts also pass this way */
	if (mcause & 0x80000000) {
//TODO		platform_interrupt(mcause);
		return;
	}

	if (mcause < 16)
		sig = sigtable[mcause];
	else
		sig = SIGILL;

	/* FIXME - kill off kernel_flag for udata.u_insys ? */
	if (kernel_flag) {
		kprintf("mepc %p mcause %d mvalue %p\n", mepc, mcause, mvalue);
		panic("ktrap");
	}
	/* Signal ourselves. We take care that the return out of
	   the exception handler always checks */
	ssig(proc, sig);
	chksigs();
}
