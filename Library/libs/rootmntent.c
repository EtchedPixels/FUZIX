/*
 *	Fuzix extensions to mntent
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/file.h>
#include <mntent.h>
#include <dirent.h>

#define DEV_PATH "/dev/"

char *root_device_name(void)
{
	static DIR dp;
	struct dirent *entry;
	struct stat filestat, rootstat;
	static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 1];

	if (stat("/", &rootstat) == 0 && opendir_r(&dp, DEV_PATH) != (DIR *) NULL) {
		while ((entry = readdir(&dp)) != (struct dirent *) NULL) {
			strcpy(namebuf, DEV_PATH);
			strlcat(namebuf, entry->d_name, sizeof(namebuf));
			if (stat(namebuf, &filestat) != 0)
				continue;
			if (!S_ISBLK(filestat.st_mode))
				continue;
			if (filestat.st_rdev != rootstat.st_dev)
				continue;
			return namebuf;
		}
	}
	return NULL;
}


char *mnt_device_path(struct mntent *m)
{
    char *p;
    if (strcmp(m->mnt_fsname, "$ROOT") == 0) {
         p = root_device_name();
         if (p)
             return p;
    }
    return m->mnt_fsname;
}

    