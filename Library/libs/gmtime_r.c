#include <time.h>
#include <string.h>

unsigned char __mon_lengths[2][12] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600

/* Do the basic computation and turn days/remainder into a tz */
void __compute_tm(struct tm *tmbuf, long days, long rem)
{
	register int y;
	unsigned char *ip;

	tmbuf->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tmbuf->tm_min = rem / 60;
	tmbuf->tm_sec = rem % 60;

	/* January 1, 1970 was a Thursday.  */
	tmbuf->tm_wday = (4 + days) % 7;
	if (tmbuf->tm_wday < 0)
		tmbuf->tm_wday += 7;
	y = 1970;
	while (days >= (rem = __isleap(y) ? 366 : 365)) {
		++y;
		days -= rem;
	}
	while (days < 0) {
		--y;
		days += __isleap(y) ? 366 : 365;
	}
	tmbuf->tm_year = y - 1900;
	tmbuf->tm_yday = days;
	ip = __mon_lengths[__isleap(y)];
	y = 0;
	while (days >= ip[y])
		days -= ip[y++];
	tmbuf->tm_mon = y;
	tmbuf->tm_mday = days + 1;
}

static void __tm_conv(struct tm *tmbuf, time_t * pt)
{
	long days;		/* This breaks but not for a while 8) */
	long rem;

	days = *pt / SECS_PER_DAY;
	rem = *pt % SECS_PER_DAY;

	__compute_tm(tmbuf, days, rem);
}


struct tm *gmtime_r(time_t *timep, struct tm *result)
{
	__tm_conv(result, timep);
	return result;
}
