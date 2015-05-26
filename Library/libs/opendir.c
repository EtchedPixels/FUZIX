#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

DIR *opendir(char *path)
{
	struct stat statbuf;
	struct _dir *dir;

	if (stat(path, &statbuf) != 0)
		return NULL;

	if ((statbuf.st_mode & S_IFDIR) == 0) {
		errno = ENOTDIR;
		return NULL;
	}
	if ((dir = calloc(1, sizeof(struct _dir))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	if ((dir->d.dd_fd = open(path, O_RDONLY | O_CLOEXEC)) < 0) {
		free(dir);
		return NULL;
	}
	return (DIR *)dir;
}
