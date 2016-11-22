/* Drivewire Time Keeping */

#include <kernel.h>
#include <drivewire.h>
#include <printf.h>
#include <kdata.h>
#include <ttydw.h>
#include "config.h"

#define DISC __attribute__((section(".discard")))

static uint8_t secs;

static const uint16_t mktime_moffset[12]= { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static int get_time( uint8_t *tbuf )
{
	int ret;
	
	if ( dwtype == DWTYPE_NOTFOUND )
		return -1;
	tbuf[0]=0x23;
	ret=dw_transaction( tbuf, 1, tbuf, 6, 0 );
	if( ret ) return -1;
	secs = tbuf[5];
	return 0;
}


/* A Classic, as stolen from Gnu */
DISC static uint32_t mul( uint32_t a, uint32_t b)
{
	uint32_t r=0;
	while (a){
		if (a & 1)
			r += b;
		a >>= 1;
		b <<= 1;
	}
	b=r;
	return b;
}



DISC int dwtime_init( void )
{
	uint8_t buffer[6];
	uint32_t ret;

	/* get time packet */
	if ( get_time( buffer ) ) return -1;
	/* figure out secs from epoc */
	uint8_t year, month, day, hour, minute, second;

	year   = buffer[0];
	month  = buffer[1];
	day    = buffer[2];
	hour   = buffer[3];
	minute = buffer[4];
	second = buffer[5];

	if(year < 70)
		year += 100;

	/* following code is based on utc_mktime() from ELKS
	   https://github.com/jbruchon/elks/blob/master/elkscmd/sh_utils/date.c 
	*/

	/* uses zero-based month index */
	month--;

	/* calculate days from years */
	ret=365;
	ret = mul( ret, year - 70 );

	/* count leap days in preceding years */
	ret += (year - 69) >> 2;
	

	/* calculate days from months */
	ret += mktime_moffset[month];

	/* add in this year's leap day, if any */
	if (((year & 3) == 0) && (month > 1)) 
		ret++;

	/* add in days in this month */
	ret += day - 1;
	/* convert to hours */
	ret = mul( ret, 24 );
	ret += hour;

	/* convert to minutes */
	ret = mul( ret, 60 );
	ret += minute;

	/* convert to seconds */
	ret = mul( ret, 60 );
	ret += second;

	tod.low=ret;
	tod.high=0;

	return 0;
}



/* Called every every decisec from timer.c */
uint8_t rtc_secs(void)
{
	static uint8_t ticks=10;
	static uint8_t longt=CONFIG_DWTIME_INTERVAL;
	uint8_t t[6];

	/* count to 10 deci's before doing anything */
	if( --ticks )
		return secs;
	ticks=10;
	/* count to INTERVAL before polling */
	if( --longt ) 
		goto nopoll;
	longt=CONFIG_DWTIME_INTERVAL;
	/* poll dw time and return it */
	if( get_time( t ) ) goto nopoll;
	return secs;
	
 nopoll:
	if( (++secs) > 59 ) secs = 0;
	return secs;
}


