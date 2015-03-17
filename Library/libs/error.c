/* Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <paths.h>
#include <errno.h>

char **__sys_errlist = 0;
int __sys_nerr = 0;

char *strerror(int err) 
{
	static char retbuf[80];
	char *p, inbuf[128];
	int cc, fd;
	uint i, bufoff = 0;
	if (__sys_nerr) {	/* sys_errlist preloaded */
		if (err < 0 || err >= __sys_nerr)
			goto UErr;
		return __sys_errlist[err];
	}
	if (err <= 0)
		goto UErr;	/* NB the <= allows comments in the file */
	if ((fd = open(_PATH_LIBERR, 0)) < 0)
		goto UErr;
	while ((cc = read(fd, inbuf, sizeof(inbuf))) > 0) {
		i = 0;
		while (i < cc) {
			if (inbuf[i] == '\n') {
				retbuf[bufoff] = '\0';
				if (err == atoi(retbuf)) {
					if ((p = strchr(retbuf,' ')) == NULL) {
						close(fd);
						goto UErr;
					}
					while (*p == ' ')
						p++;
					close(fd);
					return p;
				}
				bufoff = 0;
			}
			else if (bufoff < sizeof(retbuf) - 1)
				retbuf[bufoff++] = inbuf[i];
			++i;
		}
	}
UErr:	strcpy(retbuf, "Unknown error ");
	strncat(retbuf, _itoa(err), 10);
	return retbuf;
}
