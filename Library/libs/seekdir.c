#include <dirent.h>
#include <unistd.h>

void seekdir (DIR * dirp, off_t pos)
{
	struct _dir *dir = (struct _dir *)dirp;
	lseek(dir->d.dd_fd, dir->d.dd_loc = pos, SEEK_SET);
	dir->next = dir->last = 0;
}
