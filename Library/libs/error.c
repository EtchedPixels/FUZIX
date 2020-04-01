/* Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Rewritten by Alan Cox to use a binary file format and save a lot of space
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <paths.h>
#include <errno.h>
#include <fcntl.h>

#define LONGEST_STRING 50

static char retbuf[LONGEST_STRING + 1];
static int last_err = -1;

char *strerror(int err)
{
	uint16_t nerr;
	struct stat st;
	int fd;

	if (err < 0)
		goto sad;

	if (err == last_err)
		return retbuf;

	fd = open(_PATH_LIBERR, O_RDONLY|O_CLOEXEC);
	if (fd < 0)
		goto sad;

	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode))
		goto bad;

	if (read(fd, &nerr, 2) == 2 && err < nerr) {
		uint16_t index;
		uint16_t nexti;
		int len;

		lseek(fd, 2 + err * 2, SEEK_SET);
		read(fd, &index, 2);
		if (err < nerr - 1) {
			read(fd, &nexti, 2);
			len = nexti - index;
		} else {
			len = LONGEST_STRING;
		}
		lseek(fd, index, SEEK_SET);
		len = read(fd, retbuf, len);
		retbuf[len] = '\0';
		last_err = err;

		close(fd);
		return retbuf;
	}

bad:
	close(fd);
sad:
	strcpy(retbuf, "Unknown error ");
	strcpy(retbuf + 14, _itoa(err));
	return retbuf;
}
