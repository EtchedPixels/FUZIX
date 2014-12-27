/* sleep.c
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

unsigned int sleep(unsigned int seconds)
{
	time_t end, now;
	_time(&end, 0);
	end += seconds;
	if (_pause(seconds * 10) == 0)
		return 0;
	_time(&now, 0);
	return (int)(end - now);
}
