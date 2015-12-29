/* closedir_r.c      closedir_r implementation
 *
 */
#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int closedir_r(DIR * dir)
{
	if (dir == NULL || dir->dd_fd == -1) {
		errno = EBADF;
		return -1;
	}
	close(dir->dd_fd);
	dir->dd_fd = -1;
	return 0;
}
