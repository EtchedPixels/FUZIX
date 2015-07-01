#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include "externs.h"

void map_init(void)
{
}

void platform_init(void)
{
	memset(&udata, 0, sizeof(udata));
	tty_rawinit();
	fuzix_main();
	for (;;);
}

