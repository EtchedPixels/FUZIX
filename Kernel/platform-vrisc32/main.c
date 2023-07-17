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
	uint8_t r;
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
extern uint32_t *get_usp();
extern void set_usp(uint32_t *p);

/* This is the contents of the kstack above the C call stack for the
   exception routine. We will need to handle FPU in future FIXME */

struct trapdata {
	uint32_t reg[9];	/* Enter frame R0-R7 + old FP */
	uint32_t ra;		/* Return address */
	uint16_t psr;		/* Saved PSR */
};

static uint8_t pushd(uint32_t **usp, uint32_t v)
{
	(*usp) --;
	return uputl(v, *usp);
}

/* Invoked whenever an exception occurs */
void exception(struct trapdata *frame, uint32_t event)
{
	uint16_t m;
	ptptr proc = udata.u_ptab;
	static const uint8_t sigtable[] = {
		SIGBUS,	/* Abort */
		SIGFPE, /* Slave ? FPU or MMU fault - need to dig more */
		SIGILL,	/* Tried to execute supervisor op (ilegal in NS speak) */
		SIGFPE,	/* Division by zero */
		SIGTRAP, /* Flag instruction saw F bit set */
		SIGTRAP, /* Breakpoint trap */
		SIGTRAP, /* Trace trap */
		SIGILL /* Undefined opcode (aka illegal instruction) */
	};
	uint8_t sig = sigtable[event];
	uint32_t *kframe = (uint32_t *)frame;
	uint32_t *usp;
	int err;

	if (kernel_flag) {
		uint8_t i;
		/* FIXME dump registers nicely */
		for (i = 0; i < 16; i++)
			kprintf("%x\n", *++kframe);
		panic("ktrap");
	}
	/* Signal ourselves. We take care that the return out of
	   the exception handler always checks */
	ssig(proc, sig);
	chksigs();
}
