/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */

#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    char *cp;
    int uid;
    struct passwd *pwd;
    struct stat statbuf;

    cp = argv[1];
    if (!cp) {
	fprintf(stderr, "chown: too few arguments\n");
	return 1;
    }
    if (isdigit(*cp)) {
	uid = 0;
	while (isdigit(*cp))
	    uid = uid * 10 + (*cp++ - '0');

	if (*cp) {
	    fprintf(stderr, "chown: bad uid value\n");
	    return 1;
	}
    } else {
	pwd = getpwnam(cp);
	if (pwd == NULL) {
	    fprintf(stderr, "chown: unknown user name\n");
	    return 1;
	}
	uid = pwd->pw_uid;
    }

    argc--;
    argv++;

    while (argc-- > 1) {
	argv++;
	if ((stat(*argv, &statbuf) < 0) ||
	    (chown(*argv, uid, statbuf.st_gid) < 0)) {
	    perror(*argv);
	    return 1;
	}
    }

    return 0;
}
