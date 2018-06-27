/*	pip.c	1.5	83/05/13	*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "cpm.h"

/*
 * Copy unix file to cpm
 */

void pip(char *cmdline, int bin)
{

	char *i;

	if ((i = strchr(cmdline, ' ')) == NULL) {
		printf("too few arguments: %s\n", cmdline);
		return;
	}
	*i = '\0';
	pipc(cmdline, i + 1, bin);
}

void pipc(const char *unixfile, const char *cpmfile, int bin)
{

	FILE *ufid;
	char name[9], ext[4];
	C_FILE *cid;
	int flag = 0;

	if ((ufid = fopen(unixfile, "r")) == NULL) {
		printf("can't open %s\n", unixfile);
		return;
	}
	if (!(namesep(cpmfile, name, ext)))
		return;
	if (bin)
		flag = BINARY;
	if ((cid = c_creat(name, ext, flag)) == NULL)
		return;
	if (bin)
		pipbin(cid, ufid);
	else
		piptext(cid, ufid);
	c_close(cid);
	fclose(ufid);
}

void piptext(C_FILE * cid, FILE * ufid)
{
	int c = 0;

	while ((c = getc(ufid)) != EOF) {
		if (c == '\n') {
			if (c_putc('\r', cid) == EOF)
				break;
		}
		if (c_putc(c, cid) == EOF)
			break;
	}
}

void pipbin(C_FILE * cid, FILE * ufid)
{
	char buf[128];

	while (read(fileno(ufid), buf, 128) != 0) {
		if (c_write(cid, buf, 128) != 128) {
			fprintf(stderr, "pipbin: write error\n");
			return;
		}
	}
}
