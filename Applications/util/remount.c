#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>

static char *getdev(char *arg)
{
	FILE *f;
	struct mntent *mnt;

	f = setmntent("/etc/mtab", "r");
	if (f) {
		while (mnt = getmntent(f)) {
			if ((strcmp(mnt->mnt_fsname, arg) == 0) || (strcmp(mnt->mnt_dir, arg) == 0)) {
				endmntent(f);
				return strdup(mnt->mnt_fsname);
			}
		}
		endmntent(f);
	}
	return NULL;
}

/* FIXME: needs to update mtab entry / support nosuid etc ? */

int main(int argc, char *argv[])
{
	char *dev;
	unsigned int flags;
	
	if (argc != 3) {
		fprintf(stderr, "%s: remount device [ro][rw]\n", argv[0]);
		return 1;
	}

	dev = getdev(argv[1]);
	if (!dev) dev = argv[1];
	
	if (strcmp(argv[2], "ro") == 0)
		flags = MS_RDONLY|MS_REMOUNT;
	else if (strcmp(argv[2], "rw") == 0)
		flags = MS_REMOUNT;
	else {
		fprintf(stderr, "%s: ro or rw required.\n", argv[0]);
		return 1;
	}
	
	if (remount(dev, flags)) {
		perror("umount");
		return 1;
	}
	return 0;
}
