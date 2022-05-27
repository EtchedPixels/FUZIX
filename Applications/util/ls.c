/* The "ls" standard command.
 *
 * FIXME: we should chdir into the directory we are listing if doing stats
 * and then stat the shortname (this saves lots of filename walks and the
 * resulting touches of atime).
 * Maybe use allocated buffers instead of the stack for recursion (safer).
 * Shall detect infinite loops, in resursive mode.
 *
 * changelog:
 *  2016-08-31 DF: added -R (recursive), now working -r (reverse order), cleanup
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include <pwd.h>
#include <grp.h>
#include <dirent.h>

#ifndef UIDGID
#define UIDGID		1 /* set to zero to disable UID/GID */
#endif

#define LISTSIZE	256
#define PATHLEN		512
#ifdef	S_ISLNK
#define LSTAT		lstat
#else
#define LSTAT		stat
#endif

/*
 * Flags for the LS command.
 */
#define LSF_LONG	0x001
#define LSF_DIR 	0x002
#define LSF_INODE	0x004
#define LSF_FILE	0x008
#define LSF_RECUR	0x010
#define LSF_MULT	0x020
#define LSF_ALL		0x040
#define LSF_OCT		0x080
#define LSF_DOWN	0x100
#define LSF_UNSORT	0x200

typedef unsigned char BOOL;

static int flags;
static char prevtype;
static uint8_t bad; /* Set if any errors occur on the way */

static void lsfile(char *, struct stat *, int);
static void listfiles(char *name);
static char *modestring(int mode);
static char *timestring(time_t *t);
static void printsep(char *name);
static void printusage(FILE *out);
static int namesort(const void *p1, const void *p2);
static int revnamesort(const void *p1, const void *p2);

/*
 * Sort routines for list of filenames.
 */
static int namesort(const void *p1, const void *p2)
{
	return strcmp(* (char * const *)p1, * (char * const *) p2);
}
static int revnamesort(const void *p1, const void *p2)
{
	return -strcmp(* (char * const *)p1, * (char * const *) p2);
}

/*
 * Return the standard ls-like mode string from a file mode.
 * This is static and so is overwritten on each call.
 */
static char *modestring(int mode)
{
	static char buf[12];

	if (flags & LSF_OCT) {
		sprintf(buf, "%06o", mode);
		return buf;
	}
	strcpy(buf, "----------");

	/* Fill in the file type. */
	if (S_ISDIR(mode))
		buf[0] = 'd';
	if (S_ISCHR(mode))
		buf[0] = 'c';
	if (S_ISBLK(mode))
		buf[0] = 'b';
	if (S_ISFIFO(mode))
		buf[0] = 'p';
#ifdef	S_ISLNK
	if (S_ISLNK(mode))
		buf[0] = 'l';
#endif
	/* Now fill in the normal file permissions. */
	if (mode & S_IRUSR)
		buf[1] = 'r';
	if (mode & S_IWUSR)
		buf[2] = 'w';
	if (mode & S_IXUSR)
		buf[3] = 'x';

	if (mode & S_IRGRP)
		buf[4] = 'r';
	if (mode & S_IWGRP)
		buf[5] = 'w';
	if (mode & S_IXGRP)
		buf[6] = 'x';

	if (mode & S_IROTH)
		buf[7] = 'r';
	if (mode & S_IWOTH)
		buf[8] = 'w';
	if (mode & S_IXOTH)
		buf[9] = 'x';

	/* Finally fill in magic stuff like suid and sticky text. */
	if (mode & S_ISUID)
		buf[3] = ((mode & S_IXUSR) ? 's' : 'S');
	if (mode & S_ISGID)
		buf[6] = ((mode & S_IXGRP) ? 's' : 'S');
	if (mode & S_ISVTX)
		buf[9] = ((mode & S_IXOTH) ? 't' : 'T');
	return buf;
}

/*
 * Get the time to be used for a file.
 * This is down to the minute for new files, but only the date for old files.
 * The string is returned from a static buffer, and so is overwritten for
 * each call.
 */
static char *timestring(time_t *t)
{
	char *str;
	static char buf[26];

	str = ctime(t);
	strcpy(buf, &str[4]);
	buf[12] = '\0';
	return buf;
}

/*
 * Print a separator.
 */
static void printsep(char *name)
{
	/* type:
	 *   1 when a directory listing
	 *   2 when a standalone file
	 */
	char type = !name+1;
	if (prevtype && (type==1 || prevtype==1))
		fputc('\n', stdout);
	if (type == 1)
		printf("%s:\n", name);
	prevtype = type;
}

/*
 * Do an LS of a particular file name according to the flags.
 */
static void lsfile(char *name, struct stat *statbuf, int flags)
{
#if	UIDGID
	static char username[12];
	static int userid;
	static BOOL useridknown;
	static char groupname[12];
	static int groupid;
	static BOOL groupidknown;
	struct passwd *pwd;
	struct group *grp;
#endif
	static char buf[PATHLEN];
	char *cp = buf;
	int len;

	*cp = '\0';
	if (flags & LSF_INODE) {
		sprintf(cp, "%5d ", statbuf->st_ino);
		cp += strlen(cp);
	}
	if (flags & LSF_LONG) {
		cp += strlen(strcpy(cp, modestring(statbuf->st_mode)));
		sprintf(cp, "%4d ", statbuf->st_nlink);
		cp += strlen(cp);
#if	UIDGID
		if (!useridknown || (statbuf->st_uid != userid)) {
			if ((pwd = getpwuid(statbuf->st_uid)) != NULL)
				strcpy(username, pwd->pw_name);
			else
				sprintf(username, "%d", statbuf->st_uid);
			userid = statbuf->st_uid;
			useridknown = 1;
		}
		sprintf(cp, "%-8s ", username);
		cp += strlen(cp);
		if (!groupidknown || (statbuf->st_gid != groupid)) {
			if ((grp = getgrgid(statbuf->st_gid)) != NULL)
				strcpy(groupname, grp->gr_name);
			else
				sprintf(groupname, "%d", statbuf->st_gid);
			groupid = statbuf->st_gid;
			groupidknown = 1;
		}
		sprintf(cp, "%-8s ", groupname);
		cp += strlen(cp);
#endif
		if (S_ISDEV(statbuf->st_mode))
			sprintf(cp, "%3d,%-3d  ",
				statbuf->st_rdev >> 8, statbuf->st_rdev & 0xFF);
		else
			sprintf(cp, "%8ld ", (long) statbuf->st_size);
		cp += strlen(cp);
		sprintf(cp, "%-12s ", timestring(&statbuf->st_mtime));
	}
	fputs(buf, stdout);
	fputs(name, stdout);
#ifdef	S_ISLNK
	if ((flags & LSF_LONG) && S_ISLNK(statbuf->st_mode)) {
		if ((len = readlink(name, buf, PATHLEN - 1)) >= 0) {
			buf[len] = '\0';
			printf(" -> %s", buf);
		}
	}
#endif
	fputc('\n', stdout);
}

/*
 * List the content of a directory or list a file.
 */
static void listfiles(char *name)
{
	/* Makes vars static as much as possible to free stack space
	   since this function is recursive. */
	static struct stat statbuf;
	static struct dirent *dp;
	static BOOL endslash;
	static DIR *dirp;
	static char fullname[PATHLEN], *cp, **newlist;
	static int listsize;
	char **list, *n;
	int i, listused;

	if (LSTAT(name, &statbuf) < 0) {
		perror(name);
		bad |= 2;
		return;
	}
	/* Is this a file ? */
	if (!S_ISDIR(statbuf.st_mode)) {
		/* Apply filters. */
		if ((flags & (LSF_DIR | LSF_FILE)) != LSF_DIR) {
			printsep(NULL);
			lsfile(name, &statbuf, flags);
		}
		return;
	}
	/* Do all the files in a directory. */
	if ((dirp = opendir(name)) == NULL) {
		perror(name);
		bad |= 2;
		return;
	}

	/* Alloc list buffer. */
	listused = 0;
	listsize = LISTSIZE;
	endslash = (*name && (name[strlen(name) - 1] == '/'));
	if ((list = (char **) malloc(LISTSIZE * sizeof(char *))) == NULL) {
		fprintf(stderr, "No memory for ls buffer\n");
		exit(2);
	}

	if (flags & (LSF_RECUR | LSF_MULT))
		printsep(name);
	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '\0')
			continue;
		if (!(flags & LSF_ALL) && dp->d_name[0] == '.')
			continue;
		fullname[0] = '\0';
		if ((*name != '.') || (name[1] != '\0')) {
			strcpy(fullname, name);
			if (!endslash)
				strlcat(fullname, "/", sizeof(fullname));
		}
		strlcat(fullname, dp->d_name, sizeof(fullname));
		if (listused >= listsize) {
			newlist = realloc(list,
				((sizeof(char **)) * (listsize + LISTSIZE)));
			if (newlist == NULL) {
				fprintf(stderr, "No memory for ls buffer\n");
				bad |= 2;
				break;
			}
			list = newlist;
			listsize += LISTSIZE;
		}
		if ((list[listused] = strdup(fullname)) == NULL) {
			fprintf(stderr, "No memory for filenames\n");
			bad |= 2;
			break;
		}
		listused++;
	}
	closedir(dirp);
	/* Sort the files. */
	if (!(flags & LSF_UNSORT))
		qsort((char *) list, listused, sizeof(char *), flags & LSF_DOWN ? revnamesort : namesort);
	/* Now finally list the filenames. */
	for (i = 0; i < listused; i++) {
		n = list[i];
		/* Only do expensive stat calls if we actually need the data ! */
		if ((flags & (LSF_DIR|LSF_FILE|LSF_INODE|LSF_LONG|LSF_RECUR)) && LSTAT(n, &statbuf) < 0) {
			perror(n);
			bad |= 1;
			goto freename;
		}
		if ((cp = strrchr(n, '/')) != 0)
			++cp;
		else
			cp = n;
		/* Apply filters. */
		if (!(flags & (LSF_DIR | LSF_FILE)) ||
			((flags & LSF_DIR) && S_ISDIR(statbuf.st_mode)) ||
			((flags & LSF_FILE) && !S_ISDIR(statbuf.st_mode)))
			lsfile(cp, &statbuf, flags);
		if (!(flags & LSF_RECUR) || !S_ISDIR(statbuf.st_mode) || !strcmp(cp, ".") || !strcmp(cp, ".."))
freename:
			{ free(n); list[i] = NULL; }
	}
	/* Recursively list directories found. */
	if (flags & LSF_RECUR) {
		for (i = 0; i < listused; i++) {
			/* Only directories remain. */
			n = list[i];
			if (n) {
				listfiles(n);
				free(n);
			}
		}
	}
	free(list);
}

static void printusage(FILE *out)
{
	fprintf(out, "Usage: ls [-Radfiloru] [file...]\n");
}

int main(int argc, char *argv[])
{
	static char *def[1] = {"."};
	char *cp;
	flags = 0;
	for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
		for (cp = *argv + 1; *cp; ++cp) {
			switch (*cp) {
			case 'h':
				printusage(stdout);
				return 0;
			case 'd': /* list directories */
				flags |= LSF_DIR;
				break;
			case 'i': /* POSIX show inode */
				flags |= LSF_INODE;
				break;
			case 'l': /* POSIX long format */
				flags |= LSF_LONG;
				break;
			case 'f': /* list files (actually it lists non-directories) */
				flags |= LSF_FILE;
				break;
			case 'a': /* POSIX list also files beginning by '.' */
				flags |= LSF_ALL;
				break;
			case 'o': /* print permissions in octal */
				flags |= LSF_OCT;
				break;
			case 'u': /* do not sort */
				flags |= LSF_UNSORT;
				break;
			case 'r': /* POSIX reverse sorting order */
				flags |= LSF_DOWN;
				break;
			case 'R': /* POSIX recursive directory listing */
				flags |= LSF_RECUR;
				break;
			default:
				fprintf(stderr, "Unknown option -%c\n", *cp);
				printusage(stderr);
				return 2;
			}
		}
	}
	if (argc == 0) {
		argc = 1;
		argv = def;
	}
	if (argc > 1)
		flags |= LSF_MULT;
	while (--argc >= 0)
		listfiles(*argv++);
	fflush(stdout);
	/* 2 for a bad error, 1 for a minor error, 0 otherwise but not 3 for
	   both! */
	return (bad & 2) ? 2 : bad;
}
