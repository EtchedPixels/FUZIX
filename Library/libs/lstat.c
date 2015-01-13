#include <unistd.h>
#include <fcntl.h>
#include <syscalls.h>

int lstat(const char *name, struct stat *buf)
{
	int sts, fd = open(name, O_SYMLINK|O_RDONLY);

	if (fd < 0)
		sts = stat(name, buf);
	else {
		sts = fstat(fd, buf);
		close(fd);
	}
	return sts;
}
