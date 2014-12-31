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

#define __isleap(year)	\
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

extern char *tzname[2];
extern int daylight;
extern long timezone;

extern clock_t clock __P ((void));
extern time_t mktime __P ((struct tm * __tp));
extern double difftime __P ((time_t *__time2, time_t *__time1));

extern time_t *gtime(time_t *tvec);

extern void __tm_conv __P((struct tm *tmbuf, time_t *t, int offset));
extern void __asctime __P((char *, struct tm *));
extern char *asctime __P ((struct tm * __tp));
extern char *ctime __P ((time_t * __tp));
extern void tzset __P ((void));

extern struct tm *gmtime __P ((time_t *__tp));
extern struct tm *localtime __P ((time_t * __tp));
extern unsigned long convtime __P ((time_t *time_field));

#define CLOCKS_PER_SEC	100		/* FIXME: sysconf */
#endif
