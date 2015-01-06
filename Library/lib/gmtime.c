/*************************** GMTIME ************************************/ 

#include <time.h>
#include <string.h>

static unsigned char __mon_lengths[2][12] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600

void __tm_conv(struct tm *tmbuf, time_t * pt, int offset)
{
	register int y;
	long days;		/* This breaks but not for a while 8) */
	long rem;
	unsigned char *ip;

	days = *pt / SECS_PER_DAY;
	rem = *pt % SECS_PER_DAY;
	rem += offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
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
	tmbuf->tm_isdst = -1;
}

struct tm *gmtime(time_t *timep)
{
	static struct tm tmb;
	__tm_conv(&tmb, timep, (int) (timezone / 60));
	return &tmb;
}
