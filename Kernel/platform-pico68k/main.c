#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <flat_small.h>
#include <pico68.h>

uint16_t swap_dev = 0xFFFF;

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

/* FIXME: for this platform we definitely want a proper discard */
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
	uint32_t e = (uint32_t) & _end;
	/* Allocate the rest of memory to the userspace */
	/* Set up the memory range available */
	pagemap_setup(e, 0x30000 - e);
	/* The disk scan will have set up the paging space */
	/* We assume paging space and that all RAM pages have a swap
	   backing */
	kprintf("Motorola 680%s%d processor detected.\n", sysinfo.cpu[1] ? "" : "0", sysinfo.cpu[1]);
	enable_icache();
	/* Set up the VIA. We just use a few pins for SD right now */
	via->ddra = 0x07;	/* CS, CLK, MOSI out, MISO in, 4-7 free */
	via->ddrb = 0x00;	/* Unused */
	via->ra = 0x07;		/* All high for SD */
	/* We use T1 for the clock. This may need thought as abusing it to clock
	   SPI while clock counting the instructions has a certain appeal */
	via->t1c_l = (uint8_t) 40000;	/* 200Hz */
	via->t1c_h = 40000 >> 8;
	via->acr = 0x40;	/* Timer continous */
	via->ier = 0x40;	/* Timer interrupt */
}

/* Udata and kernel stacks */
/* We need an initial kernel stack and udata so the slot for init is
   set up at compile time */
u_block udata_block[PTABSIZE];

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *) udata.u_codebase, &vdso, 0x20);
}

uint8_t plt_udata_set(ptptr p)
{
	p->p_udata = &udata_block[p - ptab].u_d;
	return 0;
}

/* SPI glue */

void sd_spi_raise_cs(void)
{
	via->ra |= 4;
}

void sd_spi_lower_cs(void)
{
	via->ra &= ~4;
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}
