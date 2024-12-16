#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint8_t need_resched;
uint16_t swap_dev = 0xFFFF;

#define IO(x)		*((volatile uint8_t *)0xFE00 + (x))

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}


void plt_interrupt(void)
{
	uint8_t dummy;

	tty_poll();

	/* Check the PTM */
	if (IO(0x61) & 0x80) {
		IO(0x66);
		timer_interrupt();
	}
}

/* For now this and the supporting logic don't handle swap */
extern uint8_t hd_map;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(uint8_t *p)
{
	if (td_raw)
		hd_map = 1;
	else
		hd_map = 0;
	hd_read_data(p);
}

void devide_write_data(uint8_t *p)
{
	if (td_raw)
		hd_map = 1;
	else
		hd_map = 0;
	hd_write_data(p);
}