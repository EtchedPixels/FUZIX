/*************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix
    systems. Copyright (C) 2005, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "main.h"
#include "cpmutl.h"

uint8_t allocv[256];


/*----------------------------------------------------------------------*/

struct cpmfcb *get_dir_entry(DIR * dirp, struct cpmfcb *search_fcb, int first)
{
	int i;
	struct dirent *dp;
	static struct cpmfcb last_search;	/* to be preserved across calls */
	static struct stat stbuf;
	static int blkno;

	if (!dirp)
		return NULL;

	if (first) {
		stbuf.st_size = 0;
		blkno = 8;	/* fake block map numbers too (needed for CP/M 'stat') */
		/* we have 7 reserved dir blocks */
	} else if (search_fcb->ex == '?' && stbuf.st_size > 0) {
		/* remaining from last search? */
		++last_search.ex;
		if (stbuf.st_size < 16384) {
			last_search.rc = (stbuf.st_size + 127) / 128;
			for (i = 0; i < (stbuf.st_size + 2047) / 2048; ++i) {
				last_search.dmap[i * 2] = blkno & 0xff;
				last_search.dmap[i * 2 + 1] = (blkno >> 8) & 0xff;
				++blkno;
			}
			for (; i < 8; ++i) {
				last_search.dmap[i * 2] = 0;
				last_search.dmap[i * 2 + 1] = 0;
			}
			stbuf.st_size = 0;
		} else {
			last_search.rc = 128;
			for (i = 0; i < 8; ++i) {
				last_search.dmap[i * 2] = blkno & 0xff;
				last_search.dmap[i * 2 + 1] = (blkno >> 8) & 0xff;
				++blkno;
			}
			stbuf.st_size -= 16384;
		}
		return &last_search;
	}

	stbuf.st_size = 0;

	while (1) {
		while ((dp = readdir(dirp))) {
			if ((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0))
				break;
		}

		if (!dp)
			return NULL;	/* no more entries */

		/* last_search.drive = userno; - done by the caller */

		setname(&last_search, dp->d_name);
		if (!match_fcb(search_fcb, &last_search))
			continue;

		if (stat(dp->d_name, &stbuf) != 0) {
			stbuf.st_size = 0;
			continue;	/* can't stat? - don't show it */
		}
		if (S_ISDIR(stbuf.st_mode))
			continue;	/* don't show dirs either */

		last_search.ex = 0;
		if (stbuf.st_size < 16384) {
			last_search.rc = (stbuf.st_size + 127) / 128;
			for (i = 0; i < (stbuf.st_size + 2047) / 2048; ++i) {
				last_search.dmap[i * 2] = blkno & 0xff;
				last_search.dmap[i * 2 + 1] = (blkno >> 8) & 0xff;
				++blkno;
			}
			for (; i < 8; ++i) {
				last_search.dmap[i * 2] = 0;
				last_search.dmap[i * 2 + 1] = 0;
			}
			stbuf.st_size = 0;
		} else {
			last_search.rc = 128;
			for (i = 0; i < 8; ++i) {
				last_search.dmap[i * 2] = blkno & 0xff;
				last_search.dmap[i * 2 + 1] = (blkno >> 8) & 0xff;
				++blkno;
			}
			stbuf.st_size -= 16384;
		}
		return &last_search;
	}
}

int delete_files(struct cpmfcb *fcb)
{
	int retc;
	struct dirent *dp;
	struct stat stbuf;
	struct cpmfcb lfcb;
	DIR *dirp;

	dirp = opendir(".");
	if (!dirp)
		return 0xff;

	retc = 0xff;
	while (1) {
		while ((dp = readdir(dirp))) {
			if ((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0))
				break;
		}

		if (!dp)
			break;	/* no more entries */

		setname(&lfcb, dp->d_name);
		if (!match_fcb(fcb, &lfcb))
			continue;

		if (stat(dp->d_name, &stbuf) != 0) {
			stbuf.st_size = 0;
			continue;	/* can't stat? - don't delete it */
		}
		if (S_ISDIR(stbuf.st_mode))
			continue;	/* don't delete dirs either */

		unlink(dp->d_name);
		retc = 0;
	}

	closedir(dirp);

	return retc;
}

int update_allocv(void)
{
	int i;

	for (i = 0; i < 256; ++i)
		allocv[i] = 0;

	return 0;
}

char *getname(struct cpmfcb *fcb)
{
	static char name[20];
	int i;
	char c, *p;

	p = name;
	for (i = 0; i < 8; ++i) {
		c = fcb->name[i] & 0x7f;
		if (!c || c == ' ')
			break;
		*p++ = tolower(c);
	}
	*p++ = '.';
	for (i = 0; i < 3; ++i) {
		c = fcb->ext[i] & 0x7f;
		if (!c || c == ' ') {
			if (i == 0)
				p--;	/* remove dot if ext empty */
			break;
		}
		*p++ = tolower(c);
	}
	*p = 0;

	return name;
}

int setname(struct cpmfcb *fcb, char *name)
{
	int i;
	char *p;

	p = (char *)fcb->name;
	for (i = 0; i < 8; ++i) {
		if (*name && (*name != '.')) {
			*p++ = toupper(*name & 0x7f);
			++name;
		} else {
			*p++ = ' ';
		}
	}
	if (*name == '.')
		++name;
	for (i = 0; i < 3; ++i) {
		if (*name && (*name != '.')) {
			*p++ = toupper(*name & 0x7f);
			++name;
		} else {
			*p++ = ' ';
		}
	}

	return (*name == '\0');
}

int match_fcb(struct cpmfcb *mask, struct cpmfcb *fcb)
{
	int i;

	if (fcb->name[0] == ' ' && fcb->ext[0] == ' ')
		return 0;

	for (i = 0; i < 11; ++i) {
		if (mask->name[i] == '?')
			continue;
		if (toupper(mask->name[i] & 0x7f) != toupper(fcb->name[i] & 0x7f))
			return 0;
	}

	return 1;
}
