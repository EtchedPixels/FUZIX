/*
 *	Substitute the root device path for % in a command line and then
 *	execute the result.
 *
 *	Used in things liek boot scripts where we may not have pipes and thus
 *	command line substitutions. We can't put this functionality in each
 *	command, and in a few cases it won't fit (fsck for example).
 *
 *  (C) Copyright 2018 Alan Cox
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <dirent.h>
#include <unistd.h>

#define DEV_PATH "/dev/"

static char *root_device(void)
{
	static DIR dp;
	struct dirent *entry;
	struct stat filestat, rootstat;
	static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 1];

	if (stat("/", &rootstat) == 0 && opendir_r(&dp, DEV_PATH) != (DIR *) NULL) {
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

int main(int argc, char *argv[])
{
	char *root = root_device();
	int n = 2;
	if (root == NULL) {
		write(2, "substroot: unable to find root device.\n", 39);
		exit(255);
	}
	if (argc < 2) {
		write(2, "substroot app ...\n", 18);
		exit(255);
	}
	while (n < argc) {
		if (strcmp(argv[n], "%") == 0)
			argv[n] = root;
		n++;
	}
	execvp(argv[1], argv + 1);
	perror(argv[1]);
	exit(1);
}
