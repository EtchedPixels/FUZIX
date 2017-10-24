/*************************** CLOCK ************************************/

#include <types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/times.h>

clock_t clock(void)
{
	struct tms __tms;
	times(&__tms);
	/* Already correctly scaled */
	return __tms.tms_utime;
}
