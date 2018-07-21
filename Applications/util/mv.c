/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * stdio stripped out Alan Cox 2015
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <utime.h>
#include <errno.h>

typedef unsigned char BOOL;

#define FALSE 0
#define TRUE  1

#define PATHLEN  512

        
BOOL intflag;

/* Don't bother with malloc. We don't want to blow up with no space errors */
/* Don't go over 16384 until we fix the pipe bug */
#define BUF_SIZE 16384
static char buffer[BUF_SIZE];

static void writes(int fd, const char *p)
{
    write(fd, p, strlen(p));
}

/*
 * Return TRUE if a filename is a directory.
 * Nonexistant files return FALSE.
 */
BOOL isadir(char *name)
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
BOOL copyfile(char *srcname, char *destname, BOOL setmodes)
{
    int rfd;
    int wfd;
    int rcc;
    int wcc;
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

    while ((rcc = read(rfd, buffer, BUF_SIZE)) > 0) {
	bp = buffer;
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
    static char buf[BUFSIZ];

    if (dirname == NULL || *dirname == '\0')
	return filename;

    cp = strrchr(filename, '/');
    if (cp)
	filename = cp + 1;

    /* We cannot share the buffer used by copying alas*/
    strlcpy(buf, dirname, BUFSIZ);
    strlcat(buf, "/", BUFSIZ);
    strlcat(buf, filename, BUFSIZ);

    return buf;
}


int main(int argc, char *argv[])
{
    int  dirflag;
    char *srcname;
    char *destname;
    char *lastarg;

    lastarg = argv[argc - 1];

    dirflag = isadir(lastarg);

    if ((argc > 3) && !dirflag) {
        writes(2, lastarg);
        writes(2, ": not a directory\n");
	return 1;
    }

    while (argc-- > 2) {
	srcname = *(++argv);
	if (access(srcname, 0) < 0) {
	    perror(srcname);
	    continue;
	}

	destname = lastarg;
	if (dirflag)
	    destname = buildname(destname, srcname);

	if (rename(srcname, destname) >= 0)
	    continue;

	if (errno != EXDEV) {
	    perror(destname);
	    continue;
	}

	if (!copyfile(srcname, destname, TRUE))
	    continue;

	if (unlink(srcname) < 0)
	    perror(srcname);
    }

    return 0;
}
