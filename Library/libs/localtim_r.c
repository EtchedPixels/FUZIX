#include <time.h>
#include <string.h>

struct tm *localtime_r(time_t * timep, struct tm *result)
{
	tzset();
	__tm_conv(result, timep, timezone);
	return result;
}
