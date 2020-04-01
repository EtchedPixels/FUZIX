/* isatty.c
 */
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int isatty(int fd)
{
	pid_t tmp;
	if (ioctl(fd, TIOCGPGRP, &tmp) == -1)
		return 0;
	return 1;
}
