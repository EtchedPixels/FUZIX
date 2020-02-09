#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

extern void *get_usp(void);
extern void set_usp(void *p);

/* We do as much exception processing in software as we can. This code
  is called from the asm trap hooks on the supervisor stack and expected
  to sort the mess out */

/* The stackframe we work with looks like this when exception is called */

struct trapdata {
	/* MOVEM.L with predecrement stores A7 to A0 then D7 to D0. In other
	   words viewed in memory order it is D0-D7 then A0-A7 (A6 for us
	   as we don't store A7) */
	uint32_t d[8];
	uint32_t a[7];
	uint16_t trap;		/* Pushed second */
	uint16_t sr;		/* Condition codes */
	/* The CPU pushed the frame which sits above the MOVEM */
	uint16_t exception[0];	/* CPU exception frame */
};

#define FRAME_A		1
#define FRAME_B		2
#define FRAME_C		3
#define FRAME_D		4

static void explode(struct trapdata *framedata, int type)
{
	uint16_t *excp = framedata->exception;
	uint32_t *fv;
	unsigned int i = framedata->trap;
	unsigned int j;

	if (i > 12)
		i = 12;

	for (j = 0; j < i; j++)
		kputs("   '* ");
	kputchar('\n');
	for (j = 0; j < i; j++)
		kputs("  |   ");
	kputchar('\n');
	for (j = 0; j < i; j++)
		kputs(".###. ");
	kputchar('\n');
	for (j = 0; j < i; j++)
		kputs("##### ");
	kputchar('\n');
	for (j = 0; j < i; j++)
		kputs("##### ");
	kputchar('\n');
	for (j = 0; j < i; j++)
		kputs("`###' ");
	kputchar('\n');

	kprintf("Trap: %d\n", framedata->trap);
	kprintf("Register Dump\nA: ");
	for (i = 0; i < 7; i++)
		kprintf("%p ", framedata->a[i]);
	kprintf("%p\nD: ", get_usp());
	for (i = 0; i < 8; i++)
		kprintf("%p ", framedata->d[i]);
	kprintf("\nSR: %x\n", framedata->sr);

	kprintf("Exception frame:\n");
	for (j = 0;j < 16; j++)
		kprintf("%d: %x\n", j, excp[j]);

	/* 68010 long frame */
	if (type == FRAME_C) {
		kputs((excp[4] & 0x100)?"R":"W");
		kprintf(" FC %x", excp[4] & 7);
		fv = (uint32_t *)(excp +5);
		kprintf(" Addr %p SSW %x ", *fv, excp[4]);
	}
	/* 68000 long frame */
	if (type == FRAME_A) {
		kputs((excp[0] & 0x10)?"R":"W");
		kprintf(" FC %x", excp[0] & 7);
		fv = (uint32_t *)(excp + 1);
		kprintf(" Addr %p IR %x ", *fv, excp[3]);
		excp += 4;
	}
	/* All frames */
	fv = (uint32_t *)(excp + 1);
	kprintf("PC %p SR %x\n", *fv, *excp);
}

/* Our caller did a movem of the registers to kstack then pushed an
   exception and other info. This all sits on the kstack but pass
   a pointer to it for flexibility.

   On return it will pull the state back and rte. We can thus adjust
   what happens according to need.

   To keep common code we push the same frame for all cases */

static uint8_t pushw(uint16_t **usp, uint16_t v)
{
	(*usp)--;
	return uputw(v, *usp);
}

static uint8_t pushl(uint16_t **usp, uint32_t v)
{
	(*usp) -= 2;
	return uputl(v, *usp);
}

extern uint8_t kernel_flag;

int exception(struct trapdata *framedata)
{
	ptptr proc;
	unsigned int frame, fsize;
	uint8_t trap = framedata->trap;
	unsigned int sig = 0;
	uint16_t m;
	uint16_t *sp = (uint16_t *)framedata;
	uint16_t *usp = get_usp();
	uint16_t *unwind, *context;
	uint8_t err = 0;
	int i;

	/* We translate debug to SIGIOT and A and F line to SIGILL
	   We will need to fix up 68000 v 68010 move to/from SR */
	static const uint8_t trap_to_sig[] = {
		0, 0, SIGSEGV, SIGBUS, SIGILL, SIGFPE,
		SIGABRT/*CHK*/, SIGTRAP/*TRAPV */,
		SIGILL, SIGIOT, SIGILL, SIGFPE };

	proc = udata.u_ptab;

	if (sysinfo.cpu[0] == 10) {
		/* Long or short frame: the CPU tells us the frame format */
		if (framedata->exception[3] & 0x8000) {
			fsize = 29;
			frame = FRAME_C;
		} else  {
			fsize = 4;
			frame = FRAME_D;
		}
	} else {
		frame = FRAME_B;
		fsize = 3;
	}
	if (trap == 0) {
		sig = udata.u_cursig;
		udata.u_cursig = 0;
	} else if (trap < 12) {
		if (trap < 4) {
			/* On a 68010 this frame is 29 words and the event is
			   restartable (although not always usefully). We need to
			   decide whether to set the restart flag case by case */
			if (sysinfo.cpu[0] == 0) {
				frame = FRAME_A;
				fsize = 7;
			}
		}
		sig = trap_to_sig[trap];
	} else if (trap >= 32 && trap < 48)
		sig = SIGTRAP;
	/* This processing only applies to synchronous hardware exceptions */
	if (trap) {
		/* Went boom in kernel space or without a user recovery */
		if (kernel_flag || sig == 0) {
			explode(framedata, frame);
			panic("trap");
		}

		/* Cheating here .. all our exceptions are low 16 signal */
		m = 1 << sig;
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
			ssig(proc, sig);
			return 0;
		}
		recalc_cursig();	/* Put any async signal back */
	}
	if (udata.u_sigvec[sig] == SIG_DFL) {
		/* Default action for our signal ? */
		doexit(dump_core(sig));
		/* This will never return. We will go schedule new work */
		panic("exret");
	}
	/* build signal frame

		Our unwinder code does
		move.l 8(sp),sp
		movem.l a0-a1/d0-d1,(sp)+
		move.w (sp)+,ccr
		rts */


	/* Now update the user stack */
	/* - Push the recovery PC */
	/* - Patch the kernel exception frame */
	if (sysinfo.cpu[0]) {
		/* FIXME */
		err |= pushw(&usp, sp[34]);
		err |= pushw(&usp, sp[33]);
		*(uint32_t *)(&sp[33]) = (uint32_t)udata.u_sigvec[sig];
	} else {
		err |= pushw(&usp, sp[31 + fsize]);
		err |= pushw(&usp, sp[30 + fsize]);
		*(uint32_t *)(&sp[30 + fsize]) = (uint32_t)udata.u_sigvec[sig];
	}

	/* FIXME: when we do ptrace we will need to support adding the T
	   flag back here as the exception cleared it */
	err |= pushw(&usp,  framedata->sr);
	/* Push A1 A0 D1 D0 to match MOVEM.L */
	err |= pushl(&usp, framedata->a[1]);
	err |= pushl(&usp, framedata->a[0]);
	err |= pushl(&usp, framedata->d[1]);
	err |= pushl(&usp, framedata->d[0]);

	/* Remember the target for undoing the frame */
	unwind = usp;

	/* Copy in the signal context itself. 30 words of registers, 2 of
	   trap code and then the hardware exception */
	for (i = 0; i < 30 + 2 + fsize; i++)
		err |= pushw(&usp, *sp++);
	context = usp;
	err |= pushl(&usp, (uint32_t)unwind);
	/* Signal context is a secret extra non portable argument */
	err |= pushl(&usp, (uint32_t)context);
	/* We end it with the call frame as seen from the signal handler, a
	   single argument and a return address */
	err |= pushl(&usp, sig);
	err |= pushl(&usp, udata.u_codebase + 0x04);

	set_usp(usp);

	if (err) {
		doexit(dump_core(SIGSTKFLT));
		panic("exret2");
	}
	/* Once built clear the restart state */
	udata.u_sigvec[sig] = SIG_DFL;
	/* Return, RTE and end up on the signal frame */
	return 1;
}

void set_cpu_type(void)
{
	sysinfo.cpu[1] = cpu_type();
	/* We don't have a 68010 flag as there really isn't anything user
	   that matters to a well behaved 68000 app */
	if (sysinfo.cpu[1] >= 20)
		sys_cpu_feat |= AF_68000_020;
}
