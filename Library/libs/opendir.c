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
	register DIR *dir;

	if (stat(path, &statbuf) != 0)
		goto Err;
	if ((statbuf.st_mode & S_IFDIR) == 0) {
		errno = ENOTDIR;
		goto Err;
	}
	if ((dir = (DIR *) calloc(1, sizeof(DIR))) == NULL) {
		errno = ENOMEM;
		goto Err;
	}
	if ((dir->dd_buf = calloc(1, sizeof(struct dirent))) == NULL) {
		free(dir);
		errno = ENOMEM;
		goto Err;
	}
	if ((dir->dd_fd = open(path, O_RDONLY)) < 0) {
		free(dir->dd_buf);
		free(dir);
	      Err:return NULL;
	}
	return dir;
}
