/*************************** TZSET ************************************/

#include <time.h>
#include <string.h>
#include <stdlib.h>

char *tzname[2] = { "GMT", "\0\0\0" };

int daylight;
long timezone;

/* tzset expects fo find a environment string of the form TZ=...
 * ??? need to correct!
 */
void tzset(void)
{
        char *tz = getenv("TZ");

        if (tz == NULL) {
                memcpy(tzname[1], "GMT", 3);
                timezone = 0 * 60 * 60L;        /* London */
        } else {
                int v;

                memcpy(tzname[1], tz, 3);
                v = atoi(tz + 3);
                timezone = -((v / 100) * 60 + (v % 100)) * 60L;
        }
}
