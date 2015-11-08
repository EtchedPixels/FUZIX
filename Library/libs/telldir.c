#include <dirent.h>

off_t telldir(DIR * dirp)
{
	struct _dir *dir = (struct _dir *)dirp;
	return dir->d.dd_loc;
}
