/*************************** CTIME ************************************/  
    
#include <time.h>
#include <string.h>

char *ctime_r(time_t * timep, char *buf) 
{
	struct tm tmtmp;
	return asctime_r(localtime_r(timep, &tmtmp), buf);
}
