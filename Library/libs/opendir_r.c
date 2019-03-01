#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

DIR *opendir_r(DIR *dir, const char *path)
{
	struct stat statbuf;

	if (stat(path, &statbuf) != 0)
		return NULL;

	if ((statbuf.st_mode & S_IFDIR) == 0) {
		errno = ENOTDIR;
		return NULL;
	}
	if ((dir->dd_fd = open(path, O_RDONLY | O_CLOEXEC)) < 0)
		return NULL;
	dir->dd_loc = 0;
	dir->_priv.next = 0;
	dir->_priv.last = 0;
	return dir;
}
