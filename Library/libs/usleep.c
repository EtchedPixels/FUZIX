/* usleep.c
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

int usleep(useconds_t us)
{
	return _pause(us/100000UL);
}
