#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdarg.h>
#include "iomacros.h"
#include "intrinsics.h"
#include "msp430fr5969.h"

__interrupt void interrupt_handler(void)
{
	platform_interrupt();
}

void doexec(uaddr_t start_addr)
{
	di();
	udata.u_insys = 0;

	asm volatile(
		"mov %0, sp\n"
		"eint\n"
		"br %1\n"
		:
		: "g" (udata.u_isp),
		  "g" (start_addr));
}

static void unix_syscall_main(uint16_t arg0, uint16_t arg1, uint16_t arg2,
	uint16_t arg3)
{
	struct u_data* u = &udata;
	u->u_argn = arg0;
	u->u_argn1 = arg1;
	u->u_argn2 = arg2;
	u->u_argn3 = arg3;
	ei();

	unix_syscall();

	di();
}

__attribute__ ((naked)) void unix_syscall_entry(void)
{
	asm volatile (
		"dint\n"
		"mov.b r11, %0\n"
		"mov.w sp, %1\n"
		"movx.a #kstack_top, sp\n"
		"calla %2\n"
		"mov.w %1, sp\n"
		"mov.w %3, r12\n"
		"mov.w %4, r13\n"
		"eint\n"
		"reta\n"
		: "=m" (udata.u_callno),
		  "=m" (udata.u_syscall_sp)
		: "g" (unix_syscall_main),
		  "g" (udata.u_retval),
		  "g" (udata.u_error)
	);
}

int16_t dofork(ptptr child)
{
	kprintf("dofork\n");
	for (;;);
}

