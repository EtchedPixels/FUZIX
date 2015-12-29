#include <dirent.h>
#include <unistd.h>

void seekdir(DIR *dir, off_t pos)
{
	lseek(dir->dd_fd, dir->dd_loc = pos, SEEK_SET);
	dir->_priv.next = dir->_priv.last = 0;
}
