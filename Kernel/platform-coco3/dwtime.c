/* Drivewire Time Keeping */

#include <kernel.h>
#include <drivewire.h>
#include <printf.h>
#include <kdata.h>
#include <ttydw.h>
#include "config.h"


static int get_time( uint8_t *tbuf )
{
	int ret;
	
	if ( dwtype == DWTYPE_NOTFOUND )
		return -1;
	tbuf[0]=0x23;
	ret=dw_transaction( tbuf, 1, tbuf, 6, 0 );
	if( ret ) return -1;
	return 0;
}


/* Called every every decisec from timer.c */
uint8_t platform_rtc_secs(void)
{
	uint8_t t[6];
	/* poll dw time and return it */
	get_time(t);
	return t[5];
}


