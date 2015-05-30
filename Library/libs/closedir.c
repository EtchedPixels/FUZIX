/* closedir.c      closedir implementation
 *
 */
#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int closedir(DIR * dirp)
{
	struct _dir *dir = (struct _dir *)dirp;
	if (dir == NULL || dir->d.dd_fd == -1) {
		errno = EFAULT;
		return -1;
	}
	close(dir->d.dd_fd);
	dir->d.dd_fd = -1;
	free(dir);
	return 0;
}
