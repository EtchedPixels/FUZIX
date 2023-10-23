/*************************** TZSET ************************************/
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static char tzbuf0[6] = "GMT";
static char tzbuf1[6];

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

static uint8_t tzset_done;

static uint8_t dpair(char **p, unsigned long *v, uint16_t mul)
{
        register char *x = *p;
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
        uint_fast8_t negate  = 0, r;
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

static char *tzunit(char *inp, char *tzstr, long *p, int required)
{
        register char *in = inp;
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

static char *parse_dst(char *p, int n)
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
		/* M then m.w.d values */
		/* Keep dsmon 0 based to match tm */
		dsmon[n] = strtoul(p + 1, &p, 0) - 1 ;
                if (*p++ != '.')
                        return NULL;
                /* Week number */
                dswom[n] = strtoul(p, &p, 0);
                if (*p++ != '.')
                        return NULL;
                /* Day of week */
                dsdow[n] = strtoul(p, &p, 0);
                dsmode = 3;
	}
	if (*p != '/') {
		dstime[n] = 2 * 3600;	/* 02:00 is the default */
		return p;
	}
	np = parse_tzspec(p + 1, &dstime[n]);
	if (np == NULL) {
		dstime[n] = 0L;
		np = p + 1;
	}
	return np;
}	

void tzset(void)
{
        char *tz = getenv("TZ");
        register char *p;

        if (tzset_done)
                return;

        tzset_done = 1;
        memcpy(tzbuf0, "GMT", 4);
        timezone = 0 * 60 * 60L;        /* London */
        dsmode = 0;			/* No daylight savings */
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
	if (p == NULL || *p++ != ',')
		return;
	parse_dst(p, 1);
}

/* Convert the week/day rule into a day of month for this year

   It's a bit odd as the meaning is:
           nth: 1		First day 'd' in the month
           nth: 2-4		Day 'd' in week 'nth' of month
           nth: 5		Last day 'd' in the month
 */

static uint8_t do_find_day(struct tm *tm, uint_fast8_t nth, uint_fast8_t d, uint_fast8_t leap)
{
        extern const uint8_t __mon_lengths[2][12];
        /* Note mday is 1 based wday is 0 based */
        /* Find the day number of the 1st */
        int_fast8_t sday = tm->tm_mday - tm->tm_wday;
        /* nth == 1 means 'first occurence of' */
        if (sday < 0 && nth == 1)
                sday += 7;
        else
                sday %= 7;
        sday += d;
        /* We now have the day of month of the first instance */
        sday += 7 * (nth - 1);
        /* But are we beyond the end of month ? If so handle the nth 5
           case and report the last actual occurrence */
        if (sday >= __mon_lengths[leap][tm->tm_mon])
                sday -= 7;
        return sday;
}

static void convert_dow(struct tm *tm)
{
        uint_fast8_t leap = __isleap(1900+tm->tm_year);
        dsday[0] = do_find_day(tm, dswom[0], dsdow[0], leap);
        dsday[1] = do_find_day(tm, dswom[1], dsdow[1], leap);
}
        
/*
 *	tm is the time structure parsed assuming no local time. secs is
 *	the number of seconds of the day that have passed.
 *
 *	We return 0 if it's not summer time, 1 if it is, -1 if we don't know
 */
static int8_t calc_is_dst(struct tm *tm, uint32_t secs)
{
	uint16_t yday = tm->tm_yday;
	uint_fast8_t south = 0;
	
	switch(dsmode) {
	default:
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
		/* Southern hemisphere ? */
		if (dsday[0] > dsday[1])
		        south = 1;
        	if (yday > dsday[0] && yday < dsday[1])
			return !south;
		if (yday < dsday[1] || yday > dsday[0])
			return south;

                /* Right day not late enough */
                if (yday == dsday[0]) {
                        if (secs >= dstime[0])
                                return !south;
                        return south;
                }
                /* Right day, early enough */
                if (secs < dstime[1])
                        return !south;
		return south;
	/* We don't yet support the complex format */
	case 3:
	        /* No timezone has two changes in the same month so we can
	           get the direction more simply */
	        if (dsmon[0] > dsmon[1])
	                south = 1;
                /* Can we tell by the month ? */
                if (tm->tm_mon < dsmon[0] || tm->tm_mon > dsmon[1])
                        return south;
                if (tm->tm_mon > dsmon[0] && tm->tm_mon < dsmon[1])
                        return !south;
                /* We are in the month it changes. Figure out the dates */
                convert_dow(tm);
                /* Now we have a day of month for this month to work with */
                if (tm->tm_mday < dsday[0] || tm->tm_mday > dsday[1])
                        return south;
                if (tm->tm_mday > dsday[0] && tm->tm_mday < dsday[1])
                        return !south;
                /* Ok so we are on one of the change days */
                if (tm->tm_mday == dsday[0]) {
                        if (secs >= dstime[0])
                                return !south;
                        return south;
                }
                if (secs >= dstime[1])
                        return south;
		return !south;
	}
}

int32_t __is_dst(struct tm *tm, uint32_t secs)
{
        tm->tm_isdst = calc_is_dst(tm, secs);
        if (tm->tm_isdst == 1)
                return dstzone;
        else	/* Not, or unknown */
                return timezone;
}
