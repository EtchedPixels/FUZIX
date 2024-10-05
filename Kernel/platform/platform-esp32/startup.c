#include "soc/gpio_struct.h"
#include "soc/rtc_cntl_struct.h"
#include "soc/timer_group_struct.h"
#include "soc/dport_reg.h"
#include "soc/dport_access.h"
#include "globals.h"
#include "xtos.h"

extern uint8_t _stext_start;
extern uint8_t _text_start;
extern uint8_t _text_end;

extern uint8_t _sdata_start;
extern uint8_t _data_start;
extern uint8_t _data_end;

extern uint8_t _bss_start;
extern uint8_t _bss_end;

void startup(void)
{
	/* Disable the external memory. */

	DPORT_REG_WRITE(DPORT_PRO_CACHE_CTRL_REG, 0);
	DPORT_REG_WRITE(DPORT_APP_CACHE_CTRL_REG, 0);

	/* Disable the watchdog timers. */

	TIMERG0.wdtconfig0.val = 0;
	RTCCNTL.wdt_config0.val = 0;
	
	/* Relocate. */

	xthal_memcpy(&_text_start, &_stext_start, &_text_end - &_text_start);
	xthal_memcpy(&_data_start, &_sdata_start, &_data_end - &_data_start);
	bzero(&_bss_start, &_bss_end - &_bss_start);

	/* Start the application core. */

	ets_set_appcpu_boot_addr(appcore_main);
	DPORT_REG_SET_BIT(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);
	DPORT_REG_SET_BIT(DPORT_APPCPU_CTRL_C_REG, 0);
	DPORT_REG_CLR_BIT(DPORT_APPCPU_CTRL_A_REG, DPORT_APPCPU_RESETTING);
	DPORT_REG_SET_BIT(DPORT_APPCPU_CTRL_B_REG, DPORT_APPCPU_CLKGATE_EN);

	/* Move on to the relocated code. */

	procore_main();
}

