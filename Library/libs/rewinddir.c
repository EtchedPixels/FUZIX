#include <dirent.h>
#include <unistd.h>

void rewinddir(DIR *dirp)
{
	struct _dir *dir = (struct _dir *)dirp;
	lseek(dir->d.dd_fd, dir->d.dd_loc = 0, SEEK_SET);
	dir->next = dir->last = 0;
}
