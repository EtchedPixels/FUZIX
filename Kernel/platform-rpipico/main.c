#include <kernel.h>
#include <kdata.h>
#include "picosdk.h"
#include "kernel-armm0.def"
#include <hardware/sync.h>
#include "globals.h"
#include "printf.h"

uint_fast8_t platform_param(char* p)
{
	return 0;
}

void platform_idle(void)
{
	__wfi();
}

void syscall_handler(struct exception_frame* eh)
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
	#if 0
	if ((U_DATA__U_SP_OFFSET != offsetof(struct u_data, u_sp)) ||
		(U_DATA__U_PTAB_OFFSET != offsetof(struct u_data, u_ptab)) ||
		(P_TAB__P_PID_OFFSET != offsetof(struct p_tab, p_pid)) ||
			(P_TAB__P_STATUS_OFFSET != offsetof(struct p_tab, p_status)))
	{
		kprintf("U_DATA__U_SP = %d\n", offsetof(struct u_data, u_sp));
		kprintf("U_DATA__U_PTAB = %d\n", offsetof(struct u_data, u_ptab));
		kprintf("P_TAB__P_PID_OFFSET = %d\n", offsetof(struct p_tab, p_pid));
		kprintf("P_TAB__P_STATUS_OFFSET = %d\n", offsetof(struct p_tab, p_status));
		panic("bad offsets");
	}
	#endif

	ramsize = 64;
	procmem = 64;
    //sys_cpu_feat = AF_LX106_ESP8266;

	stdio_init_all();
	di();
	fuzix_main();

#if 0
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
		puts("Hello, world!");

        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
#endif
}

/* vim: sw=4 ts=4 et: */


