#include <kernel.h>
#include "globals.h"

void __attribute__((noreturn)) __pro_cpu_main(void)
{
	for (;;)
		platform_idle();
}

