/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * Run programs submitted by at.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

# define ATDIR "/usr/spool/at"
# define PDIR	"past"
# define LASTF "/usr/spool/at/lasttimedone"

int	nowtime;
int	nowdate;
int	nowyear;

void makenowtime(void)
{
	time_t t;
	struct tm *tp;

	time(&t);
	tp = localtime(&t);
	nowtime = tp->tm_hour*100 + tp->tm_min;
	nowdate = tp->tm_yday;
	nowyear = tp->tm_year;
}

void updatetime(int t)
{
	FILE *tfile;

	tfile = fopen(LASTF, "w");
	if (tfile == NULL) {
		fprintf(stderr, "can't write lastfile\n");
		exit(1);
	}
	fprintf(tfile, "%04d\n", t);
}

void run(char *file)
{
	struct stat stbuf;
	int pid, i;
	char sbuf[64];

	if (fork()!=0)
		return;
	for (i=0; i<15; i++)
		close(i);
	dup(dup(open("/dev/null", 0)));
	/* FIXME: cheaper to do the copy inline */
	sprintf(sbuf, "/bin/mv %.14s %s", file, PDIR);
	system(sbuf);
	chdir(PDIR);
	if (stat(file, &stbuf) == -1)
		exit(1);
	setgid(stbuf.st_gid);
	setuid(stbuf.st_uid);
	if ((pid = fork()) != 0) {
		if (pid == -1)
			exit(1);
		wait((int *)0);
		unlink(file);
		exit(0);
	}
	nice(3);
	execl("/bin/sh", "sh", file, 0);
	execl("/usr/bin/sh", "sh", file, 0);
	fprintf(stderr, "Can't execl shell\n");
	exit(1);
}

#define DIRSIZ	30

int main(int argc, const char *argv[])
{
	int tt, day, year, uniq;
	struct dirent *d;
	char file[DIRSIZ+1];
	DIR *dirf;

	if (chdir(ATDIR) < 0) {
		fprintf(stderr, "Can't chdir\n");
		exit(1);
	}
	makenowtime();
	if ((dirf = opendir(".")) == NULL) {
		fprintf(stderr, "Cannot read at directory\n");
		exit(1);
	}
	while ((d = readdir(dirf)) != NULL) {
		strncpy(file, d->d_name, DIRSIZ);
		if (sscanf(file, "%2d.%3d.%4d.%2d", &year, &day, &tt, &uniq) != 4)
			continue;
		if (nowyear < year)
			continue;
		if (nowyear==year && nowdate < day)
			continue;
		if (nowyear==year && nowdate==day && nowtime < tt)
			continue;
		run(file);
	}
	closedir(dirf);
	updatetime(nowtime);
	exit(0);
}
