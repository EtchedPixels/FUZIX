#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "iomacros.h"
#include "intrinsics.h"

__interrupt void interrupt_handler(void)
{
	platform_interrupt();
}

void doexec(uaddr_t start_addr)
{
	kprintf("exec %lx\n", start_addr);
	for (;;);
}

