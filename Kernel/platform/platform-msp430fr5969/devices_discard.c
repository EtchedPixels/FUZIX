#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <tty.h>
#include <devsd.h>
#include <blkdev.h>
#include <printf.h>
#include <timer.h>
#include "msp430fr5969.h"
#include "externs.h"

void device_init(void)
{
	/* Configure the watchdog timer to use ACLK as the system interrupt.
	 * ACLK was set up in the boot sequence to use the LFXT clock, which runs
	 * (relatively accurately) at 32kHz. 512 ticks at 32kHz is 64Hz.
	 */

	WDTCTL = WDTPW | WDTSSEL__ACLK | WDTTMSEL | WDTCNTCL | WDTIS__512;
	SFRIE1 |= WDTIE;

	sd_rawinit();
	devsd_init();
}

