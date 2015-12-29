#include <dirent.h>
#include <unistd.h>

void rewinddir(DIR *dir)
{
	lseek(dir->dd_fd, dir->dd_loc = 0, SEEK_SET);
	dir->_priv.next = dir->_priv.last = 0;
}
