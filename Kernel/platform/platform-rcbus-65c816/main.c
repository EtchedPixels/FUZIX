#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint16_t swap_dev = 0xFFFF;
uint8_t acia_present;
uint8_t uart_present;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * Map handling: allocate 3 banks per process
 */

void pagemap_init(void)
{
    int i;
    /* Add the user banks, taking care to land 36 as the last one as we
       use that for init  (32-35 are the kernel) */
    for (i = 6; i >= 0; i--)
        pagemap_add(36 + i * 4);
}

void map_init(void)
{
}

uint8_t plt_param(char *p)
{
    return 0;
}

static volatile uint8_t *via = (volatile uint8_t *)0xFE60;

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
	/* FIXME: we need a way to time the CPU against something to get
	   the VIA clock rate. For now hard code 4MHz */
	/* Timer 1 free running */
	via[11] = 0x40;
	via[4] = 0x40;	/* 100Hz at 4MHz */
	via[5] = 0x9C;
	via[14] = 0x7F;	/* Clear IER */
	via[14] = 0xC0;	/* Enable Timer 1 */
}

void plt_reinterrupt(void)
{
	tty_poll();
}

void plt_interrupt(void)
{
	uint8_t dummy;

	tty_poll();
	if (via[13] & 0x40) {
		dummy = via[4]; /* Reset interrupt */
		timer_interrupt();
	}
}

/* For now this and the supporting logic don't handle swap */

extern uint16_t hd_map;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_read_data(blk_op.addr);	
}

void devide_write_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_write_data(blk_op.addr);	
}

