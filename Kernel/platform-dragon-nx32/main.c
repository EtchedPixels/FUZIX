#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>

uint8_t membanks;
uint8_t system_id;
uint8_t cartslots = 1;
uint16_t carthash[4];
uint8_t bootslot = 0;

void platform_idle(void)
{
}

void do_beep(void)
{
}


/* ------------- Below bits can all move to discard if needed ------------ */

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
	int i;
	/* map bank 1 last for init, leave 0 for kernel */
	for (i = membanks - 1; i > 0; i--)
		pagemap_add(i);

#ifdef SWAPDEV
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_add(i);
#endif
}

static const char *sysname[] = {"Dragon", "COCO", "COCO3", "Unknown"};

void map_init(void)
{
	uint8_t i;

	kprintf("%s system.\n", sysname[system_id]);
	if (mpi_present()) {
		kputs("MPI cartridge detected.\n");
		cartslots = 4;
		bootslot = mpi_set_slot(0);
	}
	for (i = 0; i < cartslots; i++) {
		mpi_set_slot((i << 4) | i);
		carthash[i] = cart_hash();
		kprintf("%d: %x %c\n",
			i, carthash[i],
			i == bootslot ? '*':' ');
	}
	mpi_set_slot(bootslot);
}
