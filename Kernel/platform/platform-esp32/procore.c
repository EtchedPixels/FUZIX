#include <kernel.h>
#include "globals.h"
#include "printf.h"
#include "xtos.h"

void __attribute__((noreturn)) procore_main(void)
{
	for (;;)
		plt_idle();
}
