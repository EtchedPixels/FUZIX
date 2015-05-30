/* The "ls" standard command.
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

#define LISTSIZE	256

typedef unsigned char BOOL;

#define PATHLEN		512

#ifdef	S_ISLNK
#define LSTAT	lstat
#else
#define LSTAT	stat
#endif

/*
 * Flags for the LS command.
 */
#define LSF_LONG	0x01
#define LSF_DIR 	0x02
#define LSF_INODE	0x04
#define LSF_MULT	0x08
#define LSF_FILE	0x10
#define LSF_DOWN	0x20
#define LSF_ALL		0x40
#define LSF_OCT		0x80
#define LSF_UNSORT	0x100

static char **list;
static int listsize;
static int listused;
static int flags;

static void lsfile(char *, struct stat *, int);
static char *modestring(int mode);
char *timestring(time_t *t);
int namesort(const void *p1, const void *p2);

long time_zone = 0;
long timezone = 0;


/*
 * Sort routine for list of filenames.
 */
int namesort(const void *p1, const void *p2)
{
    return strcmp(* (char * const *)p1, * (char * const *) p2);
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
char *timestring(time_t *t)
{
    char *str;
    static char buf[26];

    str = ctime(t);
    strcpy(buf, &str[4]);
    buf[12] = '\0';
#if 0				/* ??? */
    if ((t > now) || (t < now - 365 * 24 * 60 * 60L)) {
	strcpy(&buf[7], &str[20]);
	buf[11] = '\0';
    }
#endif
    return buf;
}

/*
 * Do an LS of a particular file name according to the flags.
 */
static void lsfile(char *name, struct stat *statbuf, int flags)
{
    static char username[12];
    static int userid;
    static BOOL useridknown;
    static char groupname[12];
    static int groupid;
    static BOOL groupidknown;

    struct passwd *pwd;
    struct group *grp;
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
#if 1
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

void main(int argc, char *argv[])
{
    static char *def[1] =
    {"."};

    struct stat statbuf;
    struct dirent *dp;
    BOOL endslash;
    DIR *dirp;
    int i;
    char *cp, *name, **newlist;
	static char fullname[PATHLEN];

    if ((list = (char **) malloc(LISTSIZE * sizeof(char *))) == NULL) {
	fprintf(stderr, "No memory for ls buffer\n");
	return;
    }
    listsize = LISTSIZE;
    listused = 0;
    flags = 0;
    for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
	for (cp = *argv + 1; *cp; ++cp) {
	    switch (*cp) {
	    case 'h':
		printf("Usage: ls [-dilfaouR] [pattern]\n");
		return;
	    case 'd':
		flags |= LSF_DIR;
		break;
	    case 'i':
		flags |= LSF_INODE;
		break;
	    case 'l':
		flags |= LSF_LONG;
		break;
	    case 'f':
		flags |= LSF_FILE;
		break;
	    case 'a':
		flags |= LSF_ALL;
		break;
	    case 'o':
		flags |= LSF_OCT;
		break;
	    case 'u':
		flags |= LSF_UNSORT;
		break;
	    case 'R':
		flags |= LSF_DOWN;
		break;
	    default:
		fprintf(stderr, "Unknown option -%c\n", *cp);
		return;
	    }
	}
    }
    if (argc == 0) {
	argc = 1;
	argv = def;
    }
    if (argc > 1)
	flags |= LSF_MULT;
    while (--argc >= 0) {
	name = *argv++;
	endslash = (*name && (name[strlen(name) - 1] == '/'));
	if (LSTAT(name, &statbuf) < 0) {
	    perror(name);
	    continue;
	}
	if ((flags & LSF_DIR) || (!S_ISDIR(statbuf.st_mode))) {
	    lsfile(name, &statbuf, flags);
	    continue;
	}
	/* Do all the files in a directory. */
	if ((dirp = opendir(name)) == NULL) {
	    perror(name);
	    continue;
	}
	if (flags & LSF_MULT)
	    printf("\n%s:\n", name);
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
		    break;
		}
		list = newlist;
		listsize += LISTSIZE;
	    }
	    if ((list[listused] = strdup(fullname)) == NULL) {
		fprintf(stderr, "No memory for filenames\n");
		break;
	    }
	    listused++;
	}
	closedir(dirp);
	/* Sort the files. */
	if (!(flags & LSF_UNSORT))
	    qsort((char *) list, listused,
		  sizeof(char *), namesort);	       /**/
		/* Now finally list the filenames. */
	for (i = 0; i < listused; i++, free(name)) {
	    name = list[i];
	    if (LSTAT(name, &statbuf) < 0) {
		perror(name);
		continue;
	    }
	    if (((flags & LSF_DIR) && !S_ISDIR(statbuf.st_mode)) ||
		((flags & LSF_FILE) && S_ISDIR(statbuf.st_mode)))
		continue;
	    if ((cp = strrchr(name, '/')) != 0)
		++cp;
	    else
		cp = name;
	    lsfile(cp, &statbuf, flags);
	}
	listused = 0;
    }
    fflush(stdout);
}

