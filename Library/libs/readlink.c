/* readlink.c	readlink implementation for UZIX
 */  
#include <unistd.h>
#include <fcntl.h>
#include <syscalls.h>

int readlink(const char *name, char *buf, int size) 
{
	int sts, fd = open(name, O_SYMLINK|O_RDONLY);

	if (fd < 0)
		return -1;
	sts = read(fd, buf, size);
	close(fd);
	return sts;
}
