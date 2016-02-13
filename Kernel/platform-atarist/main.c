#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

void platform_idle(void)
{
}

void do_beep(void)
{
}

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
	vmmu_init();
}

void map_init(void)
{
}

u_block uarea_block[PTABSIZE];
uaddr_t ramtop;

void *screenbase;
uint8_t *membase = 0x10000;	/* GUESS FIXME */
uint8_t *memtop;

/*
 *	We can do our fork handling in C for once. The only oddity here is
 *	the fixups to run parent first and avoid needless memory thrashing
 */
void dofork(ptptr p)
{
	/* Child and parent udata pointers */
	u_block *uc = uarea_block + p - ptab;
	u_block *up = udata_ptr;
	uint32_t *csp = uc + 1;
	uint32_t *psp = up->u_sp;
	/* Duplicate the memory maps */
	if (pagemap_fork(p))
		return -1;
	/* Duplicate the udata */
	memcpy(&uc->u_d, &up->u_d, sizeof(struct u_data))
	/* Use the child udata for initializing the child */
	udata_ptr = uc;
	newproc(p);
	udata_ptr = up;
	/* And return as the parent. The child will return via the
	   fork return path */
	*--csp = fork_return;
	uc->u_sp = csp;
	/* Copy the saved register state over - must match switchin */
	memcpy(csp - 14, psp - 14, 4 * 14);
	/* Return as the parent and run it first (backwards to most ports) */
	p->p_status = P_READY;
	udata.u_ptab->p_status = P_RUNNING;
	return p->p_pid;
}
