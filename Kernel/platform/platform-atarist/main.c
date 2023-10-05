#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <machine.h>

uint8_t need_resched;
uint16_t features;

void plt_idle(void)
{
}

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

void plt_copyright(void)
{

}

void do_beep(void)
{
}

struct probe_bits {
	const char *name;
	uint16_t bits;
	uint32_t addr;
};

struct probe_bits probes[] = {
	{ "falcon ", FEATURE_FALCON, 0xFF8007 },
	{ "tt ", FEATURE_TT, 0xFF8260 },
	{ "ste", FEATURE_STE, 0xFF8093 },

	{ "vme ", FEATURE_VME, 0xFF8E01 },
	{ "rtc ", FEATURE_RTC, 0xFFFC21 },
	{ "tt-rtc ", FEATURE_TTRTC, 0xFF8961 },
	{ "blitter ", FEATURE_BLITTER, 0xFF8A00 },
	{ "ide ", FEATURE_IDE, 0xF00009 },
	{ NULL, 0, 0 }
};

void map_init(void)
{
	struct probe_bits *p = probes;
	/* Useful spot for hardware set up and reporting */

	kputs("Features: ");
	while(p->name) {
		if (probe_memory((uint8_t *)p->addr) == 0) {
			features |= p->bits;
			kputs(p->name);
		}
		p++;
	}
	if ((features & (FEATURE_VME|FEATURE_TT)) == FEATURE_VME) {
		features |= FEATURE_MSTE;
		kputs("mste");
        }
        kputchar('\n');
}

u_block uarea_block[PTABSIZE];

uint8_t hzticks;
uint32_t memtop;
uint16_t fdseek;
uint16_t cputype;
uint32_t screenbase;

void pagemap_init(void)
{
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;
	hzticks = *(uint16_t *)0x448 ? 60 : 50;
	memtop = *(uint32_t *)0x42E;
	fdseek = *(uint16_t *)0x440;
	cputype = *(uint16_t *)0x59E;

	kprintf("System Memory: %dK\n", memtop >> 10);
	if (hzticks == 60)
		kputs("NTSC System\n");
	if (cputype)
		kputs("Not a 68000\n");
	/* Linker provided end of kernel */
	/* Allocate the rest of memory to the userspace */
	kmemaddblk((void *)e, screenbase - e);
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
