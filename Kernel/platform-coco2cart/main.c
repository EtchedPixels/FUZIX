#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>

uint8_t membanks;
uint8_t system_id;
uint16_t swap_dev = 0xFFFF;
struct blkbuf *bufpool_end = bufpool + NBUFS;
/* We start in text 32,16 and change to 32x24 after boot */
uint8_t vid_h = 16;
uint8_t vid_b = 15;

void plt_idle(void)
{
}

void do_beep(void)
{
}

void plt_discard(void)
{
	vid256x192();
	kputs("\033Y  \033J");
}

