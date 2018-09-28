#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

static int error;
static int aflag;

static void panic(char *s)
{
	fprintf(stderr, "panic: %s\n", s);
	exit(1);
}

const char *mntpoint(const char *mount)
{
	FILE *fp;
	struct mntent *mnt;
	const char *p;

	fp = setmntent("/etc/fstab", "r");
	if (fp) {
		while (mnt = getmntent(fp)) {
			p = mnt_device_path(mnt);
			if (strcmp(mnt->mnt_dir, mount) == 0) {
				endmntent(fp);
				return p;
			}
		}
		endmntent(fp);
	}
	return NULL;
}

static void perform_fsck_exec(const char *path)
{
	if (aflag == 0)
		execl("/bin/fsck-fuzix", "fsck-fuzix", path, NULL);
	else
		execl("/bin/fsck-fuzix", "fsck-fuzix", "-a", path, NULL);
	perror("fsck-fuzix");
	exit(1);

}

static int perform_fsck(const char *path, uint8_t search, uint8_t only)
{
	pid_t pid;
	int st;
	const char *p;
	if (search) {
		p = mntpoint(path);
		if (p)
			path = p;
	}
	/* Only fsck helper we will run - so just exec it */
	if (only)
		perform_fsck_exec(path);

	/* May be multiple: use fork and exec */
	fflush(stdout);
	switch (pid = fork()) {
	case -1:
		perror("fork");
		exit(127);
	case 0:
		/* FIXME: one day bigger boxes will care about the fs type */
		perform_fsck_exec(path);
	default:
		while (waitpid(pid, &st, 0) == -1) {
			if (errno == ECHILD) {
				perror("waitpid");
				exit(1);
			}
		}
		if (WIFEXITED(st))
			return WEXITSTATUS(st);
		fprintf(stderr, "child process failed %d.\n", WTERMSIG(st));
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	struct mntent *mnt;
	if (argc > 1 && strcmp(argv[1], "-a") == 0) {
		argc--;
		argv++;
		aflag = 1;
	}

	if (argc == 1 && aflag) {
		FILE *fp = setmntent("/etc/fstab", "r");
		if (fp == NULL) {
			perror("/etc/fstab");
			exit(1);
		}
		while ((mnt = getmntent(fp)) != NULL) {
			printf("%s:\n", mnt->mnt_fsname);
			if (perform_fsck(mnt->mnt_fsname, 0, 0))
				exit(error);
		}
		endmntent(fp);
	} else {
		if (argc != 2) {
			fputs("syntax: fsck [-a] [devfile]\n", stderr);
			return 16;
		}
		perform_fsck(argv[1], 1, 1);
	}
	puts("Done.");
	exit(error);
}
