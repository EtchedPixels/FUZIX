#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <time.h>


typedef unsigned long suseconds_t;

struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};

extern int utimes(const char *filename, const struct timeval times[2]);

#ifdef _BSD_SOURCE

extern int gettimeofday(struct timeval *tv, struct timezone *tz);
extern int settimeofday(struct timeval *tv, const struct timezone *tz);

/* These are courtesy of Linux. Complete with the usual bugs. The only
   change here is to use L for the long types. */
# define timerisset(tvp)	((tvp)->tv_sec || (tvp)->tv_usec)
# define timerclear(tvp)	((tvp)->tv_sec = (tvp)->tv_usec = 0)
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))
# define timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000L)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000L;					      \
      }									      \
  } while (0)
# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000L;					      \
    }									      \
  } while (0)

#endif	/* Berkleyisms */

#endif
