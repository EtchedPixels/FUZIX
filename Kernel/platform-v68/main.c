#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <buddy.h>

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
#if 0
void pagemap_init(void)
{
	/* Allocate the buddy tables and init them */
//FIXME	buddy_init();
}
#endif

void map_init(void)
{
}

u_block udata_block[PTABSIZE];
uaddr_t ramtop;
uint8_t *membase[PTABSIZE];
uint8_t need_resched;
#if 0
/* Offsets into the buddy map for each level, byte aligned */
const uint16_t buddy_level[BUDDY_NUMLEVEL] = {
	0,		/* 256 4K pages */
	256,		/* 128 8K pages */
	384,		/* 64 16K pages */
	448,		/* 32 32K pages */
	480,		/* 16 64K pages */
	496,		/* 8 128K pages */
	504,		/* 4 256K pages */
	508,		/* 2 512K pages */
	510,		/* 1 1MB page */
};
#endif
/*
 *	We can do our fork handling in C for once. The only oddity here is
 *	the fixups to run parent first and avoid needless memory thrashing
 */
int16_t dofork(ptptr p)
{
	/* Child and parent udata pointers */
	struct u_data *uc = &udata_block[p - ptab].u_d;
	struct u_data *up = udata_ptr;
	uint32_t *csp = (uint32_t *)(uc + 1);
	uint32_t *psp = up->u_sp;
	/* Duplicate the memory maps */
	/* FIXME 
	if (pagemap_fork(p))
		return -1; */
	/* Duplicate the udata */
	memcpy(&uc, &up, sizeof(struct u_data));
	/* Use the child udata for initializing the child */
	udata_ptr = uc;
	newproc(p);
	udata_ptr = up;
	/* And return as the parent. The child will return via the
	   fork return path */
	/* FIXME: stack setup needs correcting */
//FIXME	*--csp = fork_return;
	uc->u_sp = csp;
	/* Copy the saved register state over - must match switchin */
	memcpy(csp - 14, psp - 14, 4 * 14);
	/* Return as the parent and run it first (backwards to most ports) */
	p->p_status = P_READY;
	udata.u_ptab->p_status = P_RUNNING;
	return p->p_pid;
}

/* All our binaries are zero address based */

uint8_t *pagemap_base(void)
{
	return (uint8_t*)0x20000UL;
}

void program_mmu(uint8_t *phys, usize_t top)
{
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

