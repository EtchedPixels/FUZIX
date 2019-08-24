/* Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Rewritten by Alan Cox to use a binary file format and save a lot of space
 *
 * TODO: support loading a single error string for compactness
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <paths.h>
#include <errno.h>
#include <fcntl.h>

static uint8_t *__sys_errlist;
static uint16_t *__sys_errptr;
static int __sys_nerr;
static char retbuf[80];

#define ALIGNMENT	16

#define ALIGNUP(s) (((s) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static void _load_errlist(void)
{
	struct stat st;
	int fd = open(_PATH_LIBERR, O_RDONLY|O_CLOEXEC);
	if (fd < 0)
		return;
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode))
		goto bad;
	__sys_errlist = sbrk(ALIGNUP(st.st_size));
	if (__sys_errlist == (void *) -1)
		goto bad;
	if (read(fd,__sys_errlist, st.st_size) == st.st_size) {
		__sys_nerr = *__sys_errlist;
		__sys_errptr = (uint16_t *)__sys_errlist + 1;
		close(fd);
		return;
	}
bad:
	close(fd);
	__sys_errlist = NULL;
	return;
}

char *strerror(int err)
{
	if (!__sys_errlist)
		_load_errlist();
	if (__sys_errlist && err >= 0 && err < __sys_nerr)
		return __sys_errlist + __sys_errptr[err];
	strcpy(retbuf, "Unknown error ");
	strcpy(retbuf + 14, _itoa(err));
	return retbuf;
}
