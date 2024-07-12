/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)archive.h	8.1 (Berkeley) 6/6/93
 */

#include <sys/stat.h>

#define	AR_EFMT1	"#1/"		/* extended format #1 */

/* Ar(1) options. */
#define	AR_A	0x0001
#define	AR_B	0x0002
#define	AR_C	0x0004
#define	AR_D	0x0008
#define	AR_M	0x0010
#define	AR_O	0x0020
#define	AR_P	0x0040
#define	AR_Q	0x0080
#define	AR_R	0x0100
#define	AR_T	0x0200
#define	AR_TR	0x0400
#define	AR_U	0x0800
#define	AR_V	0x1000
#define	AR_X	0x2000
extern u_int options;

/* Set up file copy. */
#define	SETCF(from, fromname, to, toname, pad) { \
	cf.rfd = from; \
	cf.rname = fromname; \
	cf.wfd = to; \
	cf.wname = toname; \
	cf.flags = pad; \
}

/* File copy structure. */
typedef struct {
	int rfd;			/* read file descriptor */
	char *rname;			/* read name */
	int wfd;			/* write file descriptor */
	char *wname;			/* write name */
#define	NOPAD	0x00			/* don't pad */
#define	RPAD	0x01			/* pad on reads */
#define	WPAD	0x02			/* pad on writes */
	u_int flags;			/* pad flags */
} CF;

/* Header structure internal format. */
typedef struct {
	off_t size;			/* size of the object in bytes */
	time_t date;			/* date */
	int lname;			/* size of the long name in bytes */
	int gid;			/* group */
	int uid;			/* owner */
	u_short mode;			/* permissions */
	char name[MAXNAMLEN + 1];	/* name */
} CHDR;

/* Header format strings. */
#define	HDR1	"%s%-13d%-12ld%-6u%-6u%-8o%-10ld%2s"
#define	HDR2	"%-16.16s%-12ld%-6u%-6u%-8o%-10ld%2s"

#define	OLDARMAXNAME	15
#define	HDR3	"%-16.15s%-12ld%-6u%-6u%-8o%-10ld%2s"


#include <sys/cdefs.h>

int append(char **argv);
int main(int argc, char **argv);
int open_archive(int mode);
void close_archive(int fd);
int get_arobj(int fd);
void put_arobj(CF *cfp, struct stat *sb);
void copy_ar(CF *cfp, off_t size);
void skip_arobj(int fd);
int contents(char **argv);
int delete(char **argv);
int extract(char **argv);
int tmp(void);
char *files(char **argv);
void orphans(char **argv);
char *rname(char *path);
int compare(char *dest);
void badfmt(void);
void error(char *name);
int move(char **argv);
int print(char **argv);
int replace(char **argv);
void strmode(register mode_t mode, char *p);
