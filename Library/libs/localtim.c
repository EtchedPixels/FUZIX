#include <time.h>
#include <string.h>

struct tm *localtime(time_t * timep)
{
	static struct tm tmb;
	__tm_conv(&tmb, timep, (int) (timezone / 60));
	return &tmb;
}
