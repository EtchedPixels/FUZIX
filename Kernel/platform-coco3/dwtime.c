/* Drivewire Time Keeping */

#include <kernel.h>
#include <drivewire.h>
#include <printf.h>

#define DISC __attribute__((section(".discard")))


struct my_uint32_t {
	uint16_t hi;
	uint16_t low;
};



static const uint16_t mktime_moffset[12]= { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static int get_time( char *tbuf )
{
	int ret;

	tbuf[0]=0x23;
	ret=dw_transaction( tbuf, 1, tbuf, 6 );
	if( ret ) return -1;

	return 0;
}


/* shift 32 to left */
DISC void shift_left ( struct my_uint32_t *a )
{
	uint16_t test=a->low & 0x8000;
	a->low <<= 1;
	a->hi <<= 1;
	if( test ) a->hi |= 1;
	return;
}

/* accumulate b into a */
DISC void acc( struct my_uint32_t *a, struct my_uint32_t *b )
{
	uint16_t t;
	t = a->low + b->low;
	if( t < a->low || t < b->low )
		a->hi += 1;
	a->low = t;
	a->hi = a->hi + b->hi;
}

DISC void acc0( struct my_uint32_t *b )
{
	memset( b, 0, sizeof( struct my_uint32_t ) );
}



DISC void multiply_8x32( int8_t a, struct my_uint32_t *b)
{
	struct my_uint32_t r;
	acc0( &r );
	while (a)
		{
			if (a & 1)
				acc( &r, b );
			a >>= 1;
			shift_left( b );
		}
	memcpy( b, &r, sizeof( struct my_uint32_t ) );
}

DISC void acc16( struct my_uint32_t *a, int16_t b )
{
	uint16_t t;
	t = b + a->low;
	if( t < b || t < a->low )
		a->hi += 1;
	a->low = t ;
}


DISC int dwtime_init( void )
{
	char buffer[6];
	struct my_uint32_t tbuf;
	struct my_uint32_t *tptr=&tbuf;

	acc0( tptr );

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
	   https://github.com/jbruchon/elks/blob/master/elkscmd/sh_utils/date.c */
	
	/* uses zero-based month index */
	month--;
	
	/* calculate days from years */
	tbuf.low=365;
	multiply_8x32(year - 70, tptr);
	
	/* count leap days in preceding years */
	acc16( tptr, (year - 69) >> 2 );
	
	/* calculate days from months */
	acc16( tptr,  mktime_moffset[month] );
	
	/* add in this year's leap day, if any */
	if (((year & 3) == 0) && (month > 1)) {
		acc16( tptr, 1 );
	}
	
	/* add in days in this month */
	acc16( tptr, day - 1);
	/* convert to hours */
	multiply_8x32(24, tptr);
	acc16( tptr, hour);
	
	/* convert to minutes */
	multiply_8x32(60, tptr);
	acc16( tptr, minute);
	
	/* convert to seconds */
	multiply_8x32(60, tptr);
	acc16( tptr, second);
	

	/* Set Kernel's TOD */
	{
		uint16_t tod[4];
		tod[0]=tbuf.hi;
		tod[1]=tbuf.low;
		tod[2]=0;
		tod[3]=0;
		wrtime((time_t *)tod);
	}
	return 0;
}






