#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

DIR *opendir(const char *path)
{
	DIR *dir = calloc(1, sizeof(DIR));
	if (dir == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	dir = opendir_r(dir, path);
	if (dir == NULL)
		free(dir);
	return dir;
}
