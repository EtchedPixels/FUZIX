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

static struct __dirent *dnext(DIR *dir)
{
        if (dir->_priv.next == dir->_priv.last) {
                int l = read(dir->dd_fd, dir->_priv.buf, sizeof(dir->_priv.buf));
                if (l <= 0)
                        return NULL;
                l /= 32;
                dir->_priv.last = l;
                dir->_priv.next = 0;
        }
        return (struct __dirent *)(dir->_priv.buf + 32 * dir->_priv.next++);
}

struct dirent *readdir(DIR * dir)
{
	struct __dirent *direntry;
	register struct dirent *buf;

	if (dir == NULL) {
		errno = EFAULT;
		return NULL;
        }

	do {
	        direntry = dnext(dir);
		if (direntry == NULL)
			return NULL;
	} while (direntry->d_name[0] == 0);

	buf = &dir->_priv.de;
	buf->d_ino = direntry->d_ino;
	buf->d_off = -1;	/* FIXME */
	buf->d_reclen = 31;
	dir->dd_loc += (buf->d_reclen + 1);
	strncpy(buf->d_name, (char *) direntry->d_name, 31);
	buf->d_name[30] = 0;
	return buf;
}
