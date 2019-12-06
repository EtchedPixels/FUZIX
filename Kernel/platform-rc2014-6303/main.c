#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint16_t swap_dev = 0xFFFF;

void platform_idle(void)
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
    /* 32-35 are the kernel, 36-38 / 39-41 / etc are user */
    /* Add 36-38 last as we hardcode 36 into our init creation in tricks.s */
    for (i = 8; i >= 0; i--)
        pagemap_add(36 + i * 3);
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
    return 0;
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
	/* FIXME: program timer */
}

void platform_interrupt(void)
{
	uint8_t dummy;

	tty_poll();
#if 0 /* FIXME timer */
	if (via[13] & 0x40) {
		dummy = via[4]; /* Reset interrupt */
		timer_interrupt();
	}
#endif	
}

/* For now this and the supporting logic don't handle swap */

extern uint8_t hd_map;
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

