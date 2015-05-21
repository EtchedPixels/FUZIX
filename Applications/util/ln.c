/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Stripped of stdio usage Alan Cox 2015
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifndef BOOL
#define BOOL  int
#define TRUE  1
#define FALSE 0
#endif

#define PATHLEN 512

static void writes(int fd, const char *s)
{
    write(fd, s, strlen(s));
}

/*
 * Return TRUE if name is a directory.
 * Return FALSE otherwise or if the file does not exist.
 */

BOOL isadir(char *name)
{
    struct stat statbuf;

    if (stat(name, &statbuf) < 0)
	return FALSE;

    return S_ISDIR(statbuf.st_mode);
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

    if (dirname == NULL || *dirname == '\0')
	return filename;

    cp = strrchr(filename, '/');
    if (cp)
	filename = cp + 1;

    strlcpy(buf, dirname, PATHLEN);
    strlcat(buf, "/", PATHLEN);
    strlcat(buf, filename, PATHLEN);

    return buf;
}


int main(int argc, char *argv[])
{
    int  dirflag;
    char *srcname, *destname, *lastarg;

    /* Symbolic link? */

    if (argv[1][0] == '-') {
#if defined(S_ISLNK) && 0	/* FIXME */
	if (strcmp(argv[1], "-s") == 0) {
	    if (argc != 4) {
		writes(2, argv[0]);
		writes(2, ": wrong number of arguments for symbolic link\n");
		return 1;
	    }

	    if (symlink(argv[2], argv[3]) < 0) {
		perror(argv[3]);
		return 1;
	    }
	    return 0;

	}
#endif
	writes(2, argv[0]);
	writes(2, ": unknown option ");
	writes(2, argv[1]);
	write(2, "\n", 1);
	return 1;
    }

    /* Here for normal hard links. */

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

	if (link(srcname, destname) < 0) {
	    perror(destname);
	    continue;
	}
    }

    return 0;
}
