#include <sys/library.h>
#include <fcntl.h>
#include <unistd.h>

int libopen(struct library *lib, const char *name, int minver)
{
	int fd = open(name, O_RDONLY);
	if (fd == -1)
		return -1;
	if (libopenfd(lib, fd, minver) == -1) {
		close(fd);
		return -1;
	}
	return 0;
}

