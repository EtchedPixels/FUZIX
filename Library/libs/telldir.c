#include <dirent.h>

off_t telldir(DIR * dir)
{
	return dir->dd_loc;
}
