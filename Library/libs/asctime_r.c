#include <time.h>
#include <string.h>

/*
 * Internal ascii conversion routine, avoid use of printf, it's a bit big!
 */

static void hit(char *buf, int val)
{
	*buf = '0' + val % 10;
}


char *asctime_r(struct tm *ptm, char *buffer)
{
	static const char days[21] = "SunMonTueWedThuFriSat";
	static const char mons[36] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	int year;

	strcpy(buffer, "Err Err .. ..:..:.. ....\n");
	if ((ptm->tm_wday >= 0) && (ptm->tm_wday <= 6))
		memcpy(buffer, days + 3 * (ptm->tm_wday), 3);
	if ((ptm->tm_mon >= 0) && (ptm->tm_mon <= 11))
		memcpy(buffer + 4, mons + 3 * (ptm->tm_mon), 3);
	year = ptm->tm_year + 1900;
	hit(buffer + 8, ptm->tm_mday / 10);
	hit(buffer + 9, ptm->tm_mday);
	hit(buffer + 11, ptm->tm_hour / 10);
	hit(buffer + 12, ptm->tm_hour);
	hit(buffer + 14, ptm->tm_min / 10);
	hit(buffer + 15, ptm->tm_min);
	hit(buffer + 17, ptm->tm_sec / 10);
	hit(buffer + 18, ptm->tm_sec);
	hit(buffer + 20, year / 1000);
	hit(buffer + 21, year / 100);
	hit(buffer + 22, year / 10);
	hit(buffer + 23, year);
	return buffer;
}
