#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>
#include <dirent.h>

static int quiet;

/* Strip ro and rw from the entry and append the correct one */
/* Will want extending when we do nosuid */
static char *modify_opts(char *opts, int flags)
{
	static char optbuf[_MAX_MNTLEN];
	char *op = optbuf;
	char *p = strtok(opts, ",");
	while(p) {
		if (strcmp(p, "ro") != 0 && strcmp(p, "rw") != 0) {
			if (op != optbuf)
				*op++ = ',';
			strcpy(op, p);
			op += strlen(p);
		}
		p = strtok(NULL,",");
	}
	if (flags & MS_RDONLY) {
		if (op != optbuf)
			*op++ = ',';
		strcpy(op, "ro");
		op += 2;
	}
	if (op == optbuf)
		return "defaults";
	*op = 0;
	return optbuf;
}

#define DEV_PATH "/dev/"

static char *root_device(void)
{
    static DIR dp;
    struct dirent *entry;
    struct stat filestat, rootstat;
    static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 1];

    if (stat("/", &rootstat) == 0
	&& opendir_r(&dp,DEV_PATH) != (DIR *) NULL) {
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


static char *getdev(char *arg, char *p)
{
	FILE *f;
	struct mntent *mnt;

	f = setmntent(p, "r");
	if (f) {
		while (mnt = getmntent(f)) {
			if ((strcmp(mnt->mnt_fsname, arg) == 0) || (strcmp(mnt->mnt_dir, arg) == 0)) {
				endmntent(f);
				return strdup(mnt->mnt_fsname);
			}
		}
		endmntent(f);
	}
	/* Special case / for boot scripts. If you have no fstab or mtab
	   we want it to do the right thing anyway so you can clean up nicely */
	if (strcmp(arg, "/"))
		return NULL;
	return root_device();
}

static void rewrite_mtab(char *name, int flags)
{
	FILE *inpf, *outf;
	struct mntent *mnt;

	if (quiet)
		return;

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
	/* Correct the options for the remounted entry */
	while (mnt = getmntent(inpf)) {
		if (strcmp(name, mnt->mnt_fsname) == 0)
			mnt->mnt_opts = modify_opts(mnt->mnt_opts, flags);
		addmntent(outf, mnt);
	}
	endmntent(inpf);
	endmntent(outf);
	if (rename("/etc/mtab.new", "/etc/mtab") < 0) {
		perror("Error installing /etc/mtab");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	char *dev;
	unsigned int flags;

	if (argc == 4 && strcmp(argv[1], "-n") == 0) {
		argc--;
		argv++;
		quiet = 1;
	}
	if (argc != 3) {
		fprintf(stderr, "%s: remount device [ro][rw]\n", argv[0]);
		return 1;
	}

	dev = getdev(argv[1], "/etc/fstab");
	if (!dev)
		dev = getdev(argv[1], "/etc/mtab");
	if (!dev)
		dev = argv[1];

	/* TODO - nosuid once supported */
	if (strcmp(argv[2], "ro") == 0)
		flags = MS_RDONLY|MS_REMOUNT;
	else if (strcmp(argv[2], "rw") == 0)
		flags = MS_REMOUNT;
	else {
		fprintf(stderr, "%s: ro or rw required.\n", argv[0]);
		return 1;
	}

	/* Rewrite first in case we are updating /etc to ro */
	if (flags & MS_RDONLY)
		rewrite_mtab(dev, flags);
	
	if (remount(dev, flags)) {
		perror("remount");
		return 1;
	}
	/* Otherwise rewrite after in case /etc was ro */
	if (!(flags & MS_RDONLY))
		rewrite_mtab(dev, flags);
	return 0;
}
