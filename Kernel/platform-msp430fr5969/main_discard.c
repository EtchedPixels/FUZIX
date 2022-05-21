#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include "externs.h"

void map_init(void)
{
}

void plt_init(void)
{
	int i;

	memset(&udata, 0, sizeof(udata));

	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);

	tty_rawinit();
	fuzix_main();
	for (;;);
}

