/*************************** TZSET ************************************/

#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

char *tzname[2] = { "GMT", "\0\0\0" };

int daylight;
long timezone;

/*
 *	POSIX has much to say about timezones and the implementations of it
 *	are not exactly small. Old Unix didn't do anything as fancy as this
 *
 *	We parse stuff the old way. We expect a timezone followed by a
 *	shift specified in hours (and yes if you are in a half hour timezone
 *	you get to write code)
 */
void tzset(void)
{
        char *tz = getenv("TZ");

        memcpy(tzname[1], "GMT", 3);
        timezone = 0 * 60 * 60L;        /* London */
        if (tz == NULL)
                return;
        while(*tz && isalpha(*tz))
                tz++;
        timezone = atoi(tz);
        timezone *= 3600;
}
