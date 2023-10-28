/*

	Copyright (c) 1993, by David Michael Betz
	All rights reserved
	
MIT License

Copyright (c) 2017 dbetz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


    Modified from the original quite a bit in order to bash it into a single
    file and shrink it down for Fuzix

    https://github.com/dbetz/advsys

*/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "advint.h"

/* useful definitions */
#define EOS     '\0'
#define LINEMAX 200
#define WORDMAX 100

/* global variables */
char line[LINEMAX + 1];

/* local variables */
static int col, maxcol, row, maxrow;
static int scnt, wordcnt;
static char theWord[WORDMAX + 1], *wordptr;

jmp_buf restart;

int pc, opcode, p2, p3, sts;
int stack[STKSIZE], *sp, *fp, *top;
long rseed = 1L;

int h_init;			/* initialization code */
int h_update;			/* update code */
int h_before;			/* before handler code */
int h_after;			/* after handler code */
int h_error;			/* error handling code */

int datafd;			/* data file descriptor */

/* table base addresses */
char *wtable;			/* word table */
char *wtypes;			/* word type table */
int wcount;			/* number of words */
char *otable;			/* object table */
int ocount;			/* number of objects */
char *atable;			/* action table */
int acount;			/* number of actions */
char *vtable;			/* variable table */
int vcount;			/* number of variables */
char *data;			/* base of data tables */
char *base;			/* current base address */
char *dbase;			/* base of the data space */
char *cbase;			/* base of the code space */
int length;			/* length of resident data structures */

/* data file header */
static char hdr[HDR_SIZE];

/* save parameters */
static long saveoff;		/* save data file offset */
static char *save;		/* save area base address */
static int slen;		/* save area length */

/* parser result variables */
int nouns[20];
int *adjectives[20];
static int actor, action, dobject, ndobjects, iobject;

/* local variables */
static char *lptr;		/* line pointer */
static int words[100];		/* word table */
static char *wtext[100];	/* word text table */
static int *wptr;		/* word pointer */
static int wcnt;		/* word count */

static int verbs[3];		/* words in the verb phrase */
static int nnums[20];		/* noun word numbers */
static int nptr;		/* noun pointer (actually, an index) */
static int adjs[100];		/* adjective lists */
static int anums[100];		/* adjective word numbers */
static int aptr;		/* adjective pointer (actually, an index) */

/* cache size */
#define CSIZE	8

/* message block cache */
static char *mbuffer[CSIZE];	/* message text block cache buffers */
static int mblock[CSIZE];	/* message text block cache block numbers */
static int mnext[CSIZE];	/* next most recently used block */
static int mhead, mtail;	/* head and tail of lru list */

static char mbufdata[CSIZE * 512];

/* message file variables */
static int mbase;		/* message base block */
static int mfd;

/* current message variables */
static int mblk;		/* current block */
static char *mbuf;		/* current buffer */
static int moff;		/* current buffer offset */


static void writes(const char *p)
{
	write(1, p, strlen(p));
}

/* execute - execute adventure code */
int execute(int code)
{
	/* setup initial program counter */
	if ((pc = code) == NIL)
		return (CHAIN);

	/* initialize */
	sp = fp = top = stack + STKSIZE;

	/* execute the code */
	for (sts = 0; sts == 0;)
		exe_one();

	return (sts);
}

/* exe_one - execute one instruction */
void exe_one(void)
{
	/* get the opcode */
	opcode = getcbyte(pc);
	pc++;

	/* execute the instruction */
	switch (opcode) {
	case OP_CALL:
		*--sp = getboperand();
		*--sp = pc;
		*--sp = (int) (top - fp);
		fp = sp;
		pc = getafield(fp[fp[2] + 3], A_CODE);
		break;
	case OP_SEND:
		*--sp = getboperand();
		*--sp = pc;
		*--sp = (int) (top - fp);
		fp = sp;
		if ((p2 = fp[fp[2] + 3]) != 0)
			p2 = getofield(p2, O_CLASS);
		else
			p2 = fp[fp[2] + 2];
		if (p2 && (p2 = getp(p2, fp[fp[2] + 1]))) {
			pc = getafield(p2, A_CODE);
			break;
		}
		*sp = NIL;
		/* return NIL if there is no method for this message */
	case OP_RETURN:
		if (fp == top)
			sts = CHAIN;
		else {
			p2 = *sp;
			sp = fp;
			fp = top - *sp++;
			pc = *sp++;
			p3 = *sp++;
			sp += p3;
			*sp = p2;
		}
		break;
	case OP_TSPACE:
		sp -= getboperand();
		break;
	case OP_TMP:
		p2 = getboperand();
		*sp = fp[-p2 - 1];
		break;
	case OP_TSET:
		p2 = getboperand();
		fp[-p2 - 1] = *sp;
		break;
	case OP_ARG:
		p2 = getboperand();
		if (p2 >= fp[2])
			error("too few arguments");
		*sp = fp[p2 + 3];
		break;
	case OP_ASET:
		p2 = getboperand();
		if (p2 >= fp[2])
			error("too few arguments");
		fp[p2 + 3] = *sp;
		break;
	case OP_BRT:
		pc = (*sp ? getwoperand() : pc + 2);
		break;
	case OP_BRF:
		pc = (*sp ? pc + 2 : getwoperand());
		break;
	case OP_BR:
		pc = getwoperand();
		break;
	case OP_T:
		*sp = T;
		break;
	case OP_NIL:
		*sp = NIL;
		break;
	case OP_PUSH:
		*--sp = NIL;
		break;
	case OP_NOT:
		*sp = (*sp ? NIL : T);
		break;
	case OP_ADD:
		p2 = *sp++;
		*sp += p2;
		break;
	case OP_SUB:
		p2 = *sp++;
		*sp -= p2;
		break;
	case OP_MUL:
		p2 = *sp++;
		*sp *= p2;
		break;
	case OP_DIV:
		p2 = *sp++;
		*sp = (p2 == 0 ? 0 : *sp / p2);
		break;
	case OP_REM:
		p2 = *sp++;
		*sp = (p2 == 0 ? 0 : *sp % p2);
		break;
	case OP_BAND:
		p2 = *sp++;
		*sp &= p2;
		break;
	case OP_BOR:
		p2 = *sp++;
		*sp |= p2;
		break;
	case OP_BNOT:
		*sp = ~*sp;
		break;
	case OP_LT:
		p2 = *sp++;
		*sp = (*sp < p2 ? T : NIL);
		break;
	case OP_EQ:
		p2 = *sp++;
		*sp = (*sp == p2 ? T : NIL);
		break;
	case OP_GT:
		p2 = *sp++;
		*sp = (*sp > p2 ? T : NIL);
		break;
	case OP_LIT:
		*sp = getwoperand();
		break;
	case OP_SPLIT:
		*sp = getboperand();
		break;
	case OP_SNLIT:
		*sp = -getboperand();
		break;
	case OP_VAR:
		*sp = getvalue(getwoperand());
		break;
	case OP_SVAR:
		*sp = getvalue(getboperand());
		break;
	case OP_SET:
		setvalue(getwoperand(), *sp);
		break;
	case OP_SSET:
		setvalue(getboperand(), *sp);
		break;
	case OP_GETP:
		p2 = *sp++;
		*sp = getp(*sp, p2);
		break;
	case OP_SETP:
		p3 = *sp++;
		p2 = *sp++;
		*sp = setp(*sp, p2, p3);
		break;
	case OP_PRINT:
		print(*sp);
		break;
	case OP_PNUMBER:
		pnumber(*sp);
		break;
	case OP_PNOUN:
		show_noun(*sp);
		break;
	case OP_TERPRI:
		trm_chr('\n');
		break;
	case OP_FINISH:
		sts = FINISH;
		break;
	case OP_CHAIN:
		sts = CHAIN;
		break;
	case OP_ABORT:
		sts = ABORT;
		break;
	case OP_EXIT:
		trm_done();
		exit(1);
		break;
	case OP_YORN:
		trm_get(line);
		*sp = (line[0] == 'Y' || line[0] == 'y' ? T : NIL);
		break;
	case OP_CLASS:
		*sp = getofield(*sp, O_CLASS);
		break;
	case OP_MATCH:
		p2 = *sp++;
		*sp = (match(*sp, nouns[p2 - 1], adjectives[p2 - 1]) ? T : NIL);
		break;
	case OP_SAVE:
		*sp = db_save();
		break;
	case OP_RESTORE:
		*sp = db_restore();
		break;
	case OP_RESTART:
		*sp = db_restart();
		break;
	case OP_RAND:
		*sp = getrand(*sp);
		break;
	case OP_RNDMIZE:
		setrand(time(0L));
		*sp = NIL;
		break;
	default:
		if (opcode >= OP_XVAR && opcode < OP_XSET)
			*sp = getvalue(opcode - OP_XVAR);
		else if (opcode >= OP_XSET && opcode < OP_XPLIT)
			setvalue(opcode - OP_XSET, *sp);
		else if (opcode >= OP_XPLIT && opcode < OP_XNLIT)
			*sp = opcode - OP_XPLIT;
		else if (opcode >= OP_XNLIT && opcode < 256)
			*sp = OP_XNLIT - opcode;
		else
			trm_str("Bad opcode\n");
		break;
	}
}

/* getboperand - get data byte */
int getboperand(void)
{
	int data;
	data = getcbyte(pc);
	pc += 1;
	return (data);
}

/* getwoperand - get data word */
int getwoperand(void)
{
	int data;
	data = getcword(pc);
	pc += 2;
	return (data);
}

/* print - print a message */
void print(int msg)
{
	int ch;

	msg_open(msg);
	while ((ch = msg_byte()) != 0)
		trm_chr(ch);
}

/* pnumber - print a number */
void pnumber(int n)
{
#ifdef __linux__
	char buf[10];
	sprintf(buf, "%d", n);
	trm_str(buf);
#else
	trm_str(_itoa(n));
#endif
}

/* getrand - get a random number between 0 and n-1 */
int getrand(int n)
{
	long k1;

	/* make sure we don't get stuck at zero */
	if (rseed == 0L)
		rseed = 1L;

	/* algorithm taken from Dr. Dobbs Journal, November 1985, page 91 */
	k1 = rseed / 127773L;
	if ((rseed = 16807L * (rseed - k1 * 127773L) - k1 * 2836L) < 0L)
		rseed += 2147483647L;

	/* return a random number between 0 and n-1 */
	return ((int) (rseed % (long) n));
}

/* setrand - set the random number seed */
void setrand(long n)
{
	rseed = n;
}

long _seed = 1L;

int advsave(char *hdr, int hlen, char *save, int slen)
{
	char fname[50];
	int fd;

	trm_str("File name? ");
	trm_get(fname);

	/* add the extension */
	strcat(fname, ".sav");

	/* create the data file */
	if ((fd = open(fname, O_WRONLY | O_TRUNC | O_CREAT, 0600)) == 0)
		return (0);

	if (write(fd, hdr, hlen) != hlen) {
		close(fd);
		return (0);
	}

	if (write(fd, save, slen) != slen) {
		close(fd);
		return (0);
	}

	/* close the file and return successfully */
	close(fd);
	return (1);
}

int advrestore(char *hdr, int hlen, char *save, int slen)
{
	char fname[50], hbuf[50], *p;
	int fd;

	if (hlen > 50)
		error("save file header buffer too small");

	trm_str("File name? ");
	trm_get(fname);

	/* add the extension */
	strcat(fname, ".sav");

	/* create the data file */
	if ((fd = open(fname, O_RDONLY)) == 0)
		return (0);

	/* read the header */
	if (read(fd, hbuf, hlen) != hlen) {
		close(fd);
		return (0);
	}

	/* compare the headers */
	for (p = hbuf; hlen--;)
		if (*hdr++ != *p++) {
			trm_str("This save file does not match the adventure!\n");
			close(fd);
			return (0);
		}

	/* read the data */
	if (read(fd, save, slen) != slen) {
		close(fd);
		return (0);
	}

	/* close the file and return successfully */
	close(fd);
	return (1);
}

/* main - the main routine */
int main(int argc, char *argv[])
{
	char *fname, *lname;
	int rows, cols, i;

	writes("ADVINT v1.2 - Copyright (c) 1986, by David Betz\n");
	writes("ANSI-fied by Matt Ackeret (unknown@apple.com or unknown@ucscb.ucsc.edu).\n");
	writes("ANSI-compliant source is available on ftp.gmd.de (mirrored on\nwuarchive.wustl.edu). " "Other games are available there also.\n");

	fname = NULL;
	lname = NULL;
	rows = 24;
	cols = 80;

	/* parse the command line */
	for (i = 1; i < argc; i++)
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
			case 'r':
			case 'R':
				rows = atoi(&argv[i][2]);
				break;
			case 'c':
			case 'C':
				cols = atoi(&argv[i][2]);
				break;
			case 'l':
			case 'L':
				lname = &argv[i][2];
				break;
		} else
			fname = argv[i];
	if (fname == NULL) {
		writes("usage: advint [-r<rows>] [-c<columns>] [-l<log-file>] <file>\n");
		exit(1);
	}

	/* initialize terminal i/o */
	trm_init(rows, cols, lname);

	/* initialize the database */
	db_init(fname);

	/* play the game */
	play();
}

/* play - the main loop */
void play(void)
{
	/* establish the restart point */
	setjmp(restart);

	/* execute the initialization code */
	execute(h_init);

	/* turn handling loop */
	for (;;) {

		/* execute the update code */
		execute(h_update);

		/* parse the next input command */
		if (parse()) {
			if (single())
				while (next() && single());
		}

		/* parse error, call the error handling code */
		else
			execute(h_error);
	}
}

/* single - handle a single action */
int single(void)
{
	/* execute the before code */
	switch (execute(h_before)) {
	case ABORT:		/* before handler aborted sequence */
		return (FALSE);
	case CHAIN:		/* execute the action handler */
		if (execute(getafield(getvalue(V_ACTION), A_CODE)) == ABORT)
			return (FALSE);
	case FINISH:		/* execute the after code */
		if (execute(h_after) == ABORT)
			return (FALSE);
		break;
	}
	return (TRUE);
}

/* error - print an error message and exit */
void error(char *msg)
{
	trm_str(msg);
	trm_chr('\n');
	exit(1);
}

/* db_init - read and decode the data file header */
void db_init(char *name)
{
	int woff, ooff, aoff, voff, n;
	char fname[50];
	int TMPINT;

	/* get the data file name */
	strcpy(fname, name);
	strcat(fname, ".dat");

	/* open the data file */
	datafd = open(fname, O_RDONLY);
	if (datafd == -1)
		error("can't open data file");

	/* read the header */

	TMPINT = read(datafd, hdr, HDR_SIZE);
	if (TMPINT != HDR_SIZE)
		error("bad data file");

	complement(hdr, HDR_SIZE);
	base = hdr;

	/* check the magic information */
	if (strncmp(&hdr[HDR_MAGIC], "ADVSYS", 6) != 0)
		error("not an adventure data file");

	/* check the version number */

	if ((n = getword(HDR_VERSION)) < 101 || n > VERSION)
		error("wrong version number");

	/* decode the resident data length header field */
	length = getword(HDR_LENGTH);

	/* allocate space for the resident data structure */
	if ((data = sbrk(length)) == ((void *)-1))
		error("insufficient memory");

	/* compute the offset to the data */
	saveoff = (long) getword(HDR_DATBLK) * 512L;

	/* read the resident data structure */
	if (lseek(datafd, saveoff, 0) < 0)
		error("bad data file");
	TMPINT = read(datafd, data, (int) length);
	if (TMPINT != (int) length)
		error("bad data file");
	complement(data, length);

	/* get the table base addresses */
	wtable = data + (woff = getword(HDR_WTABLE));
	wtypes = data + getword(HDR_WTYPES) - 1;
	otable = data + (ooff = getword(HDR_OTABLE));
	atable = data + (aoff = getword(HDR_ATABLE));
	vtable = data + (voff = getword(HDR_VTABLE));

	/* get the save data area */
	saveoff += (long) getword(HDR_SAVE);
	save = data + getword(HDR_SAVE);
	slen = getword(HDR_SLEN);

	/* get the base of the data and code spaces */
	dbase = data + getword(HDR_DBASE);
	cbase = data + getword(HDR_CBASE);

	/* initialize the message routines */
	msg_init(datafd, getword(HDR_MSGBLK));

	/* get the code pointers */
	h_init = getword(HDR_INIT);
	h_update = getword(HDR_UPDATE);
	h_before = getword(HDR_BEFORE);
	h_after = getword(HDR_AFTER);
	h_error = getword(HDR_ERROR);

	/* get the table lengths */
	base = data;
	wcount = getword(woff);
	ocount = getword(ooff);
	acount = getword(aoff);
	vcount = getword(voff);

	/* setup the base of the resident data */
	base = dbase;

	/* set the object count */
	setvalue(V_OCOUNT, ocount);
}

/* db_save - save the current database */
int db_save(void)
{
	return (advsave(&hdr[HDR_ANAME], 20, save, slen) ? T : NIL);
}

/* db_restore - restore a saved database */
int db_restore(void)
{
	return (advrestore(&hdr[HDR_ANAME], 20, save, slen) ? T : NIL);
}

/* db_restart - restart the current game */
int db_restart(void)
{
	lseek(datafd, saveoff, 0);
	if (read(datafd, save, slen) != slen)
		return (NIL);
	complement(save, slen);
	setvalue(V_OCOUNT, ocount);
	longjmp(restart, 1);
}

/* complement - complement a block of memory */
void complement(char *adr, int len)
{
	for (; len--; adr++)
		*adr = ~(*adr + 30);
}

/* findword - find a word in the dictionary */
int findword(char *word)
{
	char sword[WRDSIZE + 1];
	int wrd, i;

	/* shorten the word */
	strncpy(sword, word, WRDSIZE);
	sword[WRDSIZE] = 0;

	/* look up the word */
	for (i = 1; i <= wcount; i++) {
		wrd = getwloc(i);
		if (strcmp(base + wrd + 2, sword) == 0)
			return (getword(wrd));
	}
	return (NIL);
}

/* wtype - return the type of a word */
int wtype(int wrd)
{
	return (wtypes[wrd]);
}

/* match - match an object against a name and list of adjectives */
int match(int obj, int noun, int *adjs)
{
	int *aptr;

	if (!hasnoun(obj, noun))
		return (FALSE);
	for (aptr = adjs; *aptr != NIL; aptr++)
		if (!hasadjective(obj, *aptr))
			return (FALSE);
	return (TRUE);
}

/* checkverb - check to see if this is a valid verb */
int checkverb(int *verbs)
{
	int act;

	/* look up the action */
	for (act = 1; act <= acount; act++)
		if (hasverb(act, verbs))
			return (act);
	return (NIL);
}

/* findaction - find an action matching a description */
int findaction(int *verbs, int preposition, int flag)
{
	int act, mask;

	/* look up the action */
	for (act = 1; act <= acount; act++) {
		if (preposition && !haspreposition(act, preposition))
			continue;
		if (!hasverb(act, verbs))
			continue;
		mask = ~getabyte(act, A_MASK);
		if ((flag & mask) == (getabyte(act, A_FLAG) & mask))
			return (act);
	}
	return (NIL);
}

/* getp - get the value of an object property */
int getp(int obj, int prop)
{
	int p;

	for (; obj; obj = getofield(obj, O_CLASS))
		if ((p = findprop(obj, prop)) != 0)
			return (getofield(obj, p));
	return (NIL);
}

/* setp - set the value of an object property */
int setp(int obj, int prop, int val)
{
	int p;

	for (; obj; obj = getofield(obj, O_CLASS))
		if ((p = findprop(obj, prop)) != 0)
			return (putofield(obj, p, val));
	return (NIL);
}

/* findprop - find a property */
int findprop(int obj, int prop)
{
	int n, i, p;

	n = getofield(obj, O_NPROPERTIES);
	for (i = p = 0; i < n; i++, p += 4)
		if ((getofield(obj, O_PROPERTIES + p) & ~P_CLASS) == prop)
			return (O_PROPERTIES + p + 2);
	return (NIL);
}

/* hasnoun - check to see if an object has a specified noun */
int hasnoun(int obj, int noun)
{
	while (obj) {
		if (inlist(getofield(obj, O_NOUNS), noun))
			return (TRUE);
		obj = getofield(obj, O_CLASS);
	}
	return (FALSE);
}

/* hasadjective - check to see if an object has a specified adjective */
int hasadjective(int obj, int adjective)
{
	while (obj) {
		if (inlist(getofield(obj, O_ADJECTIVES), adjective))
			return (TRUE);
		obj = getofield(obj, O_CLASS);
	}
	return (FALSE);
}

/* hasverb - check to see if this action has this verb */
int hasverb(int act, int *verbs)
{
	int link, word, *verb;

	/* get the list of verbs */
	link = getafield(act, A_VERBS);

	/* look for this verb */
	while (link != NIL) {
		verb = verbs;
		word = getword(link + L_DATA);
		while (*verb != NIL && word != NIL) {
			if (*verb != getword(word + L_DATA))
				break;
			verb++;
			word = getword(word + L_NEXT);
		}
		if (*verb == NIL && word == NIL)
			return (TRUE);
		link = getword(link + L_NEXT);
	}
	return (FALSE);
}

/* haspreposition - check to see if an action has a specified preposition */
int haspreposition(int act, int preposition)
{
	return (inlist(getafield(act, A_PREPOSITIONS), preposition));
}

/* inlist - check to see if a word is an element of a list */
int inlist(int link, int word)
{
	while (link != NIL) {
		if (word == getword(link + L_DATA))
			return (TRUE);
		link = getword(link + L_NEXT);
	}
	return (FALSE);
}

/* getofield - get a field from an object */
int getofield(int obj, int off)
{
	return (getword(getoloc(obj) + off));
}

/* putofield - put a field into an object */
int putofield(int obj, int off, int val)
{
	return (putword(getoloc(obj) + off, val));
}

/* getafield - get a field from an action */
int getafield(int act, int off)
{
	return (getword(getaloc(act) + off));
}

/* getabyte - get a byte field from an action */
int getabyte(int act, int off)
{
	return (getbyte(getaloc(act) + off));
}

/* getoloc - get an object from the object table */
int getoloc(int n)
{
	if (n < 1 || n > ocount)
		range("object", n);
	return (getdword(otable + n + n));
}

/* getaloc - get an action from the action table */
int getaloc(int n)
{
	if (n < 1 || n > acount)
		range("action", n);
	return (getdword(atable + n + n));
}

/* getvalue - get the value of a variable from the variable table */
int getvalue(int n)
{
	if (n < 1 || n > vcount)
		range("variable", n);
	return (getdword(vtable + n + n));
}

/* setvalue - set the value of a variable in the variable table */
int setvalue(int n, int v)
{
	if (n < 1 || n > vcount)
		range("variable", n);
	return (putdword(vtable + n + n, v));
}

/* getwloc - get a word from the word table */
int getwloc(int n)
{
	if (n < 1 || n > wcount)
		range("word", n);
	return (getdword(wtable + n + n));
}

/* getword - get a word from the data array */
int getword(int n)
{
	return (getdword(base + n));
}

/* putword - put a word into the data array */
int putword(int n, int w)
{
	return (putdword(base + n, w));
}

/* getbyte - get a byte from the data array */
int getbyte(int n)
{
	return (*(base + n) & 0xFF);
}

/* getcbyte - get a code byte */
int getcbyte(int n)
{
	return (*(cbase + n) & 0xFF);
}

/* getcword - get a code word */
int getcword(int n)
{
	return (getdword(cbase + n));
}

/* getdword - get a word from the data array */
int getdword(char *p)
{
	return (((*p & 0xFF) | (*(p + 1) << 8)) & 0xFFFF);
}

/* putdword - put a word into the data array */
int putdword(char *p, int w)
{
	*p = w;
	*(p + 1) = w >> 8;
	return (w);
}

/* range - handle errors with numeric arguments */
void range(char *what, int n)
{
	error(what);
	error(" out of range: ");
	pnumber(n);
	error(".\n");
}

/* parse - read and parse an input line */
int parse(void)
{
	if (!parse1())
		return (FALSE);
	setvalue(V_ACTOR, actor);
	setvalue(V_ACTION, action);
	setvalue(V_DOBJECT, dobject);
	setvalue(V_NDOBJECTS, ndobjects);
	setvalue(V_IOBJECT, iobject);
	return (TRUE);
}

/* next - get the next command (next direct object) */
int next(void)
{
	if (getvalue(V_NDOBJECTS) > 1) {
		setvalue(V_ACTOR, actor);
		setvalue(V_ACTION, action);
		setvalue(V_DOBJECT, getvalue(V_DOBJECT) + 1);
		setvalue(V_NDOBJECTS, getvalue(V_NDOBJECTS) - 1);
		setvalue(V_IOBJECT, iobject);
		return (TRUE);
	} else
		return (FALSE);
}

/* parse1 - the main parser */
int parse1(void)
{
	int noun1, cnt1, noun2, cnt2;
	int preposition, flag;

	/* initialize */
	noun1 = noun2 = NIL;
	cnt1 = cnt2 = 0;
	nptr = aptr = 0;
	preposition = 0;
	flag = 0;

	/* initialize the parser result variables */
	actor = action = dobject = iobject = NIL;
	ndobjects = 0;

	/* get an input line */
	if (!get_line())
		return (FALSE);

	/* check for actor */
	if (wtype(*wptr) == WT_ADJECTIVE || wtype(*wptr) == WT_NOUN) {
		if ((actor = getnoun()) == NIL)
			return (FALSE);
		flag |= A_ACTOR;
	}

	/* get verb phrase */
	if (!getverb())
		return (FALSE);

	/* direct object, preposition and indirect object */
	if (*wptr) {

		/* get the first set of noun phrases (direct objects) */
		noun1 = nptr + 1;
		for (;;) {

			/* get the next direct object */
			if (getnoun() == NIL)
				return (FALSE);
			++cnt1;

			/* check for more direct objects */
			if (*wptr == NIL || wtype(*wptr) != WT_CONJUNCTION)
				break;
			wptr++;
		}

		/* get the preposition and indirect object */
		if (*wptr) {

			/* get the preposition */
			if (wtype(*wptr) == WT_PREPOSITION)
				preposition = *wptr++;

			/* get the second set of noun phrases (indirect object) */
			noun2 = nptr + 1;
			for (;;) {

				/* get the next direct object */
				if (getnoun() == NIL)
					return (FALSE);
				++cnt2;

				/* check for more direct objects */
				if (*wptr == NIL || wtype(*wptr) != WT_CONJUNCTION)
					break;
				wptr++;
			}
		}

		/* make sure this is the end of the sentence */
		if (*wptr) {
			parse_error();
			return (FALSE);
		}
	}

	/* setup the direct and indirect objects */
	if (preposition) {
		if (cnt2 > 1) {
			parse_error();
			return (FALSE);
		}
		dobject = noun1;
		ndobjects = cnt1;
		iobject = noun2;
	} else if (noun2) {
		if (cnt1 > 1) {
			parse_error();
			return (FALSE);
		}
		preposition = findword("to");
		dobject = noun2;
		ndobjects = cnt2;
		iobject = noun1;
	} else {
		dobject = noun1;
		ndobjects = cnt1;
	}

	/* setup the flags for the action lookup */
	if (dobject)
		flag |= A_DOBJECT;
	if (iobject)
		flag |= A_IOBJECT;

	/* find the action */
	if ((action = findaction(verbs, preposition, flag)) == NIL) {
		parse_error();
		return (FALSE);
	}

	/* return successfully */
	return (TRUE);
}

/* getverb - get a verb phrase and return the action it refers to */
int getverb(void)
{
	/* get the verb */
	if (*wptr == NIL || wtype(*wptr) != WT_VERB) {
		parse_error();
		return (NIL);
	}
	verbs[0] = *wptr++;
	verbs[1] = NIL;

	/* check for a word following the verb */
	if (*wptr) {
		verbs[1] = *wptr;
		verbs[2] = NIL;
		if (checkverb(verbs))
			wptr++;
		else {
			verbs[1] = words[wcnt - 1];
			if (checkverb(verbs))
				words[--wcnt] = NIL;
			else {
				verbs[1] = NIL;
				if (!checkverb(verbs)) {
					parse_error();
					return (NIL);
				}
			}
		}
	}
	return (T);
}

/* getnoun - get a noun phrase and return the object it refers to */
int getnoun(void)
{
	/* initialize the adjective list pointer */
	adjectives[nptr] = adjs + aptr;

	/* get the optional article */
	if (*wptr != NIL && wtype(*wptr) == WT_ARTICLE)
		wptr++;

	/* get optional adjectives */
	while (*wptr != NIL && wtype(*wptr) == WT_ADJECTIVE) {
		adjs[aptr] = *wptr++;
		anums[aptr] = wptr - words - 1;
		aptr++;
	}
	adjs[aptr++] = 0;

	/* get the noun itself */
	if (*wptr == NIL || wtype(*wptr) != WT_NOUN) {
		parse_error();
		return (NIL);
	}

	/* save the noun */
	nouns[nptr] = *wptr++;
	nnums[nptr] = wptr - words - 1;
	return (++nptr);
}

/* get_line - get the input line and lookup each word */
int get_line(void)
{
	/* read an input line */
	trm_chr(':');
	if ((lptr = trm_get(line)) == NULL) {
		trm_str("Speak up!  I can't hear you!\n");
		return (FALSE);
	}

	/* get each word on the line */
	for (wcnt = 0; skip_spaces(); wcnt++)
		if (get_word() == NIL)
			return (FALSE);
	words[wcnt] = NIL;

	/* check for a blank line */
	if (wcnt == 0) {
		trm_str("Speak up!  I can't hear you!\n");
		return (FALSE);
	}

	/* point to the first word and return successfully */
	wptr = words;
	return (TRUE);
}

/* skip_spaces - skip leading spaces */
int skip_spaces(void)
{
	while (spacep(*lptr))
		lptr++;
	return (*lptr != EOS);
}

/* show_noun - show a noun phrase */
void show_noun(int n)
{
	int adj, *p;

	/* print the adjectives */
	for (p = adjectives[n - 1], adj = FALSE; *p; p++, adj = TRUE) {
		if (adj)
			trm_chr(' ');
		trm_str(wtext[anums[p - adjs]]);
	}

	/* print the noun */
	if (adj)
		trm_chr(' ');
	trm_str(wtext[nnums[n - 1]]);
}

/* get_word - get the next word */
int get_word(void)
{
	int ch;

	/* get the next word */
	for (wtext[wcnt] = lptr; (ch = *lptr) != EOS && !spacep(ch);)
		*lptr++ = (isupper(ch) ? tolower(ch) : ch);
	if (*lptr != EOS)
		*lptr++ = EOS;

	/* look up the word */
	if ((words[wcnt] = findword(wtext[wcnt])) != 0)
		return (words[wcnt]);
	else {
		trm_str("I don't know the word \"");
		trm_str(wtext[wcnt]);
		trm_str("\".\n");
		return (NIL);
	}
}

/* spacep - is this character a space? */
int spacep(int ch)
{
	return (ch == ' ' || ch == ',' || ch == '.');
}

/* parse_error - announce a parsing error */
void parse_error(void)
{
	trm_str("I don't understand.\n");
}

/* trm_init - initialize the terminal module */
void trm_init(int rows, int cols, char *name)
{
	/* initialize the terminal i/o variables */
	maxcol = cols - 1;
	col = 0;
	maxrow = rows - 1;
	row = 0;
	wordptr = theWord;
	wordcnt = 0;
	scnt = 0;

}

/* trm_done - finish terminal i/o */
void trm_done(void)
{
	if (wordcnt)
		trm_word();
}

/* trm_get - get a line */
char *trm_get(char *line)
{
	if (wordcnt)
		trm_word();
	while (scnt--)
		putchr(' ');
	row = col = scnt = 0;
	return (trm_line(line));
}

/* trm_str - output a string */
void trm_str(const char *str)
{
	while (*str)
		trm_chr(*str++);
}

/* trm_xstr - output a string without logging or word wrap */
void trm_xstr(char *str)
{
	writes(str);
}

/* trm_chr - output a character */
void trm_chr(int ch)
{
	switch (ch) {
	case ' ':
		if (wordcnt)
			trm_word();
		scnt++;
		break;
	case '\t':
		if (wordcnt)
			trm_word();
		scnt = (col + 8) & ~7;
		break;
	case '\n':
		if (wordcnt)
			trm_word();
		trm_eol();
		scnt = 0;
		break;
	default:
		if (wordcnt < WORDMAX) {
			*wordptr++ = ch;
			wordcnt++;
		}
		break;
	}
}

/* trm_word - output the current word */
void trm_word(void)
{
	if (col + scnt + wordcnt > maxcol)
		trm_eol();
	else
		while (scnt--) {
			putchr(' ');
			col++;
		}
	col += write(1, theWord, wordcnt);
	wordptr = theWord;
	wordcnt = 0;
	scnt = 0;
}

/* trm_eol - end the current line */
void trm_eol(void)
{
	putchr('\n');
	if (++row >= maxrow) {
		trm_wait();
		row = 0;
	}
	col = 0;
}

/* trm_wait - wait for the user to type return */
void trm_wait(void)
{
	trm_xstr("  << MORE >>");
	while (getchr() != '\n');
	trm_xstr("            \r");
}

/* trm_line - get an input line */
char *trm_line(char *line)
{
	char *p;
	int ch;

	p = line;
	while ((ch = getchr()) != -1 && ch != '\n')
		if ((p - line) < LINEMAX)
			*p++ = ch;
	*p = 0;
	return (ch == -1 ? NULL : line);
}

/* getchr - input a single character */
int getchr(void)
{
	uint8_t c;
	if (read(0, &c, 1) != 1)
		return -1;
	return c;
}

/* putchr - output a single character */
void putchr(char ch)
{
	write(1, &ch, 1);	/* FIXME buffering */
}

/* msg_init - initialize the message routines */
void msg_init(int fd, int base)
{
	char *p;
	int i;

	/* remember the message file descriptor and base */
	mbase = base;
	mfd = fd;
	p = mbufdata;
	for (i = 0; i < CSIZE; i++) {
		mbuffer[i] = p;
		p += 512;
		mblock[i] = -1;
		mnext[i] = i + 1;
	}
	mhead = 0;
	mtail = CSIZE - 1;
	mnext[mtail] = -1;
}

/* msg_open - open a message */
void msg_open(unsigned int msg)
{
	/* save the current message block */
	mblk = msg >> 7;

	/* make sure the first block is in a buffer */
	get_block(mblk);

	/* setup the initial offset into the block */
	moff = (msg & 0x7F) << 2;
}

/* msg_byte - get a byte from a message */
int msg_byte(void)
{
	/* check for end of block and get next block */
	if (moff >= 512) {
		get_block(++mblk);
		moff = 0;
	}

	/* return the next message byte */
	return (decode(mbuf[moff++]));
}

/* decode - decode a character */
int decode(int ch)
{
	return ((ch + 30) & 0xFF);
}

/* get_block - get a block of message text */
void get_block(unsigned int blk)
{
	int last, n;
	long loff;

	/* first check the cache */
	for (n = mhead; n != -1; last = n, n = mnext[n])
		if (blk == mblock[n]) {
			if (n != mhead) {
				if ((mnext[last] = mnext[n]) == -1)
					mtail = last;
				mnext[n] = mhead;
				mhead = n;
			}
			mbuf = mbuffer[n];
			return;
		}

	/* overwrite the least recently used buffer */
	mblock[mtail] = blk;
	loff = ((long) mbase + (long) blk) << 9;

	lseek(mfd, loff, 0);
	if (read(mfd, mbuffer[mtail], 512) != 512)
		error("error reading message text");

	/* get the block */
	get_block(blk);
}
