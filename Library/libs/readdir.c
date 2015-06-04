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

static struct __dirent *dnext(struct _dir *dir)
{
        if (dir->next == dir->last) {
                int l = read(dir->d.dd_fd, dir->buf, sizeof(dir->buf));
                if (l <= 0)
                        return NULL;
                l /= 32;
                dir->last = l;
                dir->next = 0;
        }
        return (struct __dirent *)(dir->buf + 32 * dir->next++);
}

struct dirent *readdir(DIR * dirp)
{
        struct _dir *dir = (struct _dir *)dirp;
	struct __dirent *direntry;
	register struct dirent *buf;
	int len;

	if (dir == NULL || dir->d.dd_fd == -1) {
		errno = EFAULT;
		return NULL;
        }

	do {
	        direntry = dnext(dir);
		if (direntry == NULL)
			return NULL;
	} while (direntry->d_name[0] == 0);

	buf = &dir->de;
	buf->d_ino = direntry->d_ino;
	buf->d_off = -1;	/* FIXME */
	buf->d_reclen = 33;
	strncpy(buf->d_name, (char *) direntry->d_name, 31);
	buf->d_name[30] = 0;
	return buf;
}
