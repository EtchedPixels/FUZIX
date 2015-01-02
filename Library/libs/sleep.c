/* sleep.c
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syscalls.h>

/* Divide by ten in shifts. Would be nice if the compiler did that for us 8)

   FIXME: probably worth having a Z80 asm version of this */
static div10quicki(unsigned int i)
{
	unsigned int q, r;
	q = (i >> 1) + (i >> 2);
	q = q + (q >> 4);
	q = q + (q >> 8);
	q >>= 3;
	r = i - (((q << 2) + q) << 1);
	return q + (r >> 9);
}

unsigned int sleep(unsigned int seconds)
{
	time_t end, now;
	_time(&end, 1);	/* in 1/10ths */
	end += seconds * 10;
	if (_pause(seconds * 10) == 0)
		return 0;
	_time(&now, 1);
	return div10quicki(end - now);
}
