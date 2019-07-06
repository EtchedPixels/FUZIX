/*
 *	Fuzix extensions to mntent
 */

#define _FUZIX_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <mntent.h>
#include <dirent.h>

static char namebuf[sizeof("/dev/") + MAXNAMLEN + 1] = "/dev/";
static struct stat filestat;

static uint8_t test_dev(dev_t dev)
{
	if (stat(namebuf, &filestat) != 0 ||
		!S_ISBLK(filestat.st_mode) ||
		filestat.st_rdev != dev)
			return 0;
	return 1;
}

char *devname(dev_t dev)
{
	/* Has to be static to keep cc65 happy 8( */
	static DIR dp;
	struct dirent *entry;
	uint_fast8_t m;

	/* Start by doing a guess for speed. Assume normal disk layout */
	m = minor(dev);

	switch(major(dev)) {
	case 0:
		strcpy(namebuf + 5, "hd");
		namebuf[7] = 'a' + (m >> 4);
		if (m & 0x0F)
			strcpy(namebuf + 8, _uitoa(m & 0x0F));
		if (test_dev(dev))
			return namebuf;
		break;
	case 1:
		strcpy(namebuf + 5, "fd");
		strcpy(namebuf + 7, _uitoa(m));
		if(test_dev(dev))
			return namebuf;
		break;
	}

	if (opendir_r(&dp, "/dev") != (DIR *) NULL) {
		while ((entry = readdir(&dp)) != (struct dirent *) NULL) {
			strlcpy(namebuf + 5, entry->d_name, sizeof(namebuf) - 5);
			if (test_dev(dev))
				return namebuf;
		}
	}
	return NULL;
}


char *root_device_name(void)
{
	stat("/", &filestat);
	return devname(filestat.st_dev);
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
