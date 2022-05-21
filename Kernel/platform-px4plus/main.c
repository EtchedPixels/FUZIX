#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devfd.h>

uint16_t ramtop = PROGTOP;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

__sfr __at 0x04	isr;

void plt_interrupt(void)
{
 uint8_t irq = isr;
 /* We need to handle 1 for the 7508, and 2 for GAPNIO (serial in) at least */
 /* Overflow on timer */
 if (irq & 4)
  timer_interrupt();
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}

extern uint8_t map_translate[MAX_MAPS];
extern uint8_t *map_live;

/* Add map numbers. These are logical mappings one of which is the current
   memory image and the others indexes into the sidecar or cartridge memory.

   Put map #1 last as it means "in memory" and we want it for init */

void pagemap_init(void)
{
	uint8_t n = procmem / 32;
	uint8_t *p = map_translate;
	uint8_t nv = 0xFF;
	uint8_t ct;
	uint8_t i;

	if (!sidecar && carttype != 2)
	 panic("No suitable memory extension");
	n /= 32;
	/* Handle sidecar memory */
	if (sidecar) {
		nv = n - 4;	/* Number of non sidecar blocks (including in RAM) */
		ct = 0x83;	/* 0x80-0x83 are sidecar maps */
	} else
		ct = n;		/* 1..n are cartridge (1 = in memory) */

	for (i = n; i > 0; i--) {
		if (i == nv)
			ct = nv;
		*p++ = ct--;
		pagemap_add(i);
	}
	map_live = p - 1;	/* We add RAM last */
}

void map_init(void)
{
}
