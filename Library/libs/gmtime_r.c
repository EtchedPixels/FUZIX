/*************************** GMTIME ************************************/ 

#include <time.h>
#include <string.h>

struct tm *gmtime_r(time_t *timep, struct tm *result)
{
	__tm_conv(result, timep, (int) (timezone / 60));
	return result;
}
