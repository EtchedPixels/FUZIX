/*************************** DIFFTIME **********************************/ 

#include <time.h>
#include <string.h>

double difftime(time_t * __time2, time_t * __time1)
{
	struct tm tma, tmb;
	unsigned long tm1, tm2;
	__tm_conv(&tma, __time1, 0);
	__tm_conv(&tmb, __time2, 0);
	
	/* each year is 365 days plus 8 hours = 365.25 days */ 
	tm1 =
	    tma.tm_year * 32227200L + tma.tm_yday * 86400L +
	    tma.tm_hour * 3600 + tma.tm_min * 60 + tma.tm_sec;
	tm2 =
	    tmb.tm_year * 32227200L + tmb.tm_yday * 86400L +
	    tmb.tm_hour * 3600 + tmb.tm_min * 60 + tmb.tm_sec;
	return (tm2 - tm1);
}

