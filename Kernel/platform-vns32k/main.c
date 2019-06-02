#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint32_t sysconfig;

void platform_idle(void)
{
	/* FIXME: disable IRQ, run tty interrupt, re-enable ? */
}

void do_beep(void)
{
}

/*
 *	MMU initialize
 */

void map_init(void)
{
}

uaddr_t ramtop;
uint8_t need_resched;

uaddr_t pagemap_base(void)
{
	return 0x10000UL;
}

uint8_t platform_param(char *p)
{
	return 0;
}

void platform_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

arg_t _memalloc(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOSYS;
	return -1;
}

/* Live udata and kernel stack */
u_block udata_block;
uint16_t irqstack[128];	/* Used for swapping only */

void install_vdso(void)
{
//	extern uint8_t vdso[];
	/* Should be uput etc */
//	memcpy((void *)udata.u_codebase, &vdso, 0x40);
}

void platform_interrupt(void)
{
	if (*(volatile uint8_t *)0xFFF40)
		timer_interrupt();
	tty_interrupt();
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
	/* All our traps are in the low 16 signals */
	m = 1 << sig;
	if (proc->p_sig[0].s_ignored & m)
		return;
	if (proc->p_sig[0].s_held & m) {
		ssig(proc, sig);
		return;
	}
	recalc_cursig();
	if (udata.u_sigvec[sig] == SIG_DFL) {
		/* Check if we need any other default checks here (and 68000exception.c) */
		doexit(dump_core(sig));
		panic("exret");
	}
	/* At this point the user stack is untouched by the exception and the
	   kernel stack return address points to the next user instruction
	   (or continuation of the existing one in some cases
	   
	   Our unwinder currently (and we don't save a copy of kernel context)

	   cmpd 0,0		; drop signal argument and spare for context
	   movd tos,r0
	   lprb usr,r0
	   restore [r0,r1]
	   ret 0
	   
	   So we need to push
		r0,r1
		saved status reg (off kstack) as word
		the 32bit return in the kstack
		context (tbd)
		signal number
		unwinder
	*/
	usp = get_usp();
	err = pushd(&usp, frame->reg[0]);
	err |= pushd(&usp, frame->reg[1]);
	err |= pushd(&usp, frame->psr);
	err |= pushd(&usp, frame->ra);
	err |= pushd(&usp, 0);
	err |= pushd(&usp, sig);
	/* TBD FIXME */
	err |= pushd(&usp, udata.u_codebase + 0x04);
	frame->ra = (uint32_t)udata.u_sigvec[sig];
	udata.u_sigvec[sig] = SIG_DFL;
	set_usp(usp);

	if (err) {
		doexit(dump_core(SIGSTKFLT));
		panic("exret2");
	}
}
