#include <time.h>
#include <string.h>

struct tm *localtime(time_t * timep)
{
	static struct tm tmb;
	tzset();
	__tm_conv(&tmb, timep, timezone);
	return &tmb;
}
