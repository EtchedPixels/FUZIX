/* isatty.c
 */
#include <syscalls.h>
#include <sys/stat.h>

int isatty(int fd)
{
	struct stat stat;

	/* FIXME: should do a tty ioctl */
	if (fstat(fd, &stat) == -1 || (stat.st_mode & S_IFMT) != S_IFCHR)
		return 0;
	return 1;
}
