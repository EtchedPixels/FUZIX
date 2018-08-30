#ifndef __TIME_H
#define __TIME_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stddef.h>
#include <syscalls.h>

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

struct timezone {
	int tz_minuteswest;	/* minutes west of Greenwich */
	int tz_dsttime; 	/* type of dst correction */
};

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

#define __isleap(year)	\
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

extern char *tzname[2];
extern int daylight;
extern long timezone;

extern clock_t clock(void);
extern time_t mktime(struct tm * __tp);
#define difftime(x,y)	((double)((x)-(y)))


extern void __compute_tm(struct tm *tmbuf, long days, long rem);
extern int32_t __is_dst(struct tm *tm, uint32_t secs);
extern char *asctime(struct tm * __tp);
extern char *asctime_r(struct tm *, char * __buf);
extern char *ctime(time_t * __tp);
extern char *ctime_r(time_t * __tp, char * __buf);
extern void tzset(void);

extern struct tm *gmtime(time_t *__tp);
extern struct tm *localtime(time_t * __tp);
extern struct tm *gmtime_r(time_t *__tvec, struct tm *__result);
extern struct tm *localtime_r(time_t *__tvec, struct tm *__result);

typedef int clockid_t;

#define CLOCKS_PER_SEC	100		/* FIXME: sysconf */

#define CLOCK_REALTIME	0
#define CLOCK_MONOTONIC 1

extern int clock_getres(clockid_t __clk_id,  struct timespec *__res);
extern int clock_gettime(clockid_t __clk_id,  struct timespec *__tp);
extern int clock_nanosleep(clockid_t __clk_id, int __flags,
	const struct timespec *__request, struct timespec *__remain);
extern int clock_settime(clockid_t __clk_id,  const struct timespec *__tp);

#define TIMER_ABSTIME	1

extern int nanosleep(const struct timespec *__request, struct timespec *__remain);

#endif
