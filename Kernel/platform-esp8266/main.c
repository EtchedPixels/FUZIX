#include <stdint.h>
#include <eagle_soc.h>
#include "esp8266_peri.h"
#include "kernel.h"
#include "kdata.h"
#include "printf.h"
#include "ftl.h"
#include "globals.h"
#include "rom.h"

volatile uint32_t* const esp8266_gpioToFn[16] = { &GPF0, &GPF1, &GPF2, &GPF3, &GPF4, &GPF5, &GPF6, &GPF7, &GPF8, &GPF9, &GPF10, &GPF11, &GPF12, &GPF13, &GPF14, &GPF15 };

uaddr_t ramtop = DATATOP;
uint8_t need_resched;

void map_init(void) {}
void platform_discard(void) {}
void platform_monitor(void) {}
void platform_reboot(void) {}

uint_fast8_t platform_param(char* p)
{
	return 0;
}

void device_init(void)
{
	flash_dev_init();
}

static void syscall_handler_cb(struct __exception_frame *ef, int cause)
{
	udata.u_callno = ef->a2;
	udata.u_argn = ef->a3;
	udata.u_argn1 = ef->a4;
	udata.u_argn2 = ef->a5;
	udata.u_argn3 = ef->a6;
	udata.u_insys = 1;

	unix_syscall();

	udata.u_insys = 0;

	ef->a2 = udata.u_retval;
	ef->a3 = udata.u_error;

	ef->epc += 3; /* skip SYSCALL instruction */
}

int main(void)
{
	ramsize = 80;
	procmem = 64;
    sys_cpu_feat = AF_LX106_ESP8266;

	_xtos_set_exception_handler(1, syscall_handler_cb);

	di();
	fuzix_main();
}

