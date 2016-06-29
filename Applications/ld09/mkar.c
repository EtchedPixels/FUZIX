
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ar.h"

static struct ar_hdr arbuf;

static char buf[512];
static const char *path;
static char space = ' ';

static void twrite(int fd, const void *p, int len)
{
	if (write(fd, p, len) != len)
		err(1, "%s: write failed", path);
}

static void copy_file(const char *p, int fd)
{
	int ifd;
	ssize_t l;

	if ((ifd = open(p, O_RDONLY)) == -1) {
		perror(p);
		exit(1);
	}
	while ((l = read(ifd, buf, sizeof(buf))) > 1)
		twrite(fd, buf, l);
	if (l == 1)
		twrite(fd, &space, 1);
	if (l == -1) {
		perror("read");
		exit(1);
	}
	close(ifd);
}

int main(int argc, char *argv[])
{
	char buf[128];
	int fd;
	struct stat st;
	int ar, libarg = 0, need_o = 0, got_o = 0;

	for (ar = 1; ar < argc; ar++) {
		if (argv[ar][0] == '-') {
			if (argv[ar][1] == 'r')
				need_o = 1;
			if (argv[ar][1] == 'o') {
				got_o++;
				libarg = 0;
			}
		} else {
			if (libarg == 0)
				libarg = ar;
		}
        }
	if (libarg == 0 || got_o > 1 || need_o > got_o)
		errx(1, "-o option required for -r");

        path = argv[libarg];

	if ((fd = open(path, O_WRONLY)) == -1)
		err(1, "unable to open %s", path);


	twrite(fd, ARMAG, SARMAG);

	for (ar = 1; ar < argc; ar++) {
		if (ar != libarg && argv[ar][0] != '-') {
			char *ptr;
			if (stat(argv[ar], &st) < 0)
				err(1, "cannot stat object %s", argv[ar]);
			if ((ptr = strrchr(argv[ar], '/')))
				ptr++;
			else
				ptr = argv[ar];
			memset(&arbuf, ' ', sizeof(arbuf));
			snprintf(buf, sizeof(buf), "%s/                 ", ptr);
			strncpy(arbuf.ar_name, buf, sizeof(arbuf.ar_name));

			snprintf(arbuf.ar_date, 12, "%-12ld",
				 (long) st.st_mtime);
			snprintf(arbuf.ar_uid, 6, "%-6d",
				 (int) (st.st_uid % 1000000L));
			snprintf(arbuf.ar_gid, 6, "%-6d",
				 (int) (st.st_gid % 1000000L));
			snprintf(arbuf.ar_mode, 8, "%-8lo",
				 (long) st.st_mode);
			snprintf(arbuf.ar_size, 10, "%-10ld",
				 (long) st.st_size);
			memcpy(arbuf.ar_fmag, ARFMAG,
			       sizeof(arbuf.ar_fmag));

			twrite(fd, &arbuf, sizeof(arbuf));
			copy_file(argv[ar], fd);
		}
	}
	close(fd);
	exit(0);
}
