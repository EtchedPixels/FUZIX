#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>

/* Assumed length of a line in /etc/mtab */
#define MTAB_LINE 160

char *getdev(char *arg)
{
	FILE *f;
	char tmp[MTAB_LINE];
	char* dev;
	char* mntpt;

	f = fopen("/etc/mtab", "r");
	if (f) {
		while (fgets(tmp, sizeof(tmp), f)) {
			dev = strtok(tmp, " \t");
			if (*tmp == '#' || dev == NULL)
				continue;
			mntpt = strtok(NULL, " \t");
			if (mntpt == NULL)
				continue;
			if ((strcmp(dev, arg) == 0) || (strcmp(mntpt, arg) == 0)) {
				fclose(f);
				return strdup(dev);
			}
		}
		fclose(f);
	}
	return NULL;
}

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
