#include "soc/rtc_cntl_reg.h"
#include "soc/timer_group_reg.h"
#include "soc/dport_reg.h"

/* The boot loader only appears to be able to reliably load code in the
 * SRAM2/SRAM0 MMU areas, where we're eventually going to want to put our
 * processes. So, once everything's been loaded into memory, we need to
 * relocate ourselves into our final location (SRAM2/SRAM1). Remember that at
 * this point the whole kernel is in instruction memory and only 32-bit
 * accesses will work. */

	.section .text.relocate
.globl __relocate
__relocate:
	# This must be the first word in the output file.

	j 1f
	.literal_position
1:

	# Turn off interrupts.

	rsil a2, 15

	# Disable the external memory.

	movi a2, DPORT_PRO_CACHE_CTRL_REG
	movi a3, 0
	s32i a3, a2, 0

	movi a2, DPORT_APP_CACHE_CTRL_REG
	s32i a3, a2, 0

	# Disable the watchdog timers.

	movi a3, 0

	movi a2, TIMG_WDTCONFIG0_REG(0)
	s32i a3, a2, 0

	movi a2, RTC_CNTL_WDTCONFIG0_REG
	s32i a3, a2, 0
	
	# Copy text segment.

	movi a2, _stext_start # source
	movi a3, _text_start  # destination
	movi a4, _text_end
1:
	bgeu a3, a4, 2f
	l32i a5, a2, 0
	s32i a5, a3, 0
	addi a2, a2, 4
	addi a3, a3, 4
	j 1b
2:

	# Copy data segment.

	movi a2, _sdata_start # source
	movi a3, _data_start  # destination
	movi a4, _data_end
1:
	bgeu a3, a4, 2f
	l32i a5, a2, 0
	s32i a5, a3, 0
	addi a2, a2, 4
	addi a3, a3, 4
	j 1b
2:

	# Clear BSS segment.

	movi a3, _bss_start
	movi a4, _bss_end
	movi a5, 0
1:
	bgeu a3, a4, 2f
	s32i a5, a3, 0
	addi a3, a3, 4
	j 1b
2:

	# Initialise the application core.

	movi a2, DPORT_APPCPU_CTRL_D_REG
	movi a3, __app_cpu_entry
	s32i a3, a2, 0

	movi a2, DPORT_APPCPU_CTRL_B_REG
	movi a3, DPORT_APPCPU_CLKGATE_EN
	s32i a3, a2, 0

	movi a2, DPORT_APPCPU_CTRL_A_REG
	movi a3, DPORT_APPCPU_RESETTING
	s32i a3, a2, 0
	movi a3, 0
	s32i a3, a2, 0

	movi a2, DPORT_APPCPU_CTRL_C_REG
	s32i a3, a2, 0

	# Start the main protocol core code.

	j __pro_cpu_entry

	.text

__pro_cpu_entry:
	movi sp, prostack+256
	j __pro_cpu_main

__app_cpu_entry:
	movi sp, appstack+256
	j __app_cpu_main
	
.comm prostack, 256
.comm appstack, 256

