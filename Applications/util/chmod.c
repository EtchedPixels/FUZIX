/* chmod - Change file modes				Author: V. Archer */

/* Copyright 1991 by Vincent Archer
 *	You may freely redistribute this software, in source or binary
 *	form, provided that you do not alter this copyright mention in any
 *	way.
 *
 * Stdio usage removed Alan Cox 2015
 */

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef S_ISLNK
#define S_ISLNK(mode)	0
#define lstat		stat
#endif

#define USR_MODES (S_ISUID|S_IRWXU)
#define GRP_MODES (S_ISGID|S_IRWXG)
#define EXE_MODES (S_IXUSR|S_IXGRP|S_IXOTH)
#ifdef S_ISVTX
#define ALL_MODES (USR_MODES|GRP_MODES|S_IRWXO|S_ISVTX)
#else
#define ALL_MODES (USR_MODES|GRP_MODES|S_IRWXO)
#endif

#define PATH_MAX	512	/* FIXME: it's a sysconf shouldn't be
                                   static here */
/* Common variables */
char *symbolic;
mode_t new_mode, u_mask;
int  rflag, errors;
struct stat st;
char path[PATH_MAX + 1];

int main(int argc, char *argv[]);
mode_t parsemode(char *symbolic, mode_t oldmode);
int do_change(char *name);
void usage(void);

/* Parse a P1003.2 4.7.7-conformant symbolic mode. */
mode_t parsemode(char *symbolicp, mode_t oldmode)
{
    register char *symbolic = symbolicp;
    register mode_t who, mask, newmode, tmpmask;
    char action;

    newmode = oldmode & ALL_MODES;
    while (*symbolic) {
	who = 0;
	for (; *symbolic; symbolic++) {
	    if (*symbolic == 'a') {
		who |= ALL_MODES;
		continue;
	    }
	    if (*symbolic == 'u') {
		who |= USR_MODES;
		continue;
	    }
	    if (*symbolic == 'g') {
		who |= GRP_MODES;
		continue;
	    }
	    if (*symbolic == 'o') {
		who |= S_IRWXO;
		continue;
	    }
	    break;
	}
	if (!*symbolic || *symbolic == ',')
	    usage();
	while (*symbolic) {
	    if (*symbolic == ',')
		break;
	    switch (*symbolic) {
		default: usage();
		case '+':
		case '-':
		case '=': action = *symbolic++; break;
	    }
	    mask = 0;
	    for (; *symbolic; symbolic++) {
		if (*symbolic == 'u') {
		    tmpmask = newmode & S_IRWXU;
		    mask |= tmpmask | (tmpmask << 3) | (tmpmask << 6);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'g') {
		    tmpmask = newmode & S_IRWXG;
		    mask |= tmpmask | (tmpmask >> 3) | (tmpmask << 3);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'o') {
		    tmpmask = newmode & S_IRWXO;
		    mask |= tmpmask | (tmpmask >> 3) | (tmpmask >> 6);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'r') {
		    mask |= S_IRUSR | S_IRGRP | S_IROTH;
		    continue;
		}
		if (*symbolic == 'w') {
		    mask |= S_IWUSR | S_IWGRP | S_IWOTH;
		    continue;
		}
		if (*symbolic == 'x') {
		    mask |= EXE_MODES;
		    continue;
		}
		if (*symbolic == 's') {
		    mask |= S_ISUID | S_ISGID;
		    continue;
		}
		if (*symbolic == 'X') {
		    if (S_ISDIR(oldmode) || (oldmode & EXE_MODES))
			mask |= EXE_MODES;
		    continue;
		}
#ifdef S_ISVTX
		if (*symbolic == 't') {
		    mask |= S_ISVTX;
		    who |= S_ISVTX;
		    continue;
		}
#endif
		break;
	    }
	    switch (action) {
	    case '=':
		if (who)
		    newmode &= ~who;
		else
		    newmode = 0;
	    case '+':
		if (who)
		    newmode |= who & mask;
		else
		    newmode |= mask & (~u_mask);
		break;
	    case '-':
		if (who)
		    newmode &= ~(who & mask);
		else
		    newmode &= ~mask | u_mask;
	    }
	}
	if (*symbolic)
	    symbolic++;
    }
    return (newmode);
}


/*
 *  Main module. The single option possible (-R) does not warrant a call to
 *  the getopt() stuff.
 */

int main(int argc, char *argv[])
{
    int ex_code = 0;

    argc--;
    argv++;

    if (argc && strcmp(*argv, "-R") == 0) {
	argc--;
	argv++;
	rflag = 1;
    } else {
	rflag = 0;
    }

    if (!argc--) usage();

    if (!strcmp(argv[0], "--")) {	/* Allow chmod -- -r, as in Draft11 example */
	if (!argc--) usage();
	argv++;
    }

    symbolic = *argv++;
    if (!argc) usage();

    if (*symbolic >= '0' && *symbolic <= '7') {
	new_mode = 0;
	while (*symbolic >= '0' && *symbolic <= '7')
	    new_mode = (new_mode << 3) | (*symbolic++ & 07);
	if (*symbolic) usage();
	new_mode &= ALL_MODES;
	symbolic = (char *) 0;
    } else {
	u_mask = umask(0);
    }

    while (argc--)
	if (do_change(*argv++))
	    ex_code = 1;

    return (ex_code);
}


/* Apply a mode change to a given file system element. */
int do_change(char *name)
{
    mode_t m;
    DIR *dir;
    register struct dirent *entp;
    register char *namp;

    if (lstat(name, &st)) {
	perror(name);
	return 1;
    }

    if (S_ISLNK(st.st_mode) && rflag)
	return 0;		/* Note: violates POSIX. */

    if (!symbolic)
	m = new_mode;
    else
	m = parsemode(symbolic, st.st_mode);

    if (chmod(name, m)) {
	perror(name);
	errors = 1;
    } else {
	errors = 0;
    }

    if (S_ISDIR(st.st_mode) && rflag) {
	if ((dir = opendir(name)) == NULL) {
	    perror(name);
	    return (1);
	}
	if (name != path)
	    strcpy(path, name);
	namp = path + strlen(path);
	*namp++ = '/';
	while ((entp = readdir(dir)) != NULL) {
	    if (entp->d_name[0] != '.' ||
		(entp->d_name[1] &&
		 (entp->d_name[1] != '.' || entp->d_name[2]))) {
		strcpy(namp, entp->d_name);
		errors |= do_change(path);
	    }
	}
	closedir(dir);
	*--namp = '\0';
    }
    return (errors);
}


/* Display Posix prototype */
void usage(void)
{
    write(2, "Usage: chmod [-R] mode file...\n",31);
    exit(1);
}
