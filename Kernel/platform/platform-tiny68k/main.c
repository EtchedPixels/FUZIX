#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

uint16_t swap_dev = 0xFFFF;

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

uint8_t plt_param(char *p)
{
	return 0;
}

void plt_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

void pagemap_init(void)
{
	/* Linker provided end of kernel */
	/* TODO: create a discard area at the end of the image and start
	   there */
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;
	/* Allocate the rest of memory to the userspace */
	kmemaddblk((void *)e, 0xFF8000 - e);

	kprintf("Motorola 680%s%d processor detected.\n",
		sysinfo.cpu[1]?"":"0",sysinfo.cpu[1]);
	enable_icache();
}

/* Udata and kernel stacks */
/* We need an initial kernel stack and udata so the slot for init is
   set up at compile time */
u_block udata_block0;
static u_block *udata_block[PTABSIZE] = {&udata_block0,};

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x20);
}

uint8_t plt_udata_set(ptptr p)
{
	u_block **up = &udata_block[p - ptab];
	if (*up == NULL) {
		*up = kmalloc(sizeof(struct u_block), 0);
		if (*up == NULL)
			return ENOMEM;
	}
	p->p_udata = &(*up)->u_d;
	return 0;
}
