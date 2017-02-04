#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <flat_mem.h>

/* FIXME don't hard code! */
uint8_t *memtop =  (uint8_t *)0x100000;
uint8_t *membase = (uint8_t *)0x018000;

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

void pagemap_init(void)
{
	vmmu_init();
}

uaddr_t ramtop;
uint8_t need_resched;

uint8_t platform_param(char *p)
{
	return 0;
}

void platform_discard(void)
{
}

void platform_mmu_setup(struct mmu_context *m)
{
	/* Allocate an initial space for the init task that will hold
	   the execve arguments and be freed when init loads */
	m->base = vmmu_alloc(&m->mmu, MMU_BLKSIZE, 0, 0,  1);
	if (m->base == NULL)
		panic("initmmu");
	kprintf("init boot at %p\n", m->base);
}

void fast_zero_block(void *p)
{
	memset(p, 0, MMU_BLKSIZE);
}

void fast_swap_block(void *ap, void *bp)
{
	/* TODO */
	uint32_t *a = ap, *b = bp;
	int n = 0;
	while(n++ < MMU_BLKSIZE/4) {
		uint32_t t = *a;
		*a++ = *b;
		*b++ = t;
	}
}

void fast_copy_block(void *a, void *b)
{
	/* TODO */
	memcpy(a, b, MMU_BLKSIZE);
}

void fast_op_complete(void)
{
}


/* Live udata and kernel stack for each process */
/* FIXME: we need to find a way to make these smaller or bank them out but
   that has a material cost */

u_block udata_block[PTABSIZE];
uint16_t irqstack[128];

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x40);
}

extern void *get_usp(void);
extern void set_usp(void *p);

void signal_frame(uint8_t *trapframe, uint32_t d0, uint32_t d1, uint32_t a0,
	uint32_t a1)
{
	extern void *udata_shadow;
	uint8_t *usp = get_usp();
	udata_ptr = udata_shadow;
	uint16_t ccr = *(uint16_t *)trapframe;
	uint32_t addr = *(uint32_t *)(trapframe + 2);
	int err = 0;

	/* Build the user stack frame */

	/* FIXME: eventually we should put the trap frame details and trap
	   info into the frame */
	usp -= 4;
	err |= uputl(addr, usp);
	usp -= 4;
	err |= uputw(ccr, usp);
	usp -= 2;
	err |=uputl(a1, usp);
	usp -= 4;
	err |= uputl(a0, usp);
	usp -= 4;
	err |= uputl(d1, usp);
	usp -= 4;
	err |= uputl(d0, usp);
	usp -= 4;
	err |= uputl(udata.u_codebase + 4, usp);
	set_usp(usp);

	if (err) {
		kprintf("%d: stack fault\n", udata.u_ptab->p_pid);
		doexit(dump_core(SIGKILL));
	}
	/* Now patch up the kernel frame */
	*(uint16_t *)trapframe = 0;
	*(uint32_t *)(trapframe + 2) = (uint32_t)udata.u_sigvec[udata.u_cursig];
	udata.u_sigvec[udata.u_cursig] = SIG_DFL;
	udata.u_cursig = 0;
}

