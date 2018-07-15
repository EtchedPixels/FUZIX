/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Stripped of stdio use Alan Cox 2015
 * Set up to use bigger buffers.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

typedef unsigned char BOOL;

#define FALSE 0
#define TRUE  1

#define PATHLEN  512

BOOL intflag;

BOOL isadir(char *);
BOOL copyfile(char *, char *, BOOL);

char *buildname(char *, char *);

static void writes(int fd, const char *p)
{
  write(fd, p, strlen(p));
}

int main(int argc, char *argv[])
{
    BOOL dirflag;
    char *srcname;
    char *destname;
    char *lastarg;

    lastarg = argv[argc - 1];

    dirflag = isadir(lastarg);

    if (argc > 3 && !dirflag) {
        writes(2, lastarg);
        writes(2, ": not a directory\n");
	return 1;
    }
    if (argc < 3) {
        writes(2, "cp: destination required\n");
        return 1;
    }
    while (argc-- > 2) {
        srcname = argv[1];
	destname = lastarg;
	if (dirflag)
	    destname = buildname(destname, srcname);

	copyfile(*++argv, destname, FALSE);
    }
    return 0;
}
/*
 * Return TRUE if a filename is a directory.
 * Nonexistant files return FALSE.
 */
BOOL
isadir(char *name)
{
    struct stat statbuf;

    if (stat(name, &statbuf) < 0)
	return FALSE;

    return S_ISDIR(statbuf.st_mode);
}

/*
 * Copy one file to another, while possibly preserving its modes, times,
 * and modes.  Returns TRUE if successful, or FALSE on a failure with an
 * error message output.  (Failure is not indicted if the attributes cannot
 * be set.)
 */

/* We can't bump this past 16384 right now until we fix issue #291 */
/* It's also a nice safe number because it'll fit our smallest platforms */
#define BUF_SIZE 16384

static char buf[BUF_SIZE];

BOOL copyfile(char *srcname, char *destname, BOOL setmodes)
{
    int  rfd, wfd, rcc, wcc;
    char *bp;
    struct stat statbuf1;
    struct stat statbuf2;
    struct utimbuf times;

    if (stat(srcname, &statbuf1) < 0) {
	perror(srcname);
	return FALSE;
    }
    if (stat(destname, &statbuf2) < 0) {
	statbuf2.st_ino = -1;
	statbuf2.st_dev = -1;
    }
    if ((statbuf1.st_dev == statbuf2.st_dev) &&
	(statbuf1.st_ino == statbuf2.st_ino)) {
	writes(2, "Copying file \"");
	writes(2, srcname);
	writes(2, "\" to itself\n");
	return FALSE;
    }
    rfd = open(srcname, 0);
    if (rfd < 0) {
	perror(srcname);
	return FALSE;
    }
    wfd = creat(destname, statbuf1.st_mode);
    if (wfd < 0) {
	perror(destname);
	close(rfd);
	return FALSE;
    }
    while ((rcc = read(rfd, buf, BUF_SIZE)) > 0) {
	if (intflag) {
	    close(rfd);
	    close(wfd);
	    return FALSE;
	}
	bp = buf;
	while (rcc > 0) {
	    wcc = write(wfd, bp, rcc);
	    if (wcc < 0) {
		perror(destname);
		goto error_exit;
	    }
	    bp += wcc;
	    rcc -= wcc;
	}
    }

    if (rcc < 0) {
	perror(srcname);
	goto error_exit;
    }
    close(rfd);
    if (close(wfd) < 0) {
	perror(destname);
	return FALSE;
    }
    if (setmodes) {
	chmod(destname, statbuf1.st_mode);

	chown(destname, statbuf1.st_uid, statbuf1.st_gid);

	times.actime = statbuf1.st_atime;
	times.modtime = statbuf1.st_mtime;

	utime(destname, &times);
    }
    return TRUE;


  error_exit:
    close(rfd);
    close(wfd);

    return FALSE;
}

/*
 * Build a path name from the specified directory name and file name.
 * If the directory name is NULL, then the original filename is returned.
 * The built path is in a static area, and is overwritten for each call.
 */
char *buildname(char *dirname, char *filename)
{
    char *cp;
    static char buf[PATHLEN];

    if ((dirname == NULL) || (*dirname == '\0'))
	return filename;

    cp = strrchr(filename, '/');
    if (cp)
	filename = cp + 1;

    strlcpy(buf, dirname, PATHLEN);
    strlcat(buf, "/", PATHLEN);
    strlcat(buf, filename, PATHLEN);

    return buf;
}
