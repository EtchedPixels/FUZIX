//#define GRAPHICS


/* 

    QRun: Quill game runner for Fuzix.
    Copyright (C) 2015  Alan Cox
    
    Derived from
 
    UnQuill: Disassemble games written with the "Quill" adventure game system
    Copyright (C) 1996-2000  John Elliott

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

                             - * -
*/

/* Pull Quill data from a .SNA file.  *
 * Output to stdout.                  */

/* Format of the Quill database:

 Somewhere in memory (usually 6D04h for version 'A' and 6B85h for version 'C')
there is a colour definition table:

	DEFB	10h,ink,11h,paper,12h,flash,13h,bright,14h,inverse,15h,over
	DEFB	border

We use this as a magic number to find the Quill database in memory. After
the colour table is a table of values:

	DEFB	no. of objects player can carry at once
	DEFB	no. of objects
	DEFB	no. of locations
	DEFB	no. of messages

then a table of pointers:

	DEFW	rsptab	;Response table
	DEFW	protab	;Process table
	DEFW	objtop	;Top of object descriptions
	DEFW	loctop	;Top of locations.
	DEFW	msgtop	;Top of messages.
	**
	DEFW	conntab	;Table of connections.
	DEFW	voctab	;Table of words.
	DEFW	oloctab	;Table of object inital locations.
	DEFW	free	;[Early] Start of free memory
                    ;[Late]  Word/object mapping
	DEFW	freeend	;End of free memory
	DEFS	3	;?
	DEFB	80h,dict ;Expansion dictionary (for compressed games).
			         ;Dictionary is stored in ASCII, high bit set on last
			         ;letter of each word.
rsptab:	...		     ;response	- both stored as: DB verb, noun
protab: ...          ;process                     DW address-of-handler
                     ;                               terminated by verb 0.
                                         Handler format is:
                                          DB conditions...0FFh
                                          DB actions......0FFh
objtab:	...		;objects	- all are stored with all bytes
objtop:	DEFW	objtab        complemented to deter casual hackers.
        DEFW    object1
        DEFW    object2 etc.
loctab:	...		;locations       Texts are terminated with 0E0h
loctop:	DEFW    loctab           (ie 1Fh after decoding).
        DEFW    locno1
        DEFW    locno2 etc.
msgtab:	...		;messages
msgtop:	DEFW    msgtab
        DEFW    mess1
        DEFW    mess2 etc.
conntab: ...    ;connections    - stored as DB WORD,ROOM,WORD,ROOM...
                                  each entry is terminated with 0FFh.

voctab: ...		;vocabulary     - stored as DB 'word',number - the
                                  word is complemented to deter hackers.
                                  Table terminated with a word entry all
                                  five bytes of which are 0 before
                                  decoding.
oloctab: ...    ;initial locations of objects. 0FCh => not created
                                               0FDh => worn
                                               0FEh => carried
                                               0FFh => end of table

In later games (those made with Illustrator?), there is an extra byte 
(the number of system messages) after the number of messages, and at 
the label ** the following word is inserted:

        DEFW    systab  ;Points to word pointing to table of system
                    ;messages.
systab: DEFW    sysm0
        DEFW    sysm1 etc.

The address of the user-defined graphics is stored at 23675. In "Early" games,
the system messages are at UDGs+168.

CPC differences:

* I don't know where the UDGs are.
* Strings are terminated with 0xFF (0 after decoding) rather than 0xE0 
  (0x1F).
* No Spectrum colour codes in strings. Instead, the 
  code 0x14 means "newline" and 0x09 means "toggle reverse video".
  There are other CPC colour codes in the range 0x01-0x0A, but I don't
  know their meaning.
* I have assumed that the database always starts at 0x1BD1, which it does 
  in the snapshots I have examined.
  
Commodore 64 differences:

* I don't know where the UDGs are.
* Strings are terminated with 0xFF (0 after decoding) rather than 0xE0 
  (0x1F).
* No Spectrum colour codes in strings. 
* I have assumed that the database always starts at 0x0804, which it does 
  in the snapshots I have examined.

  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#ifndef __linux__
#include "sys/graphics.h"
#endif

#define VERSION "0.2"

typedef uint8_t uchr;	/* for brevity */
typedef uint16_t ushrt;	/* -- ditto -- */

#define ZBUF_NUM		32	/* 8K */
#define NIL  100
#define LOC  101
#define MSG  102
#define OBJ  103
#define SWAP 104
#define PLC  105

static char getch(void);

static uchr zmem(ushrt);
static ushrt zword(ushrt);
static void clrscr(void);
static void oneitem(ushrt, uchr);
static void opch32(char);
static void spc(void);
static void nl(void);
static void opstr32(const char *);
static void expch(uchr, ushrt *);
static void expdict(uchr, ushrt *);

static char present(uchr);
static uchr doproc(ushrt, uchr, uchr);
static void listat(uchr);
static void initgame(void);
static void playgame(ushrt);
static void sysmess(uchr);
static uchr ffeq(uchr, uchr);
static uchr condtrue(ushrt);
static uchr autobj(uchr);
static uchr runact(ushrt, uchr);
static uchr cplscmp(ushrt, char *);
static uchr matchword(char **);
static void usage(void);
static void dec32(ushrt);
static char yesno(void);
static void savegame(void);
static void loadgame(void);

static void initdisplay(void);
static void drawlocation(void);

/* Supported architectures */
#define ARCH_SPECTRUM 0
#define ARCH_CPC      1
#define ARCH_C64      2

static char arch = ARCH_SPECTRUM;	/* Architecture */
static char xpos = 0;		/* Used in msg display */
static char dbver = 0;		/* Which format of database? 0 = early Spectrum
				 *                          10 = later Spectrum/CPC */
static ushrt vocab, dict;	/* Spectrum address of vocabulary & dictionary */
static char nobeep = 0;		/* Peace and quiet? */
static char running = 1;	/* Actually playing the game? */
static ushrt loctab;
static ushrt objtab;		/* Tables of locations, objects, messages */
static ushrt msgtab;
static ushrt sysbase;		/* Base of system messages */
static ushrt conntab;		/* Connections table */
static ushrt postab;		/* Object start positions table */
static ushrt objmap;		/* Word-to-object map */
static ushrt proctab, resptab;	/* Process and Response tables */
static char *inname;		/* Input filename */

static uchr fileid = 0xFF;	/* Save file identity byte */

struct gstate {
	uint8_t v2;
	uint8_t v1;
	uint8_t fileid;
	uchr flags[37];		/* The Quill flags, 0-36. */
#define TURNLO state.flags[31]	/* (31-36 are special */
#define TURNHI state.flags[32]	/* Meanings of the special flags */
#define VERB state.flags[33]
#define NOUN state.flags[34]
#define CURLOC state.flags[35]	/* state.flags[36] == 0 */
	uchr objpos[219];	/* Positions of objects */
	uchr xor;
};

static struct gstate state;
static struct gstate ramsave;	/* RAM save buffer */

static uchr maxcar;		/* Max no. of portable objects */
static uchr maxcar1;		/* As maxcar - later games can change it on the fly */
#define NUMCAR state.flags[1]		/* Number of objects currently carried */
static uchr nobj;		/* No. of objects */
static uchr nsys;		/* No. of system messages */
static uchr alsosee = 1;	/* Message "You can also see" */
static uchr estop;		/* Emergency stop flag */

static int infile;

static uchr found, nmsg, nloc;
static ushrt mem_base;		/* Minimum address loaded from snapshot     */
static ushrt mem_size;		/* Size of memory loaded from snapshot      */
static short mem_offset;	/* Load address of snapshot in physical RAM */
static char snapid[20];

static int fast, slow, miss;

static uchr console;		/* Console graphics mode */

#define AV0 argv[0]

#define AV10 argv[1][0]
#define AV11 argv[1][1]

#define isoptch(c) ( (c) == '-' )

/* Keep static for cc65. Used for snapshot headers and also for save
   load filename */
static unsigned char header[256];


void errstr(const char *t1, const char *t2)
{
	write(2, t1, strlen(t1));
	if (t2) {
		write(2, ": ", 2);
		write(2, t2, strlen(t2));
	}
	write(2, "\n", 1);
}

#define debugstr(x)

void readbuf(char *p, int len)
{
	char *ep = p + len - 1;
	while (p < ep && read(0, p, 1)) {
		if (*p == '\n') {
			p++;
			break;
		}
		p++;
	}
	*p = 0;
}

static ushrt check_signature(ushrt n);

static ushrt ucptr;

int main(int argc, char *argv[])
{
	ushrt n, zxptr = 0;

/* If command looks like a help command, print helpscreen */

	while(*++argv && **argv == '-') {
		switch(**argv) {
			default:
			case 'h':
				usage();
			case 'l':			
				dbver = 10;
				break;
			case 'q':
				nobeep = 1;
				break;
		}		/* End For */
	}			/* End If */

	/* Load the snapshot. To save space, we ignore the printer
	   buffer and the screen, which can contain nothing of value. */

	if (*argv == NULL || argv[1])
		usage();

	inname = *argv;

	if ((infile = open(inname, O_RDONLY)) < 0) {
		perror(inname);
		exit(1);
	}

	/* << v0.7: Check for CPC6128 format */
	if (read(infile, snapid, sizeof(snapid)) != sizeof(snapid)) {
		perror(inname);
		exit(1);
	}

	arch = ARCH_SPECTRUM;
	mem_base = 0x5C00;	/* First address loaded */
	mem_size = 0xA400;	/* Number of bytes to load from it */
	mem_offset = 0x3FE5;	/* Load address of snapshot in host system memory */

	if (!memcmp(snapid, "MV - SNA", 9)) {	/* CPCEMU snapshot */
		arch = ARCH_CPC;

		mem_base = 0x1B00;
		mem_size = 0xA500;
		mem_offset = -0x100;
		dbver = 10;	/* CPC engine is equivalent to  
				 * the "later" Spectrum one. */

		debugstr("CPC snapshot signature found.");
	}
	if (!memcmp(snapid, "VICE Snapshot File\032", 19)) {	/* VICE snapshot */
		int nv;

		arch = ARCH_C64;
		memset(header, 0, sizeof(header));
		/* [0.8.7] Do a quick, minimal parse of the VSF to find the 
		 * C64MEM block. Earlier unquills assumed it would be at a 
		 * fixed offset; it isn't. */
		lseek(infile, 0L, SEEK_SET);
		if (read(infile, header, sizeof(header)) != sizeof(header)) {
			errstr("Warning: Failed to read C64 snapshot header",
			       NULL);
		}

		mem_base = 0x800;
		mem_size = 0xA500;
		mem_offset = -0x74;
		for (nv = 0; nv < (int)sizeof(header) - 6; nv++) {
			if (!memcmp(header + nv, "C64MEM", 6)) {
				mem_offset = -(nv + 0x1A);
				break;
			}
		}

		dbver = 5;	/* C64 engine is between the two Spectrum 
				 * ones. */

		debugstr("C64 snapshot signature found.");
	}
	/* >> v0.7 */

	/* Skip screen/printer buffer/registers and load the rest */

	/* .SNA read ok. Find a Quill signature */

	switch (arch) {
	case ARCH_SPECTRUM:

		/* I could _probably_ assume that the Spectrum database is 
		 * always at one location for "early" and another for "late"
		 * games (0x6D04 for "early", 0x6B85 for "late". Try these
		 * first; if that fails, scan the whole file. */

		if (check_signature(0x6D04)) {
			dbver = 0;
			found = 1;
			zxptr = 0x6D11;
			break;
		}
		if (check_signature(0x6B85)) {
			dbver = 10;
			found = 1;
			zxptr = 0x6B92;
			break;
		}

		for (n = 0x5C00; n < 0xFFF5; n++) {
			if (check_signature(n)) {
				debugstr("Quill signature found.");
				found = 1;
				zxptr = n + 13;
				break;
			}
		}
		break;

	case ARCH_CPC:
		found = 1;
		zxptr = 0x1BD1;	/* From guesswork: CPC Quill files
				 * always seem to start at 0x1BD1 */
		break;
	case ARCH_C64:
		found = 1;
		zxptr = 0x804;	/* From guesswork: C64 Quill files
				 * always seem to start at 0x804 */
		break;
	}

	if (!found) {
		errstr("Not a valid Quill .SNA file", inname);
		exit(1);
	}

	ucptr = zxptr;
	maxcar1 = maxcar = zmem(zxptr);	/* Player's carrying capacity */
	nobj = zmem(zxptr + 1);	/* No. of objects */
	nloc = zmem(zxptr + 2);	/* No. of locations */
	nmsg = zmem(zxptr + 3);	/* No. of messages */
	if (dbver) {
		++zxptr;
		nsys = zmem(zxptr + 3);	/* No. of system messages */
		vocab = zword(zxptr + 18);	/* Words list */
		dict = zxptr + 29;	/* Expansion dictionary */
	} else
		vocab = zword(zxptr + 16);

	resptab = zword(zxptr + 4);
	proctab = zword(zxptr + 6);
	objtab = zword(zxptr + 8);
	loctab = zword(zxptr + 10);
	msgtab = zword(zxptr + 12);
	if (dbver)
		sysbase = zword(zxptr + 14);
	else
		sysbase = zword(23675) + 168;	/* Just after the UDGs */
/*	fprintf(stderr, "sysbase at %u, udg at %u\n", 
		sysbase, zword(23675));*/
	conntab = zword(zxptr + 14 + (dbver ? 2 : 0));
	if (dbver)
		objmap = zword(zxptr + 22);
	postab = zword(zxptr + 18 + (dbver ? 2 : 0));

	/* Run the game */

	while (running) {
		estop = 0;
		srand(1);
		initdisplay();
		initgame();		/* Initialise the game */
		playgame(zxptr);	/* Play it */
		if (estop) {
			estop = 0;	/* Emergency stop operation, game restarts */
			continue;	/* automatically */
		}
		sysmess(13);
		nl();
		if (yesno() == 'N') {
			running = 0;
			sysmess(14);
			nl();
		}
	}
/*	printf("Fast %d Slow %d Miss %d\n", fast, slow, miss);*/
	return 0;
}				/* End main() */

static void usage(void)
{
	errstr("-I  : Use disk graphics.",
	       NULL);
	errstr("-L  : Attempt to interpret as a 'later' type Quill file.",
	       NULL);
	errstr("-Q : Quiet (no beeping)", NULL);
	exit(1);
}

/* Based on an idea by Staffan Vilcans: Skip the fseek() if it isn't needed */
static ushrt last_addr = 0;
static uchr *last_ptr;

static uchr zbuf[ZBUF_NUM][256];
uchr zbuf_addr[ZBUF_NUM];
uchr zbuf_pri[ZBUF_NUM];	/* 0 = unused , 1+ is use count */

static uchr zbuf_alloc(void)
{
	uchr low = 255;
	uchr i, lnum = 0;
	for (i = 0; i < ZBUF_NUM; i++) {
		if (zbuf_pri[i] == 0)
			return i;
		if (zbuf_pri[i] < low) {
			lnum = i;
			low = zbuf_pri[i];
		}
	}
	return lnum;
}

static void zbuf_sweep(void)
{
	uchr i;
	for (i = 0; i < ZBUF_NUM; i++)
		if (zbuf_pri[i] > 1)
			zbuf_pri[i] /= 2;
}

static void zbuf_load(uchr slot, ushrt addr)
{
	/* FIXME: SNA files are not nicely block aligned so we might want
	   to align to their wacky offsets, or just split off the loader */
	uchr ah = (addr >> 8);
	zbuf_addr[slot] = ah;
	zbuf_pri[slot] = 0x80;
	addr &= 0xFF00;
	if (lseek(infile, addr - mem_offset, SEEK_SET) < 0 ||
	    read(infile, zbuf[slot], 256) != 256) {
		perror(inname);
		exit(1);
	}
}

static uchr zbuf_find(ushrt addr)
{
	uchr ah = addr >> 8;
	uchr i;
	for (i = 0; i < ZBUF_NUM; i++) {
		if (zbuf_addr[i] == ah) {
			slow++;
			zbuf_pri[i] |= 0x80;
			return i;
		}
	}
	zbuf_sweep();
	i = zbuf_alloc();
	zbuf_load(i, addr);
	miss++;
	return i;
}

static uchr zmem(ushrt addr)
{
	uchr c;

	/* Fast path - current buffer */
	if (addr == last_addr + 1) {
		if (!((last_addr ^ addr) & 0xFF00)) {
			fast++;
			last_addr++;
			return *++last_ptr;
		}
	}

	/* All Spectrum memory accesses are routed through this procedure.
	 * If TINY is defined, this accesses the .sna file directly.
	 */

	if (addr < mem_base || (arch != ARCH_SPECTRUM &&
				(addr >= (mem_base + mem_size)))) {
/*		fprintf(stderr, "addr %d mb %d, arch %d\n",
			addr, mem_base, arch);*/
		errstr("Invalid address requested", NULL);
		exit(1);
	}
	/* Find the right buffer */
	c = zbuf_find(addr);
	last_addr = addr;
	last_ptr = zbuf[c] + (addr & 0xFF);
	return *last_ptr;
}

static ushrt zword(ushrt addr)
{
	return (ushrt) (zmem(addr) + (256 * zmem(addr + 1)));
}

static void dec32(ushrt v)
{
	char h = 0, t = 0;

	while (v >= 100) {
		h++;
		v -= 100;
	}
	while (v >= 10) {
		t++;
		v -= 10;
	}
	if (h)
		opch32(h + '0');
	if (h || t)
		opch32(t + '0');
	opch32(v + '0');
}


static void nl(void)
{
	opch32('\n');
}

static void spc(void)
{
	opch32(' ');
}

static void opstr32(const char *s)
{
	while (*s)
		opch32(*s++);
}

static ushrt check_signature(ushrt n)
{
	if ((zmem(n) == 0x10) && (zmem(n + 2) == 0x11) &&
	    (zmem(n + 4) == 0x12) && (zmem(n + 6) == 0x13) &&
	    (zmem(n + 8) == 0x14) && (zmem(n + 10) == 0x15))
		return 1;
	return 0;
}

static char getch(void)
{
	struct termios ts, ots;
	char c;
	int i;

	tcgetattr(0, &ts);	/* Switch the terminal to raw mode */
	tcgetattr(0, &ots);
	cfmakeraw(&ts);
	tcsetattr(0, TCSANOW, &ts);

	do {
		i = read(0, &c, 1);
	}
	while (i < 0);

	tcsetattr(0, TCSANOW, &ots);

	return c;
}

static uchr runact(ushrt ccond, uchr noun)
{
/* Conditions have been met; execute the actions */
/* WARNING: This procedure contains goto statements - take care! */

	uchr cact, cdone = 0, cobj, nout, n;
/*	time_t wait1; */
	int nv;

	cact = zmem(ccond);	/* Action byte */

	while ((cact != 0xFF) && !cdone) {
		/* Translate condition numbers for old Spectrum games */
		if ((!dbver) && (cact > 11))
			cact += 9;	/* Translate condition numbers */
		if ((!dbver) && (cact == 11))
			cact = 17;	/* for old-style games */
		if ((!dbver) && (cact > 29))
			cact++;

		/* Translate numbers for C64 games */
		if (dbver > 0 && dbver < 10 && cact >= 13)
			cact += 4;

		switch (cact) {
		case 0:	/* INVEN */
			nout = 0;
			nl();
			sysmess(9);
			nl();
			for (n = 0; n < nobj; n++) {
				if (state.objpos[n] == 254) {
					oneitem(objtab, n);
					nl();
					nout++;
				}
				if (state.objpos[n] == 253) {
					oneitem(objtab, n);
					sysmess(10);
					nl();
					nout++;
				}
			}
			if (nout == 0)
				sysmess(11);
			nl();
			break;
		case 1:	/* DESC */
			cdone = 2;
			break;
		case 2:	/* QUIT */
			sysmess(12);
			if (yesno() == 'N')
				cdone = 1;
			break;
		case 3:	/* END */
			cdone = 3;
			break;
		case 4:	/* DONE */
			cdone = 1;
			break;
		case 5:	/* OK */
			cdone = 1;
			sysmess(15);
			nl();
			break;
		case 6:	/* ANYKEY */
			sysmess(16);
			getch();
			nl();
			break;
		case 7:	/* SAVE */
			savegame();
			cdone = 2;
			break;
		case 8:	/* LOAD */
			loadgame();
			cdone = 2;
			break;
		case 9:	/* TURNS */
			sysmess(17);
			dec32(TURNLO + (256 * TURNHI));
			sysmess(18);
			if (TURNLO + (256 * TURNHI) != 1) {
				if (dbver > 0)
					sysmess(19);
				else
					opch32('s');
			}
			if (dbver > 0)
				sysmess(20);
			else
				opch32('.');
			nl();
			break;
		case 10:	/* SCORE */
			sysmess(19 + (dbver ? 2 : 0));
			dec32(state.flags[30]);
			if (dbver > 0)
				sysmess(22);
			else
				opch32('.');
			nl();
			break;
		case 11:	/* CLS */
			clrscr();
			break;
		case 12:	/* DROPALL (note: not a true DROP ALL) */
			for (cobj = 0; cobj < nobj; cobj++)
				if ((state.objpos[cobj] == 254)
				    || (state.objpos[cobj] == 253))
					state.objpos[cobj] = CURLOC;
			NUMCAR = 0;
			nl();
			break;
		case 13:	/* AUTOG  Warning - these four cases contain gotos */
			cobj = autobj(noun);
			/*	  if (cobj==0xFF) cdone=1; */
			goto get0001;
		case 14:	/* AUTOD */
			cobj = autobj(noun);
			/*	  if (cobj==0xFF) cdone=1; */
			goto drop0001;
		case 15:	/* AUTOW */
			if (noun < 200) {
				sysmess(8);
				cdone = 1;
				break;
			}	/* v0.5 */
			cobj = autobj(noun);
			/*	  if (cobj==0xFF) cdone=1; */
			goto wear0001;
		case 16:	/* AUTOR */
			if (noun < 200) {
				sysmess(8);
				cdone = 1;
				break;
			}	/* v0.5 */
			cobj = autobj(noun);
			/*	  if (cobj==0xFF) cdone=1; */
			goto rem0001;
		case 17:	/* PAUSE - and extra functions */
			n = zmem(++ccond);
			if ((dbver > 0) && (state.flags[28] < 0x17)) {
				uchr ybrk = 1;
				switch (state.flags[28]) {
				/* The following subfunctions of PAUSE cannot
				 * be implemented on this style of text-based
				 * terminal, or deliberately are no-ops.
				 */

				case 1:
				case 2:
				case 3:
				case 5:
				case 6:
					/* Sound effects... */
				case 16:
				case 17:
				case 18:
					/* Set the keyboard click */
				case 4:	/* Make the screen flicker */
				case 7:	/* Select font 1 */
				case 8:	/* Select font 2 */
				case 19:	/* Graphics on/off */
				case 20:	/* No-op */
					break;

				/* Subfunctions which can be implemented */

				case 9:
					clrscr();	/* Clear screen impressively */
					break;
				case 10:	/* Set the "you can also see" message */
					if (n < nsys)
						alsosee = n;
					break;
				case 11:	/* Set maxcar1 */
					maxcar1 = n;
					break;
				case 12:	/* Restart the game */
					alsosee = 1;
					fileid = 255;
					maxcar1 = maxcar;
					cdone = 3;	/* End game */
					estop = 1;	/* Emergency stop */
					break;
				case 13:	/* Reboot the Spectrum */
					/* FIXME: via game I/O */
					opstr32("\n\nGame terminated.\n");
					exit(2);
				case 14:	/* Increase number of portable objects */
					if ((255 - n) >= maxcar1)
						maxcar1 += n;
					else
						maxcar1 = 255;
					break;
				case 15:	/* Decrease number of portable objects */
					if (maxcar1 < n)
						maxcar1 = 0;
					else
						maxcar1 -= n;
					break;
				case 21:	/* RAMsave & RAMload */
					if (n == 50) {	/* RAMload */
						if (!ramsave.v2)
							break;
						memcpy(&ramsave, &state, sizeof(state));
					} else {	/* RAMsave */

						ramsave.v2 = 1;
						memcpy(&state, &ramsave, sizeof(state));
					}
					break;

				case 22:	/* Change identity byte in save file */
					fileid = n;
					break;

				default:	/* Cases 0, 23, 24 */
					ybrk = 0;
					break;	/* Pretend it's a normal pause */
				}
				state.flags[28] = 0;
				if (ybrk)
					break;
			}
			/* 0 means 256 50ths so this works fine */
			n--;
			n = (n + 3) / 5;
			/* Always delay a bit */
			if (n == 0)
				n = 1;
#ifndef __linux__
			/* Not portable but clock_nanosleep is even less so */
			_pause(n);
#endif
			break;
		case 19:	/* INK */
			if (arch == ARCH_CPC)
				++ccond;	/* On CPCs, INK has 2 parameters */
		case 18:
		case 20:	/* PAPER, and BORDER. Ignore. */
			++ccond;	/* (v0.5 Skip parameter byte!) */
			break;
		case 21:	/* GOTO */
			CURLOC = zmem(++ccond);
			break;
		case 22:	/* MESSAGE */
			oneitem(msgtab, zmem(++ccond));
			nl();
			break;
		case 26:	/* WEAR */
			cobj = zmem(++ccond);
 wear0001:		cdone = 1;
			if (state.objpos[cobj] != 254)
				sysmess(25 + (dbver ? 3 : 0));
			else {
				state.objpos[cobj] = 253;
				NUMCAR--;
				cdone = 0;
			}
			nl();
			break;
		case 24:	/* GET */
			cobj = zmem(++ccond);
 get0001:		cdone = 1;
			if (cobj == 0xFF)
				sysmess(8);
			else if (state.objpos[cobj] == 254)
				sysmess(22 + (dbver ? 3 : 0));
			else if (state.objpos[cobj] == 253)
				sysmess(26 + (dbver ? 3 : 0));
			else if (state.objpos[cobj] != CURLOC)
				sysmess(23 + (dbver ? 3 : 0));
			else if (maxcar1 == NUMCAR)
				sysmess(21 + (dbver ? 3 : 0));
			else {
				cdone = 0;
				state.objpos[cobj] = 254;
				NUMCAR++;
			}
			nl();
			break;
		case 25:	/* DROP */
			cobj = zmem(++ccond);
 drop0001:		cdone = 1;
			if (cobj == 0xFF)
				sysmess(8);
			else if ((state.objpos[cobj] != 254) && (state.objpos[cobj] != 253))
				sysmess(25 + (dbver ? 3 : 0));
			else {
				state.objpos[cobj] = CURLOC;
				NUMCAR--;
				cdone = 0;
			}
			nl();
			break;
		case 23:	/* REMOVE */
			cobj = zmem(++ccond);
 rem0001:		cdone = 1;
			if (state.objpos[cobj] != 253)
				sysmess(20 + (dbver ? 3 : 0));
			else if (maxcar1 == NUMCAR)
				sysmess(21 + (dbver ? 3 : 0));
			else {
				state.objpos[cobj] = 254;
				NUMCAR++;
				cdone = 0;
			}
			nl();
			break;
		case 27:	/* DESTROY */
			cobj = zmem(++ccond);
			if (state.objpos[cobj] == 254)
				NUMCAR--;
			state.objpos[cobj] = 252;
			break;
		case 28:	/* CREATE */
			cobj = zmem(++ccond);
			state.objpos[cobj] = CURLOC;
			break;
		case 29:	/* SWAP */
			cobj = zmem(++ccond);
			nout = zmem(++ccond);
			n = state.objpos[cobj];
			state.objpos[cobj] = state.objpos[nout];
			state.objpos[nout] = n;
			break;
		case 30:	/* PLACE */
			cobj = zmem(++ccond);
			nout = zmem(++ccond);
			if (state.objpos[cobj] == 254)
				NUMCAR--;
			state.objpos[cobj] = nout;
			break;
		case 31:	/* SET */
			state.flags[zmem(++ccond)] = 0xFF;
			break;
		case 32:	/* CLEAR */
			state.flags[zmem(++ccond)] = 0;
			break;
		case 33:	/* PLUS */
			n = zmem(++ccond);
			nv = state.flags[n] + zmem(++ccond);
			if (nv < 0)
				nv = 0;
			else if (nv > 255)
				nv = 255;
			state.flags[n] = nv;
			break;
		case 34:	/* MINUS */
			n = zmem(++ccond);
			nv = state.flags[n] - zmem(++ccond);
			if (nv < 0)
				nv = 0;
			else if (nv > 255)
				nv = 255;
			state.flags[n] = nv;
			break;
			break;
		case 35:	/* LET */
			n = zmem(++ccond);
			state.flags[n] = zmem(++ccond);
			break;
		case 36:	/* BEEP */
			if (!nobeep) {
				uchr b = 7;
				/* FIXME */
				write(1, &b, 1);
			}
			n = zmem(++ccond);
			ccond++;
/*			usleep(10000L * n); FIXME */
/*			printf("sleep %d\n", n);*/
			if (n > 100)
				sleep(n/100);
			break;
		default:
			errstr("Invalid action", NULL);
		}
		cact = zmem(++ccond);
	}
	return (cdone);
}

static uchr condtrue(ushrt ccond)
{
	uchr cond, ctrue = 1;
	uchr arg1;
	uchr fp, op;

	cond = zmem(ccond);	/* Condition byte */

	while ((cond != 0xFF) && ctrue) {	/* If ctrue = 0, condition invalid */
		arg1 = zmem(++ccond);	/* Always at least one cond arg */
		/* Hoist these for compilers not smart enough to do so */
		fp = state.flags[arg1];
		op = state.objpos[arg1];
		switch (cond) {
		case 0:	/* AT */
			ctrue = (arg1 == CURLOC);
			break;
		case 1:	/* NOTAT */
			ctrue = (arg1 != CURLOC);
			break;
		case 2:	/* ATGT */
			ctrue = (arg1 < CURLOC);
			break;
		case 3:	/* ATLT */
			ctrue = (arg1 > CURLOC);
			break;
		case 4:	/* PRESENT */
			ctrue = present(op);
			break;
		case 5:	/* ABSENT */
			ctrue = (!present(op));
			break;
		case 6:	/* WORN */
			ctrue = (op == 253);
			break;
		case 7:	/* NOTWORN */
			ctrue = (op != 253);
			break;
		case 8:	/* CARRIED */
			ctrue = (op == 254);
			break;
		case 9:	/* NOTCARR */
			ctrue = (op != 254);
			break;
		case 10:	/* CHANCE */
			ctrue = ((rand() % 100) < (arg1));
			break;
		case 11:	/* ZERO */
			ctrue = (fp == 0);
			break;
		case 12:	/* NOTZERO */
			ctrue = fp;
			break;
		case 13:	/* EQ */
			ctrue = (fp == zmem(++ccond));
			break;
		case 14:	/* GT */
			ctrue = (fp > zmem(++ccond));
			break;
		case 15:	/* LT */
			ctrue = (fp < zmem(++ccond));
			break;
		default:
			errstr("Unknown condition code", NULL);
			ctrue = 0;
		}
		cond = zmem(++ccond);
	}
	return (ctrue);
}

static void initgame(void)
{
	uchr n;
	ushrt obase;

	alsosee = 1;		/* Options possibly set by later games */
	fileid = 255;
	maxcar1 = maxcar;

	obase = postab;		/* Object initial positions table */
	for (n = 0; n < 36; n++)
		state.flags[n] = 0;

	NUMCAR = 0;
	for (n = 0; n < nobj; n++) {
		state.objpos[n] = zmem(obase + n);
		if (state.objpos[n] == 254)
			NUMCAR++;
	}
}

static void savegame(void)
{
	uchr xor = fileid, l, n;
	int savefd;

	state.v2 = 2;
	state.v1 = 1;
	state.fileid = fileid;
	for (n = 0; n < 37; n++)
		xor ^= state.flags[n];
	for (n = 0; n < 0xDB; n++)
		xor ^= state.objpos[n];
	state.xor = xor;
	nl();
	write(1, "Save game to file>", 18);
	readbuf((char *)header, sizeof(header));
	l = strlen((char *)header) - 1;
	if ((header[l] == '\r') || (header[l] == '\n'))
		header[l] = 0;

	/* FIXME: remove stdio */
	savefd = open(header,  O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if (savefd == -1) {
		/* FIXME - via game output.. */
		errstr("Could not create", (char *)header);
		return;
	}
	if (write(savefd, &state, sizeof(state)) != sizeof(state) ||
		close(savefd) == -1) {
		opstr32("Write error on ");
		opstr32((char *)header);
		nl();
		/* Doing a double close is harmless */
		close(savefd);
		return;
	}
}

/* Format of Quill save file (based on a Spectrum .TAP file):

DW 0102h	;Length / magic no.
DB 0FFh		;Block type ("fileid") - usually 0FFh, but can be changed in
		;later games by PAUSE, subfunction 22.
DS 31		;Flags 0-30
DW xx		;Flags 31-32 = no. of turns
DB xx		;Flag  33    = verb
DB xx		;Flag  34    = noun
DW xx		;Flags 35-36 = location (flag 36 is always 0)
DS 0DBh		;Object locations table, terminated with 0FFh.
DB xsum		;XOR of all bytes except the 0102h.
*/

static struct gstate load;

static void loadgame(void)
{
	uchr xor = 0, l;
	ushrt n;
	int loadfd;
	uint8_t *p = ((uint8_t *)&load) + 2;

	nl();
	write(1, "Load game from file>", 14);

	readbuf((char *)header, sizeof(header));
	l = strlen((char *)header) - 1;
	if ((header[l] == '\r') || (header[l] == '\n'))
		header[l] = 0;

	loadfd = open(header, O_RDONLY);
	if (loadfd == -1) {
		opstr32("Could not open ");
		opstr32((char *)header);
		nl();
		return;
	}
	if (read(loadfd, &load, sizeof(load)) != sizeof(load)) {
		close(loadfd);
		opstr32("Read error on ");
		opstr32((char *)header);
		nl();
		return;
	}
	for (n = 2; n < 0x103; n++)
		xor ^= *p++;
	if ((xor != load.xor)
	    || (load.v2 != 2)
	    || (load.v1 != 1)
	    || (load.fileid != fileid)) {
		opstr32((char *)header);
		opstr32(" is not a suitable save file\nPress RETURN...");
		getch();
		return;
	}
	memcpy(&state, &load, sizeof(state));
}

static char present(uchr loc)
{
	if (loc == 254 || loc == 253 || loc == CURLOC)
		return 1;
	return 0;
}

static void playgame(ushrt zxptr)
{
	uchr desc = 1, verb, noun, pn, r;
	char *lbstart;
	ushrt connbase;
	char linebuf[80];

	lbstart = linebuf;
	while (1) {		/* Main loop */
		if (desc) {
			clrscr();
			/* 0.7.5: Darkness */
			if (state.flags[0] && (!present(state.objpos[0]))) {
				sysmess(0);
				nl();
			} else {
				drawlocation();
				oneitem(loctab, CURLOC);
				nl();
				listat(CURLOC);	/* List objects present */
			}
			desc = 0;

			/* Decrement flags depending on location descriptions */

			if (state.flags[2])
				state.flags[2]--;
			if (state.flags[0] && state.flags[3])
				state.flags[3]--;
			if (state.flags[0] && (!present(0)) && state.flags[4])
				state.flags[4]--;
		}

		/* [new in 0.7.0: Flag decrements moved to *after* the
		 * process table; this makes Bugsy work properly */

		/* Process "process" conditions */

		verb = 0xFF;
		noun = 0xFF;

		r = doproc(zword(zxptr + 6), verb, noun);	/* The Process Table */
		if (r == 2)
			desc = 1;	/* DESC */
		else if (r == 3)
			break;	/* END  */
		else {
			/* Decrement flags not depending on location descriptions */

			if (state.flags[5])
				state.flags[5]--;
			if (state.flags[6])
				state.flags[6]--;
			if (state.flags[7])
				state.flags[7]--;
			if (state.flags[8])
				state.flags[8]--;
			if (state.flags[0] && state.flags[9])
				state.flags[9]--;
			if (state.flags[0] && (!present(0)) && state.flags[10])
				state.flags[10]--;

			/* Print the prompt */

			pn = (rand() & 3);
			sysmess(pn + 2);
			nl();
			if (dbver == 0)
				sysmess(28);
			readbuf(linebuf, sizeof(linebuf));

			TURNLO++;
			if (TURNLO == 0)
				TURNHI++;

			/* Parse the input */

			lbstart = linebuf;
			verb = matchword(&lbstart);
			if (verb != 0xFF) {
				VERB = verb;
				noun = matchword(&lbstart);
				NOUN = noun;
//                              if (noun == 0xFF) noun = 0xFE;

				/* v0.7.5: Moved "response" conditions to after the attempt to 
				 *         move the player
				 */

				/* Attempt to move player */
				r = 0;
				/*				if (verb < 20)*//* A movement word       */
/*				{  * This test is incorrect; Quill does not *
 *				   * insist on the word number being < 20   */
				connbase = zword(2 * CURLOC + conntab);
				while ((!r) && (zmem(connbase) != 0xFF))
					if (verb == zmem(connbase)) {
						CURLOC = zmem(++connbase);
						desc = 1;
						r = 1;
					} else
						connbase += 2;
				/* } */
				if (r == 0) {
					/* Process "response" conditions */
					r = doproc(zword(zxptr + 4), verb,
						   noun);
					if (r == 2)
						desc = 1;	/* DESC */
					else if (r == 3)
						break;	/* END  */
				}
				/* Print "I can't do that/go that way" */
				if (r == 0) {
					if (verb < 20)
						sysmess(7);
					else
						sysmess(8);
					nl();
				}
			} else
				sysmess(6);	/* Unknown verb */
		}
	}
}

static uchr cplscmp(ushrt first, char *snd)
{
	if (((255 - (zmem(first++))) & 0x7F) != (snd[0]))
		return 0;
	if (((255 - (zmem(first++))) & 0x7F) != (snd[1]))
		return 0;
	if (((255 - (zmem(first++))) & 0x7F) != (snd[2]))
		return 0;
	if (((255 - (zmem(first++))) & 0x7F) != (snd[3]))
		return 0;
	return 1;
}

static uchr matchword(char **wordbeg)
{
/* Match a word of player input with the vocabulary table */

	ushrt matchp = vocab;
	char wordbuf[5];
	int i;

	wordbuf[4] = 0;
	while (1) {
		for (i = 0; i < 4; i++) {
			wordbuf[i] = (**wordbeg);
			if (islower(wordbuf[i]))
				wordbuf[i] = toupper(wordbuf[i]);	/* (v0.4.1), was munging numbers */
			if (wordbuf[i] == 0)
				wordbuf[i] = ' ';
			if (wordbuf[i] == '\n')
				wordbuf[i] = ' ';
			if (wordbuf[i] == '\r')
				wordbuf[i] = ' ';
			if (wordbuf[i] == ' ')
				(*wordbeg)--;
			(*wordbeg)++;
		}
		while ((**wordbeg)
		       && (**wordbeg != '\n')
		       && (**wordbeg != '\r')
		       && (**wordbeg != ' '))
			(*wordbeg)++;
		while (zmem(matchp)) {
			if (cplscmp(matchp, wordbuf)) {
				return zmem(matchp + 4);
			}
			matchp += 5;
		}
		matchp = vocab;

		if (((**wordbeg) == 0)
		    || ((**wordbeg) == '\r')
		    || ((**wordbeg) == '\n'))
			return 0xFF;

		while ((**wordbeg) == 0x20) {
			if ((**wordbeg) == 0)
				return 0xFF;
			(*wordbeg)++;
		}
	}
	return 0xFF;

}

static void listat(uchr locno)
{
/* List items at location n */

	uchr any = 0, n;

	for (n = 0; n < nobj; n++)
		if (state.objpos[n] == locno) {
			if (any == 0) {
				any = 1;
				if (locno < 253) {
					sysmess(alsosee);
					nl();
				}
			}
			oneitem(objtab, n);
			nl();
		}
}

static uchr ffeq(uchr x, uchr y)
{				/* Match x with y, 0FFh matches any */
	return (uchr) (x == 0xFF || y == 0xFF || x == y);
}

static uchr doproc(ushrt table, uchr verb, uchr noun)
{
	ushrt ccond;
	uchr done = 0;		/* Done returns: 0 for fell off end of table
				   1 for DONE
				   2 for DESC
				   3 for END (end game) */
	uchr t, tverb, tnoun, td1 = 0;

	while ((zmem(table)) && !done) {
		tverb = zmem(table++);
		tnoun = zmem(table++);
		ccond = zword(table++);
		table++;

		if (ffeq(verb, tverb) && ffeq(noun, tnoun)) {
			t = condtrue(ccond);
			/* Skip over condition clauses */
			/* FIXME: check if condskip change in unquill 0.9
			   is valid ?? */
			while (zmem(ccond++) != 0xFF) ;
			if (t) {
				done = runact(ccond, noun);
				/* Returned nonzero if should not scan */
				td1 = 1;	/* Something was run */
			}
		}
	}
	if (done == 0 && td1)
		done = 1;
	return done;
}

static uchr autobj(uchr noun)
{				/* Find object number for AUTOx actions */
	uchr n;

	if (dbver == 0)
		return 0xFF;
	if (noun > 253)
		return 0xFF;
	for (n = 0; n < nobj; n++)
		if (noun == zmem(objmap + n))
			return n;
	return 0xFF;
}

static char yesno(void)
{
	char n;

	while (1) {
		n = getchar();
		if ((n == 'Y') || (n == 'y'))
			return ('Y');
		if ((n == 'N') || (n == 'n'))
			return ('N');
	}
	return ('N');
}

static void sysmess(uchr sysno)
{
	uchr cch = 0;
	ushrt msgadd;

	if (dbver > 0) {
		oneitem(sysbase, sysno);
		return;
	}
	msgadd = sysbase;
/*	fprintf(stderr, "sysmsg %d - base %u\n", sysno, sysbase); */
	while (sysno) {		/* Skip (sysno) messages */
		while (cch != 0x1F)
			cch = 0xFF - (zmem(msgadd++));
		sysno--;
		cch = 0xFF - (zmem(msgadd++));
	}
	msgadd--;
	while (cch != 0x1F) {
		cch = 0xFF - (zmem(msgadd++));
		expch(cch, &msgadd);
	}
}

static void oneitem(ushrt table, uchr item)
{
	ushrt n;
	uchr cch;
	uchr term;

	if (arch == ARCH_SPECTRUM)
		term = 0x1F;
	else
		term = 0;
	cch = ~term;

	n = zword(table + 2 * item);

	while (1) {
		cch = (0xFF - (zmem(n++)));
		if (cch == term)
			break;
		expch(cch, &n);
	}
}

static void expdict(uchr cch, ushrt * n)
{
	ushrt d = dict;

	if (dbver > 0) {	/* Early games aren't compressed & have no dictionary */
		cch -= 164;
		while (cch)
			if (zmem(d++) > 0x7f)
				cch--;

		/* d=address of expansion text */

		while (zmem(d) < 0x80)
			expch(zmem(d++) & 0x7F, n);
		expch(zmem(d) & 0x7F, n);
	}
}

static void expch_c64(uchr cch, ushrt * n)
{
	if (cch >= 'A' && cch <= 'Z')
		cch = tolower(cch);
	cch &= 0x7F;

	if ((cch > 31) && (cch < 127))
		opch32(cch);
	else if (cch > 126)
		opch32('?');
	else if (cch == 8)
		opch32(8);
	else if (cch == 0x0D)
		nl();
}

static void expch_cpc(uchr cch, ushrt * n)
{
	if ((cch > 31) && (cch < 127))
		opch32(cch);
	/*      else if (cch > 164)  expdict(cch, n); */
	else if (cch > 126)
		opch32('?');
	else if (cch == 9)
		return;
	else if (cch == 0x0D)
		nl();
	else if (cch == 0x14)
		nl();
	else if (cch < 31)
		opch32('?');
}

static void expch(uchr cch, ushrt * n)
{
	ushrt tbsp;

	if (arch == ARCH_CPC) {
		expch_cpc(cch, n);
		return;
	}
	if (arch == ARCH_C64) {
		expch_c64(cch, n);
		return;
	}

	if ((cch > 31) && (cch < 127))
		opch32(cch);
	else if (cch > 164)
		expdict(cch, n);
	else if (cch > 126)
		opch32(cch);
	else if (cch == 6)
		for (spc(); (xpos % 16); spc()) ;
	else if (cch == 8)
		opch32(8);
	else if (cch == 0x0D)
		nl();
	else if (cch == 0x17) {
		tbsp = (255 - zmem((*n)++)) & 0x1F;
		++(*n);
		if (xpos > tbsp)
			nl();
		for (; tbsp > 0; tbsp--)
			spc();
	} else if ((cch > 0x0F) && (cch < 0x16))
		(*n)++;
}


static uchr ink = 7;
static uchr paper = 0;
static uchr inverse = 0;
static uchr ypos;	/* Not yet used */
static uint8_t lcolour = 0x70;
static char colbuf[6] = "\033aX\033bX";

static struct termios saved_ts;
static uchr ts_valid;

static uchr udg;

static void termexit(int sig)
{
	if (ts_valid)
		tcsetattr(0, TCSANOW, &saved_ts);
	_exit(0);
}

static void termclean(void)
{
	if (ts_valid)
		tcsetattr(0, TCSANOW, &saved_ts);
}

#ifndef __linux__

static struct fontinfo fs;

static unsigned load_tile(unsigned n)
{
	unsigned r = 4;	/* Rows per block */
	uchr ch[17];
	uchr *p;
	uchr i, h, l;

	switch(fs.format) {
	case FONT_INFO_8X16:
		n = 8;
		/* Fall through */
	case FONT_INFO_8X8:
		h = (n & 1) ? 0x0F: 0x00;
		h |= (n & 2) ? 0xF0: 0x00;
		l = (n & 4) ? 0x0F : 0x00;
		l |= (n & 8) ? 0xF0 : 0x00;
		break;
	case FONT_INFO_6X12P16:
		n = 6;
		/* Fall through */
	case FONT_INFO_6X8:
		h = (n & 1) ? 0x07: 0x00;
		h |= (n & 2) ? 0x38: 0x00;
		l = (n & 4) ? 0x07 : 0x00;
		l |= (n & 8) ? 0x38 : 0x00;
		break;
	case FONT_INFO_4X6:
		r = 3;
		/* Fall through */
	case FONT_INFO_4X8:
		h = (n & 1) ? 0x33: 0x00;
		h |= (n & 2) ? 0xCC: 0x0F;
		l = (n & 4) ? 0x33 : 0x00;
		l |= (n & 8) ? 0xCC : 0x00;
		break;
	default:
		/* We don't know how to make block chars in weird forms */
		return 0;
	}
	p = ch;
	*p++ = n + fs.udg_low;
	for (i = 0; i < n; i++)
		*p++ = h;
	for (i = 0; i < n; i++)
		*p++ = l;
	if(ioctl(0, VTSETUDG, &ch))
		return 0;
	return 1;
}

static unsigned load_user(unsigned n)
{
	unsigned r = 4;	/* Rows per block */
	uchr ch[17];
	uchr *p = ch;
	uchr i, tmp;
	uint16_t ptr = zword(23675);

	*p++ = n + fs.udg_low + 0x10;
	for (i = 0; i < 8; i++) {
		uchr bits = zmem(ptr++);
		
		switch(fs.format) {
		case FONT_INFO_8X16:
			*p++ = bits;
		case FONT_INFO_8X8:
			*p++ = bits;
			break;
		case FONT_INFO_6X8:
			*p++ = ((bits & 0x70) >> 2) | ((bits & 0x0E) >> 1);
			break;
		case FONT_INFO_6X12P16:
			if (i & 1)
				*p++ = ((bits & 0x70) >> 2) | ((bits & 0x0E) >> 1);
			*p++ = ((bits & 0x70) >> 2) | ((bits & 0x0E) >> 1);
			break;
		case FONT_INFO_4X6:
			if (i == 0 && i == 4)
				break;
		case FONT_INFO_4X8:
			tmp = (bits & 0x03) ? 0x11 : 0x00;
			tmp |= (bits & 0x0C) ? 0x22 : 0x00;
			tmp |= (bits & 0x30) ? 0x44 : 0x00;
			tmp |= (bits & 0xC0) ? 0x88 : 0x00;
			*p++ = tmp;
			break;
		default:
			return 1;
		}
	}
	if(ioctl(0, VTSETUDG, &ch))
		return 1;
	return 2;
}

static unsigned load_udg(void)
{
	unsigned i = fs.udg_low;
	unsigned n;

	if (fs.udg_high - fs.udg_low < 16)
		return 0;
	for (n = 0; n < 16; n++) {
		if (load_tile(n) == 0)
			return 0;
	}
	/* Now can we fit in user UDG ? */
	if (fs.udg_high - fs.udg_low <= 0x25)
		return 1;
	for (n = 0; n < 25; n++) {
		if (load_user(n) == 0)
			return 1;
	}
	return 2;
}

#endif
		
static void terminit(void)
{
	struct termios ts;
	if (tcgetattr(0, &ts)) {
		memcpy(&saved_ts, &ts, sizeof(ts));
		ts_valid = 1;
		signal(SIGINT, termexit);
		signal(SIGPIPE, termexit);
		signal(SIGQUIT, termexit);
		atexit(termclean);
		cfmakeraw(&ts);
		tcsetattr(0, TCSANOW, &ts);
#ifndef __linux__
		if (ioctl(0, VTSIZE, 0) != -1)
			console = 1;
		if (ioctl(0, VTGETUDG, &fs) == 0)
			udg = load_udg();
#endif
	}
}

static void drawlocation(void)
{
}

static void initdisplay(void)
{
	terminit();
}

static void clrscr(void)
{
	if (console) {
		write(1, "\033H\033J", 4);
		xpos = 0;
		ypos = 0;
	}
	/* TODO termcap */
}

static void setcolour(void)
{
	uint8_t ncol;
	if (inverse)
		ncol = (paper << 4) | ink;
	else
		ncol = (ink << 4) | paper;
	if (ncol != lcolour) {
		colbuf[2] = ink | 0x40;
		colbuf[5] = paper | 0x40;
		write(1, colbuf, 6);
		lcolour = ncol;
	}
}

static void opch32(char ch)
{
	static uchr state;	/* Video render state */
	if (console) {
		switch(state) {
		case 1:
			state = 0;
			ink = ch & 7;
			return;
		case 2:
			state = 0;
			paper = ch & 7;
			return;
		case 3:
			state = 0;
			return;
		case 4:
			inverse = ch & 1;
			state = 0;
			return;
		}
			
		/* Interpret ZX codes in full */
		switch(ch) {
		case 8:
			if (xpos)
				xpos--;
			break;
		case 16:	/* ink */
			state = 1;
			break;
		case 17:	/* paper */
			state = 2;
			break;
		case 18:	/* flash - ignore */
			state = 3;
			break;
		case 19:	/* bright - ignore */
			state = 3;
			break;
		case 20:	/* inverse */
			state = 4;
			break;
		case 21:	/* over - ignore */
			state = 3;
			break;
		case 9:
			ch = ' ';	/* Really cursor right */
		default:
			if (ch >= 0x7F && !udg)
				ch = ' ';
			if (ch >= 0x90 && udg != 2)
				ch = '#';
			write(1, &ch, 1);
			if (arch == ARCH_SPECTRUM && xpos == 31)
				nl();
			else if (arch != ARCH_SPECTRUM && xpos == 39)
				nl();
			else
				xpos++;
		/* TOOD: A-C, 22, 23 */
		}
		return;
	}
	if (ch > 126)
		ch = '?';	/* No UDGs */
	write(1, &ch, 1);
	if (ch == '\n')
		xpos = 0;
	else if (ch == 8 && xpos)
		xpos--;
	else if (arch == ARCH_SPECTRUM && xpos == 31)
		nl();
	else if (arch != ARCH_SPECTRUM && xpos == 39)
		nl();
	else
		xpos++;
}
