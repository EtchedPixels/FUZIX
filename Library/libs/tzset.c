/*************************** TZSET ************************************/

#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

char tzbuf0[6] = "GMT";
char tzbuf1[6];

char *tzname[2] = { tzbuf0, tzbuf1  };

int daylight;
long timezone;
static long dstzone;
static long dstime[2];	/* Shift for winter/summer */
static uint16_t dsday[2];
static uint8_t dsdow[2];
static uint8_t dswom[2];
static uint8_t dsmon[2];
static uint8_t dsmode;

static uint8_t dpair(char **p, unsigned long *v, uint16_t mul)
{
        char *x = *p;
        long n;
        if (!isdigit(*x))
                return 2;	/* Error */
        if (isdigit(x[1])) {
                n = (*x - '0') * 10 + (x[1] - '0');
                x++;
        } else
                n = (*x - '0');
        x++;
        n *= mul;
        *v += n;
        if (*x == ':') {
                x++;
                *p = x;
                return 0;
        }
        /* No : - this is the last field */
        *p = x;
        return 1;
}

static char *parse_tzspec(char *in, long *l)
{
        uint8_t negate  = 0, r;
        unsigned long v = 0;
        /* The offset element of a timezone is
                [+|-]hh[:mm][:ss] */
        if (*in == '-') {
                negate = 1;
                in++;
        } else if (*in == '+')
                in++;
        /* This pair is mandatory */
        r = dpair(&in, &v, 3600);
        if (r == 2)
                return NULL;
        if (r == 0)
                if (dpair(&in, &v, 60) == 0)
                        /* Back up over a final : */
                        if (dpair(&in, &v, 1) == 0)
                                in--;
        if (negate)
                *l = -v;
        else
                *l = v;
        return in;
}

/*
 *	POSIX has much to say about timezones and the implementations of it
 *	are not exactly small. Old Unix didn't do anything as fancy as this
 *
 *	We parse stuff the old way for now. For British Summer Time it's
 *	TZ=BST+01
 *
 *	We don't support the more advanced format which is
 *
 *	TZ=GMT00BST  (or BST01 - 1 hour is implied)
 *
 *	which gives a time zone and summer time variant with optional shift,
 *	but doesn't say when to apply it (which is 'magic' for the library)
 *
 *	POSIX then allows a very complex specification of exactly when time
 *	switches in terms of dates, days of week, last thursday and so on...
 *
 */

static char *tzunit(char *in, char *tzstr, long *p, int required)
{
        char *pe = tzstr + 5;
        while(*in && isalpha(*in)) {
                if (tzstr < pe)
                        *tzstr++ = *in;
                in++;
        }
        *pe = 0;
        pe = parse_tzspec(in, p);
        /* It's ok not to have the numbers bit the second time and
           it implies one hour positive */
        if (pe == NULL) {
                if (required)
                        return NULL;
                else {
                        *p = timezone + 3600;
                        return in;
                }
        }
        return pe;
        
}

char *parse_dst(char *p, int n)
{
	char *np;

        /* If we want to support the rest of the POSIX stuff we now need to
           parse the start/time and end/time fields. These are not simple
           Jn Julian date n (day 1-365, leap years ignored)
           n  Julian date n (day 0-365, leap years included)
           Mm.w.d day d of week w of month m week 5 always means 'last day d
              of the month'
           And of course we have to do something with the data!
         */
        if (*p == 'J') {
		p++;
		dsmode = 2;
		dsday[n] = strtoul(p, &p, 0);
	} else if (*p != 'M') {
		dsmode = 1;
		dsday[n] = strtoul(p, &p, 0);
	} else {
		/* The complicated one */
		/* TODO */
	}
	if (*p != '/') {
		dstime[n] = 2 * 3600;	/* 02:00 is the default */
		return p;
	}
	np = parse_tzspec(p, &dstime[n]);
	if (np == NULL) {
		dstime[n] = 0L;
		np = p;
	}
	return p;
}	

        
void tzset(void)
{
        char *tz = getenv("TZ");
        char *p;

        memcpy(tzbuf0, "GMT", 4);
        timezone = 0 * 60 * 60L;        /* London */
        if (tz == NULL)
                return;
        p = tzunit(tz, tzbuf0, &timezone, 1);
        /* Not valid */
        if (p == NULL)
                return;
        if (isalpha(*p))
                p = tzunit(p, tzbuf1, &dstzone, 0);
        /* If no comma then we are not specifying when the zone changes,
           and presumably we should look it up somewhere */
        if (*p++ != ',')
                return;
	p = parse_dst(p, 0);
	if (p == NULL || *p != ',')
		return;
	parse_dst(p, 1);
}

/*
 *	Not yet quite right: we need to consider the hour as well
 */
uint8_t __in_dst(struct tm *tm)
{
	uint16_t yday = tm->tm_yday;
	switch(dsmode) {
	case 0:
		return 0;
	case 2:
		/* Days 1-365, don't count Feb 29th on leap year. We are
		   0 based at the moment. Add 1 if it's before the 29th
		   or not a leap year, then do the normal math */
		if (yday < 30 + 28 || !__isleap(tm->tm_year + 1900))
			yday++;
		/* Fall through */
	case 1:
		/* Summer in the middle of the year */
		if (dsday[0] < dsday[1]) {
			if (yday >= dsday[0] && yday < dsday[1])
				return 1;
			return 0;
		} else {
		/* Southern hemisphere */
			if (yday < dsday[1] || yday >= dsday[0])
				return 1;
			return 0;
		}
	/* We don't yet support the complex format */
	case 3:
		return 0;
	}
}
