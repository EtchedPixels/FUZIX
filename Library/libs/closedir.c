/* close.c      closedir implementation
 *
 */
#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int closedir(DIR * dir)
{
	if (dir == NULL || dir->dd_buf == NULL || dir->dd_fd == 0) {
		errno = EFAULT;
		return -1;
	}
	close(dir->dd_fd);
	free(dir->dd_buf);
	dir->dd_fd = 0;
	dir->dd_buf = NULL;
	free(dir);
	return 0;
}
