#include <time.h>
#include <string.h>

struct tm *localtime(time_t * timep)
{
	static struct tm tmb;
	return localtime_r(timep, &tmb);
}
