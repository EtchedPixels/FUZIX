/*	hexdmp.c	1.5	83/05/13	*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpm.h"

void dump(const char *cmdline)
{
	char name[9], ext[4];
	C_FILE *cid;

	if (!cmdline) {
		printf("missing argument (file name)\n");
		return;
	}
	if (!(namesep(cmdline, name, ext)))
		return;
	if ((cid = c_open(name, ext, READ)) == NULL)
		return;
	hexdump(cid);
	c_close(cid);
}

void hexdump(C_FILE * fp)
{
	int c, nc = 0, cbuf[16], blcnt = 0;
	char *pager;
	FILE *piped;

	printf("\n");
	if ((pager = getenv("PAGER")) != 0) {
		/* try opening a pipe */
		if ((piped = popen(pager, "w")) == NULL)
			piped = stdout;
	} else
		piped = stdout;

	/* FIX to uint8_t */
	while ((c = c_getc(fp)) != EOF) {
		cbuf[nc % 16] = c;
		if (nc % 128 == 0)
			fprintf(piped, "\n      Block %04d\n", blcnt++);
		if (nc % 16 == 0)
			fprintf(piped, "%04x  ", nc);
		++nc;
		fprintf(piped, "%02x ", c);
		if (nc % 16 == 0)
			printline(piped, cbuf, 16);
	}
	if (nc % 16 != 0)
		printline(piped, cbuf, nc % 16);
	fprintf(piped, "\n");
	if (piped != stdout)
		pclose(piped);
}

void printline(FILE * piped, int *cbuf, int nc)
{
	int i1;

	for (i1 = 0; i1 < nc; ++i1) {
		if (cbuf[i1] > 31 && cbuf[i1] < 127)
			fprintf(piped, "%c", cbuf[i1]);
		else
			fprintf(piped, ".");
	}
	fprintf(piped, "\n");
}
