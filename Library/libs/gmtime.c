#include <time.h>
#include <string.h>

struct tm *gmtime(time_t *timep)
{
	static struct tm tmb;
	return gmtime_r(timep, &tmb);
}
