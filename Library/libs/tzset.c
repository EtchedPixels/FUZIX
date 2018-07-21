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
 *	FIXME: The spec says hh:mm:ss, the real world appears fine with the
 *	hours bit being a single digit.
 */

void tzset(void)
{
        char *tz = getenv("TZ");
        char *p, *pe;

        memcpy(tzbuf0, "GMT", 4);
        timezone = 0 * 60 * 60L;        /* London */
        if (tz == NULL)
                return;
        p = tzbuf0;
        pe = p + 5;
        while(*tz && isalpha(*tz)) {
                if (p < pe)
                        *p++ = *tz;
                tz++;
        }
        tzbuf0[5] = 0;
        parse_tzspec(tz, &timezone);
}
