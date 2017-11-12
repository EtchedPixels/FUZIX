#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>

/* Assumed length of a line in /etc/mtab */
#define MTAB_LINE 160
#define MAXFS	16

static char *devlist[MAXFS];
static char **devp = &devlist[0];

static char tmp[MTAB_LINE];

const char *readmtab(FILE *fp)
{
    char *dev;
    while (fgets(tmp, sizeof(tmp), fp) != NULL) {
        dev = strtok(tmp, " \t");
        if (dev == NULL || *tmp == '#')
            continue;
        return dev;
    }
    return NULL;
}

char *getdev(char *arg)
{
	FILE *f;
	const char *dev;
	char* mntpt;

	f = fopen("/etc/mtab", "r");
	if (f) {
		while ((dev = readmtab(f)) != NULL) {
			mntpt = strtok(NULL, " \t");
			if ((strcmp(dev, arg) == 0) || (strcmp(mntpt, arg) == 0)) {
				fclose(f);
				return strdup(dev);
			}
		}
		fclose(f);
	}
	return NULL;
}

int deleted(const char *p)
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

/* FIXME: error checking, atomicity */
int rewrite_mtab(void)
{
	FILE *inpf, *outf;
	char *tmp_fname;
	static char tmp[MTAB_LINE];
	static char tmp2[MTAB_LINE];
	char* dev;

	inpf = fopen("/etc/mtab", "r");
	if (!inpf) {
		perror("Can't open /etc/mtab");
		exit(1);
	}
	outf = fopen("/etc/mtab.new", "w");
	if (!outf) {
		perror("Can't create temporary file");
		exit(1);
	}
	while (fgets(tmp, sizeof(tmp), inpf)) {
		strncpy( tmp2, tmp, MTAB_LINE );
		dev = strtok(tmp, " \t");
		if (dev && deleted(dev)) {
			continue;
		} else {
			fputs(tmp2, outf);
		}
	}
	fclose(inpf);
	fclose(outf);
	if (rename("/etc/mtab.new", "/etc/mtab") < 0) {
		perror("Error installing /etc/mtab");
		exit(1);
	}
	return 0;
}

int main(int argc, char *argv[])
{
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
		f = fopen("/etc/mtab", "r");
		if (f == NULL) {
			perror("mtab");
			exit(1);
		}
		while((dev = readmtab(f)) != NULL) {
			char *mntpt = strtok(NULL, " \t");
			/* We can't unmount / */
			if (strcmp(mntpt, "/"))
				queue_rm_mtab(dev);
		}
		fclose(f);
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
