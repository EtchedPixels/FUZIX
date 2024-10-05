#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

extern uint8_t kempston, kmouse, kempston_mbmask;

uint8_t plt_param(char *p)
{
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}

void pagemap_init(void)
{
	uint8_t i;
	/* We have 256 pages 0-255 but we need to work with the complement */
	/* In addition certain pages are magic
		5 / 2 / 0 are the classic spectrum pages
		0-7 are the 128K spectrum pages
		then above that the compat ones for Pentagon etc
		
		This effectively means we need to consider special:
		page 5/7 for the Spectrum video modes
		page 4/6 for 256x192 16 colour
		page 1/5 for 640x200 and 320x200 mode
		page 1/3/5/7 for ATM text mode without it being packed
		In one page mode it ends up in page 8 or 10 !
		
		So we do
		0 / 1 / 2 / 3		Kernel
		4-8			Video
		9			Buffers (eventually)
		10			Video alt
		11			Init
		
		We could in theory reduce it a bit by shuffling the buffers
		around on a video change
		
		We have 4MB, we actually can't use even 1MB with 16 processes!
		
		Might want to move the kernel a bit so we can run a ZX spectrum
		as a process ?
		
		*/
	for (i = 12; i < 255; i++) {
		pagemap_add(~i);
		i++;
	}
	/* Common for init */
	pagemap_add(~10);
}

void plt_copyright(void)
{
	/* We always have the kempston interfaces present */
	kempston = 1;
	kmouse = 1;
	kempston_mbmask = 7;
	/* FIXME: do we need to tweak dev/zx/devinput to allow us to force
	   'turbo' features or make mbmask tell us - the Evo has 3 buttons
	  and wheel on 4-7 */
}

