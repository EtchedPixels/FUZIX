#include <kernel.h>
#include <kdata.h>
#include "picosdk.h"
#include "kernel-armm0.def"
#include "globals.h"
#include "printf.h"
#include "core1.h"

uint_fast8_t platform_param(char* p)
{
	return 0;
}

void fatal_exception_handler(struct extended_exception_frame* eh)
{
    kprintf("FLAGRANT SYSTEM ERROR! EXCEPTION %d\n", eh->cause);
    kprintf(" r0=%p r1=%p  r2=%p  r3=%p\n", eh->r0, eh->r1, eh->r2, eh->r3);
    kprintf(" r4=%p r5=%p  r6=%p  r7=%p\n", eh->r4, eh->r5, eh->r6, eh->r7);
    kprintf(" r8=%p r9=%p r10=%p r11=%p\n", eh->r8, eh->r9, eh->r10, eh->r11);
    kprintf("r12=%p sp=%p  lr=%p  pc=%p\n", eh->r12, eh->sp, eh->lr, eh->pc);
    kprintf("PROGBASE=%p PROGLOAD=%p PROGTOP=%p\n", PROGBASE, PROGLOAD, PROGTOP);
    kprintf("UDATA=%p KSTACK=%p-%p\n", &udata, &udata+1, ((uint32_t)&udata) + UDATA_SIZE);
    kprintf("user mode relative: lr=%p pc=%p isp=%p brk=%p\n",
        eh->lr-PROGLOAD, eh->pc-PROGLOAD, udata.u_isp, udata.u_break);
    panic("fatal exception");
}

void syscall_handler(struct svc_frame* eh)
{
    udata.u_callno = *(uint8_t*)(eh->pc - 2);
    udata.u_argn = eh->r0;
    udata.u_argn1 = eh->r1;
    udata.u_argn2 = eh->r2;
    udata.u_argn3 = eh->r3;
    udata.u_insys = 1;

    unix_syscall();

    udata.u_insys = 1;
    eh->r0 = udata.u_retval;
    eh->r1 = udata.u_error;
}

int main(void)
{
    core1_init();

	if ((U_DATA__U_SP_OFFSET != offsetof(struct u_data, u_sp)) ||
		(U_DATA__U_PTAB_OFFSET != offsetof(struct u_data, u_ptab)) ||
		(P_TAB__P_PID_OFFSET != offsetof(struct p_tab, p_pid)) ||
        (P_TAB__P_STATUS_OFFSET != offsetof(struct p_tab, p_status)) ||
        (UDATA_SIZE_ASM != UDATA_SIZE))
	{
		kprintf("U_DATA__U_SP = %d\n", offsetof(struct u_data, u_sp));
		kprintf("U_DATA__U_PTAB = %d\n", offsetof(struct u_data, u_ptab));
		kprintf("P_TAB__P_PID_OFFSET = %d\n", offsetof(struct p_tab, p_pid));
		kprintf("P_TAB__P_STATUS_OFFSET = %d\n", offsetof(struct p_tab, p_status));
		panic("bad offsets");
	}

	ramsize = (SRAM_END - SRAM_BASE) / 1024;
	procmem = USERMEM / 1024;

	di();
	fuzix_main();
}

/* vim: sw=4 ts=4 et: */


