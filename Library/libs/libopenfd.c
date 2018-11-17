#include <sys/library.h>
#include <fcntl.h>
#include <unistd.h>

int libopenfd(struct library *lib, int fd, int minver)
{
	if (read(fd, lib, sizeof(struct library)) != sizeof(struct library))
		return -1;
	if (lib->magic != LIB_MAGIC)
		return -1;
	if (lib->version < minver)
		return -1;
	lib->fd = fd;
	return 0;
}

