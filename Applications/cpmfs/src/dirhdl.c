/*	dirhdl.c	1.7	83/05/13	*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cpm.h"

/* Display cp/m directory on stdout */

void dispdir(void)
{

	int cnt, i;
	int filecnt = 0;
	int blkcnt;
	char name[8], ext[3];

	for (cnt = 0; cnt < maxdir; cnt++) {
		if ((dirbuf + cnt)->status != (char) 0xe5) {
			if ((dirbuf + cnt)->extno == '\0') {
				strncpy(name, (dirbuf + cnt)->name, 8);
				strncpy(ext, (dirbuf + cnt)->ext, 3);
				for (i = 0; i < 8; i++)
					name[i] &= 0x7f;
				for (i = 0; i < 3; i++)
					ext[i] &= 0x7f;
				printf("%.8s %.3s", name, ext);
				if (++filecnt % 4 == 0)
					printf("\n");
				else
					printf("   :   ");
			}
		}
	}
	blkcnt = blks_used();
	if (filecnt % 4 > 0)
		printf("\n");
	if (filecnt == 0)
		printf("No files\n");
	else
		printf("Total of %d files. %d blocks used, %d blocks free.\n", filecnt, blkcnt, seclth * sectrk * (tracks - 2) / blksiz - blkcnt);
}

void getdir(void)
{
	int bl, blks;
	int offset = 0;

	blks = maxdir * 32 / blksiz;
	if ((maxdir * 32) % blksiz > 0)
		++blks;
	for (bl = 0; blks > 0; bl++, blks--) {
		if (getblock(bl, ((char *)dirbuf) + offset, -1) == EOF) {
			fprintf(stderr, "getdir: fatal error\n");
			exit(0);
		}
		offset += blksiz / 32;
	}
}


void savedir(void)
{

	int bl, blks;
	int offset = 0;

	blks = maxdir * 32 / blksiz;
	if ((maxdir * 32) % blksiz > 0)
		++blks;
	for (bl = 0; blks > 0; bl++, blks--) {
		if (putblock(bl, ((char *)dirbuf) + offset, -1) == EOF) {
			fprintf(stderr, "savedir: fatal error\n");
			exit(0);
		}
		offset += blksiz / 32;
	}
}

/* Search the cp/m directory for the file given by the input
 * parameters, return -1 if not found,
 * directory index to the file's first extent is
 * returned if found.
 */

int searchdir(const char *name, const char *ext)
{
	int ind, i;
	char cname[8], cext[3];

	for (ind = 0; ind < maxdir; ++ind) {
		strncpy(cname, (dirbuf + ind)->name, 8);
		strncpy(cext, (dirbuf + ind)->ext, 3);
		for (i = 0; i < 8; i++)
			cname[i] &= 0x7f;
		for (i = 0; i < 3; i++)
			cext[i] &= 0x7f;
		if ((dirbuf + ind)->status == (char) 0xe5)
			continue;
		if ((strncmp(name, cname, 8) == 0) && (strncmp(ext, cext, 3) == 0) && ((dirbuf + ind)->extno == '\0'))
			return ind;
	}
	return -1;
}
