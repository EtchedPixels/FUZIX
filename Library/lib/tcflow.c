#include <termios.h>
#include <unistd.h>
#include <errno.h>

int tcflow(int fd, int action)
{
	/* TODO */
	errno = EINVAL;
	return -1;
}
