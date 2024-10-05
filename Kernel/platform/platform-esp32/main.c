#include "kernel.h"
#include "kdata.h"
#include "printf.h"
#include "soc/gpio_struct.h"
#include "kernel-esp32.def"
#include "xtos.h"

#define LED 2

uint8_t sys_cpu;
uint8_t sys_cpu_feat;

void set_cpu_type(void)
{
	sys_cpu = A_LX6;
}

/* On entry we're running on the app core default stack, which is owned by
 * XTOS. We'll switch to a process stack when the first process starts and then
 * never touch it again.
 */
void __attribute__((noreturn)) appcore_main(void) {
	gpio_dev_t* gpio = &GPIO;
	gpio->func_out_sel_cfg[LED].func_sel = 0x100;
	gpio->enable_w1ts = 1<<LED;
	gpio->out_w1ts = 1<<LED;

	/* Check offsets. */

	if ((U_DATA__U_SP != offsetof(struct u_data, u_sp)) ||
	    (U_DATA__U_PTAB != offsetof(struct u_data, u_ptab)) ||
	    (U_DATA__U_CALLNO != offsetof(struct u_data, u_callno)) ||
	    (P_TAB__P_PID_OFFSET != offsetof(struct p_tab, p_pid)) ||
		(P_TAB__P_STATUS_OFFSET != offsetof(struct p_tab, p_status)))
	{
		kprintf("U_DATA__U_SP = %d\n", offsetof(struct u_data, u_sp));
		kprintf("U_DATA__U_PTAB = %d\n", offsetof(struct u_data, u_ptab));
		kprintf("U_DATA__U_CALLNO = %d\n", offsetof(struct u_data, u_callno));
		kprintf("P_TAB__P_PID_OFFSET = %d\n", offsetof(struct p_tab, p_pid));
		kprintf("P_TAB__P_STATUS_OFFSET = %d\n", offsetof(struct p_tab, p_status));
		panic("bad offsets");
	}

	ramsize = 256;
	procmem = 256;
	kputs("\n\n\n");
	sys_cpu_feat = AF_LX6_ESP32;

	/* And off we go */
	fuzix_main();
}

uint16_t swap_dev = 0xffff;
uint8_t need_resched;

void map_init(void)
{
	panic("map_init");
}

void plt_discard(void) {}

void plt_monitor(void)
{
	while(1)
		asm volatile ("waiti 15");
}

void plt_reboot(void)
{
	panic("reboot");
}

uint_fast8_t plt_param(char* p)
{
	return 0;
}

