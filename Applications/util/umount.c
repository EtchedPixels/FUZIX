#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>

#define MAXFS	16

static char *devlist[MAXFS];
static char **devp = &devlist[0];

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

static int deleted(const char *p)
{
	char **d = &devlist[0];

	while(d < devp) {
		if (*d && strcmp(*d, p) == 0)
			return 1;
		d++;
	}
	return 0;
}

static void queue_rm_mtab(const char *p)
{
	if (devp == &devlist[MAXFS])
		fputs("umount: too many file systems.\n", stderr);
	else
		*devp++ = strdup(p);
}

static int rewrite_mtab(void)
{
	FILE *inpf, *outf;
	struct mntent *mnt;

	inpf = setmntent("/etc/mtab", "r");
	if (!inpf) {
		perror("Can't open /etc/mtab");
		exit(1);
	}
	outf = setmntent("/etc/mtab.new", "w");
	if (!outf) {
		perror("Can't create temporary file");
		exit(1);
	}
	while (mnt = getmntent(inpf)) {
		/* FIXME: should we check device and dir ? */
		if (deleted(mnt->mnt_fsname))
			continue;
		else
			addmntent(outf, mnt);
	}
	endmntent(inpf);
	endmntent(outf);
	if (rename("/etc/mtab.new", "/etc/mtab") < 0) {
		perror("Error installing /etc/mtab");
		exit(1);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct mntent *mnt;
	const char *dev;
	char **dp;
	FILE *f;
	int err = 0;

	umask(022);
	
	if (argc != 2) {
		fprintf(stderr, "%s: umount device\n", argv[0]);
		return 1;
	}
	if (strcmp(argv[1], "-a") == 0) {
		/* We need to umount things in reverse order if we have
		   mounts on mounts */
		f = setmntent("/etc/mtab", "r");
		if (f == NULL) {
			perror("mtab");
			exit(1);
		}
		while(mnt = getmntent(f)) {
			/* We can't unmount / */
			if (strcmp(mnt->mnt_dir, "/"))
				queue_rm_mtab(mnt->mnt_fsname);
		}
		endmntent(f);
		dp = devp;
		/* Unmount in reverse order of mounting */
		while(--dp >= devlist) {
			if (umount(*dp) == -1) {
				perror(*dp);
				*dp = NULL;
				err |= 1;
			}
		}
		rewrite_mtab();
	} else {
		dev = getdev(argv[1]);
		if (!dev)
			dev = argv[1];

		if (umount(dev) == 0) {
			queue_rm_mtab(dev);
			rewrite_mtab();
		} else
			perror("umount");
		return 1;
	}
	return err;
}
