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

extern clock_t clock __P ((void));
extern time_t mktime __P ((struct tm * __tp));
extern double difftime __P ((time_t *__time2, time_t *__time1));


extern void __tm_conv __P((struct tm *tmbuf, time_t *t, int offset));
extern char *asctime __P ((struct tm * __tp));
extern char *asctime_r __P((struct tm *, char * __buf));
extern char *ctime __P ((time_t * __tp));
extern char *ctime_r __P ((time_t * __tp, char * __buf));
extern void tzset __P ((void));

extern struct tm *gmtime __P ((time_t *__tp));
extern struct tm *localtime __P ((time_t * __tp));
extern struct tm *gmtime_r(time_t *tvec, struct tm *result);
extern struct tm *localtime_r(time_t *tvec, struct tm *result);

typedef int clockid_t;

#define CLOCKS_PER_SEC	100		/* FIXME: sysconf */

#define CLOCK_REALTIME	0
#define CLOCK_MONOTONIC 1

extern int clock_getres(clockid_t clk_id,  struct timespec *res);
extern int clock_gettime(clockid_t clk_id,  struct timespec *tp);
extern int clock_nanosleep(clockid_t clk_id, int flags,
	const struct timespec *request, struct timespec *remain);
extern int clock_settime(clockid_t clk_id,  const struct timespec *tp);

#define TIMER_ABSTIME	1

extern int nanosleep(const struct timespec *request, struct timespec *remain);

#endif
