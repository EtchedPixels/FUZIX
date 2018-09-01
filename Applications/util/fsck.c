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

	fp = setmntent("/etc/fstab", "r");
	if (fp) {
		while (mnt = getmntent(fp)) {
			if (strcmp(mnt->mnt_dir, mount) == 0) {
				endmntent(fp);
				return mnt->mnt_fsname;
			}
		}
		endmntent(fp);
	}
	return NULL;
}

static int perform_fsck_iter(const char *path, int search)
{
	pid_t pid;
	int st;
	const char *p;
	if (search) {
		p = mntpoint(path);
		if (p)
			path = p;
	}
	fflush(stdout);
	switch (pid = fork()) {
	case -1:
		perror("fork");
		exit(127);
	case 0:
		/* FIXME: one day bigger boxes will care about the fs type */
		if (aflag == 0)
			execl("/bin/fsck-fuzix", "fsck-fuzix", path, NULL);
		else
			execl("/bin/fsck-fuzix", "fsck-fuzix", "-a", path, NULL);
		perror("fsck-fuzix");
		exit(1);
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

int perform_fsck(char *name, int search)
{
	int r;
	while ((r = perform_fsck_iter(name, search)) & 64)
		puts("Restarting fsck.\n");
	return r;
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
			if (perform_fsck(mnt->mnt_fsname, 0))
				exit(error);
		}
		endmntent(fp);
	} else {
		if (argc != 2) {
			fputs("syntax: fsck [-a] [devfile]\n", stderr);
			return 16;
		}
		perform_fsck(argv[1], 1);
	}
	puts("Done.");
	exit(error);
}
