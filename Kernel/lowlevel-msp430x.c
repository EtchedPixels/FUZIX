#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdarg.h>
#include "iomacros.h"
#include "intrinsics.h"

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

__attribute__ ((naked)) void unix_syscall_entry(void)
{
	asm volatile (
		"mov.b r11, %0\n"
		"mov r12, %1\n"
		"mov r13, %2\n"
		"mov r14, %3\n"
		"mov r15, %4\n"
		"jmp unix_syscall_main\n"
		: "=m" (udata.u_callno),
		  "=m" (udata.u_argn),
		  "=m" (udata.u_argn1),
		  "=m" (udata.u_argn2),
		  "=m" (udata.u_argn3)
		:
	);
}

void unix_syscall_main(void)
{
	kprintf("syscall %d(%x, %x, %x, %x)!\n", udata.u_callno,
		udata.u_argn, udata.u_argn1, udata.u_argn2, udata.u_argn3);
	for (;;);
}

