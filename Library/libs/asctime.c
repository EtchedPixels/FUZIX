#include <time.h>
#include <string.h>

/* asctime - convert date and time to ascii.
 * returns a pointer to the character string containing the date and time.
 */
char *asctime(struct tm *timeptr)
{
	static char timebuf[26];

	if (timeptr == 0)
		return 0;
	return asctime_r(timeptr, timebuf);
}
