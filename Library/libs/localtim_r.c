#include <time.h>
#include <string.h>

/* Slight duplication here so gmtime doesn't suck in tz and all the rest */
#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600

static void __tm_conv_local(struct tm *tmbuf, time_t * pt)
{
	long days;		/* This breaks but not for a while 8) */
	long rem;
	long shift;

	days = *pt / SECS_PER_DAY;
	rem = *pt % SECS_PER_DAY;

	/* Assume not DST for initial maths */
	rem += timezone;

	/* This loop normally only happens once max */
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}

	__compute_tm(tmbuf, days, rem);

	shift = __is_dst(tmbuf, rem) - timezone;
	/* Recalculate effect of DST */
	if (shift) {
		rem += shift;
		/* This loop normally only happens once max */
		while (rem < 0) {
			rem += SECS_PER_DAY;
			--days;
		}
		while (rem >= SECS_PER_DAY) {
			rem -= SECS_PER_DAY;
			++days;
		}
		__compute_tm(tmbuf, days, rem);
	}
}



struct tm *localtime_r(time_t * timep, struct tm *result)
{
	tzset();
	__tm_conv_local(result, timep);
	return result;
}
