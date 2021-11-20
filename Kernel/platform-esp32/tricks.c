#include "kernel.h"
#include "kdata.h"
#include "printf.h"
#include "soc/gpio_struct.h"
#include "kernel-esp32.def"
#include "xtos.h"

int16_t dofork(ptptr child)
{
	panic("dofork");
}

void platform_switchout(void)
{
	panic("switchout");
}

void switchin(ptptr process)
{
	panic("switchin");
}

void program_vectors(uint16_t* pageptr) {}

void platform_idle(void)
{
	asm volatile ("waiti 0");
}

void platform_doexec(void)
{
	panic("doexec");
}

