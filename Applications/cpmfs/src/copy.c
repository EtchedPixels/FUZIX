/*	copy.c	1.8	83/05/13	*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpm.h"

#define	CTRL_Z	0x1a		/* CP/M end-of-file */

/* 
 * copy cpmfile to unix file 
 */

void copyc(char *cmdline, int bin)
{

	char *i;

	if ((i = strchr(cmdline, ' ')) == NULL) {
		printf("too few arguments: %s\n", cmdline);
		return;
	}
	*i = '\0';
	copy(cmdline, i + 1, bin);
}

void copy(char *cpmfile, char *unixfile, int bin)
{
	FILE *ufid;
	char name[9], ext[4];
	char *pager = 0;
	C_FILE *cid;

	if (!(namesep(cpmfile, name, ext)))
		return;
	if ((cid = c_open(name, ext, READ)) == NULL)
		return;

	if (unixfile == NULL) {
		if ((pager = getenv("PAGER")) != 0 && *pager) {
			/* try opening a pipe to the pager */
			if ((ufid = popen(pager, "w")) == NULL)
				/* failed, use stdout */
				ufid = stdout;
		} else
			ufid = stdout;
	} else {
		if (access(unixfile, 0) == 0) {
			printf("%s already exists\n", unixfile);
			return;
		}
		if ((ufid = fopen(unixfile, "w")) == NULL) {
			printf("can't open %s\n", unixfile);
			return;
		}
	}
	if (bin)
		copybin(cid, ufid);
	else
		copytext(cid, ufid);
	c_close(cid);
	if (pager && *pager)
		pclose(ufid);
}

void copytext(C_FILE * cid, FILE * ufid)
{
	int c = 0;

	while (((c = c_getc(cid)) != EOF) && (c != CTRL_Z)) {
		if (c != '\r')
			putc(c, ufid);
	}
	if (isatty(fileno(ufid)))
		printf("\n");
	else
		fclose(ufid);
}

void copybin(C_FILE * cid, FILE * ufid)
{
	int c = 0;

	while ((c = c_getc(cid)) != EOF)
		putc(c, ufid);
	fclose(ufid);
}
