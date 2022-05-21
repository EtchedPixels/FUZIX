#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ibmpc.h>

uint8_t kernel_flag = 1;
uint8_t pc_at = 1;
uint16_t equipment_word;
uint8_t cpu_type;
uint8_t bus_width;
uint8_t fpu_type;

void plt_idle(void)
{
	/* Do a halt or what ? */
}

void do_beep(void)
{
}

void pagemap_init(void)
{
	/* FIXME: use proper kernel _discard to correct end allowing for EBDA */
	pagemap_init_range(64, 640);
}

/*
 *	MMU initialize. We use this to set everything up. Should move into
 *	discard
 */

void map_init(void)
{
	static char *cpu[]={"808","80C8","NEC V","8018","8028"};
	static char *fpu[]={"87","287+"};
	cpu_detect();
	kputs("Processor: ");
	kputs(cpu[cpu_type]);
	if (cpu_type == 2)
		kputs(bus_width == 8 ? "20" : "30");
	else
		kputs(bus_width == 8 ? "8" : "6");
	if (cpu_type == 4)
		kputs("+");
	if (fpu_type) {
		kputs(" / 80");
		kputs(fpu[fpu_type]);
	}
	kputs("\n");
	/* Should we do bogomips 8) */
//	init_irq();
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
