/* readdir.c    readdir implementation
 *
 */
#include <unistd.h>
#include <alloc.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

struct dirent *readdir(DIR * dir)
{
	struct __dirent direntry;
	register struct dirent *buf;
	int len;

	if (dir == NULL || dir->dd_buf == NULL || dir->dd_fd == 0) {
		errno = EFAULT;
		return NULL;
	}
	direntry.d_name[0] = 0;
	while (direntry.d_name[0] == 0) {
	        len = _getdirent(dir->dd_fd, &direntry, sizeof(direntry));
	        if (len > sizeof(direntry)) {
			errno = ERANGE;
			return NULL; 
		}
		if (len == 0)
			return NULL;
	}
	buf = dir->dd_buf;
	buf->d_ino = direntry.d_ino;
	buf->d_off = -1;	/* FIXME */
	buf->d_reclen = len + 1;
	strncpy(buf->d_name, (char *) direntry.d_name, len - 2);
	buf->d_name[len - 1] = 0;
	return buf;
}
