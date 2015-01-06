/*************************** CTIME ************************************/  
    
#include <time.h>
#include <string.h>

char *ctime(time_t * timep) 
{
	return asctime(localtime(timep));
}
