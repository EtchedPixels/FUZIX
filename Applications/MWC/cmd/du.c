/*
 * Print a disc usage summary for
 * some directories.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define	NLINK	500		/* Size of link table */
#define	NFNAME	512		/* Size of filename */
#define	NRECUR	12		/* Depth of recursion < NFILE-3-2 */

int	aflag;			/* do files as well */
int	sflag;			/* only give total */
int	depth;			/* recursion depth */

int	nfile;			/* files allowed open */

struct	linktab	{
	ino_t	l_ino;
	dev_t	l_dev;
}	linktab[NLINK];

struct	linktab	*ltp;
char	fname[NFNAME];

char	toodeep[] = "directory structure too deep to traverse";

struct	stat	sb;

#define ND	20
#define NBN	256

/*
 * indirect block overhead based upon size of file
 */
long	ranges[] = {
	ND,				/* direct blocks */
	ND + NBN,			/* single indirect */
	ND + NBN*NBN,			/* double indirect */
	ND + NBN*NBN*NBN		/* tripple indirect */
};

extern long dufork(char *ep);

void usage(void)
{
	write(2, "Usage: du [-s] [name ...]\n", 26);
	exit(1);
}

static void writes(int fd, const char *p)
{
	write(fd, p, strlen(p));
}

void dumsg(const char *x, const char *p)
{
	writes(2, x);
	if (p) {
		write(2, " `", 2);
		writes(2, p);
		write(2, "'\n", 2);
	} else
		write(2, "\n", 1);
}

void duerr(const char *x, const char *p)
{
	dumsg(x, p);
	exit (1);
}

/*
 * Build up the next entry
 * in the name.
 */
char *buildname(struct dirent *dp, char *ep)
{
	char *cp = dp->d_name;
	size_t s = strlen(dp->d_name);

	if (ep + s + 2 >= &fname[NFNAME]) {
		dumsg(toodeep, NULL);
		return (NULL);
	}
	if (ep[-1] != '/')
		*ep++ = '/';
	strcpy(ep, cp);
	return (ep + s);
}

/*
 * Add an entry to the table
 * of i-numbers with multiple links.
 * If there are too many multi-link files,
 * they will get counted twice.
 * Return 1 if already there.
 */
int addlink(dev_t dev, ino_t ino)
{
	struct linktab *lp;

	for (lp = linktab; lp<ltp; lp++)
		if (lp->l_ino==ino && lp->l_dev==dev)
			return (1);
	if (lp-linktab >= NLINK)
		return (0);
	lp->l_ino = ino;
	lp->l_dev = dev;
	ltp++;
	return (0);
}


/*
 * Print out a size line.
 */

void prsize(uint16_t blocks)
{
	writes(1, _itoa(blocks));
	writes(1, "\t");
	writes(1, fname);
	write(1, "\n", 1);
}

/*
 * Do a du on a single file.
 * Now takes into account indirect blocks.
 */
long dusize(void)
{
	int i;
	long blocks;

	if ((sb.st_mode & S_IFMT) != S_IFDIR
	 && sb.st_nlink > 1
	 && addlink(sb.st_dev, sb.st_ino))
		return (0);
	blocks = (sb.st_size+BUFSIZ-1) / BUFSIZ;
	for (i = 0; i < sizeof(ranges)/sizeof(ranges[0]); ++i)
		if (blocks <= ranges[i])
			break;
	return (blocks + i);
}

/*
 * Do a du on a single entry
 * The pointer is the end pointer
 * into the fname buffer.
 */
long duentry(char *ep)
{
	uint16_t total;
	uint32_t fsize;
	char *np;
	DIR *fd;
	struct dirent *dp;
	int dirf;

	switch (sb.st_mode & S_IFMT) {
	case S_IFREG:
		return dusize();

	case S_IFDIR:
		total = dusize();
		if (++depth >= NRECUR) {
			depth = 0;
			return dufork(ep);
		}
		if ((fd = opendir(fname)) == NULL) {
			dumsg(fname, "cannot open");
			return (0);
		}
		while ((dp = readdir(fd)) != NULL) {
			np = dp->d_name;
			if (*np++=='.'
			  && (*np=='\0' || (*np++=='.' && *np=='\0')))
				continue;
			if ((np = buildname(dp, ep)) == NULL)
				continue;
			if (stat(fname, &sb) < 0) {
				dumsg("stat failed on", fname);
				continue;
			}
			dirf = (sb.st_mode&S_IFMT)==S_IFDIR;
			fsize = duentry(np);
			if (aflag && !sflag && !dirf)
				prsize(fsize);
			total += fsize;
		}
		closedir(fd);
		*ep = '\0';
		if (!sflag)
			prsize(total);
		--depth;
		return total;

	default:
		return 0;
	}
}

/*
 * Fork to do a du on recursive directory
 * structure that is too deep to fit into
 * user's open files.
 */
long dufork(char *ep)
{
	int i;
	int pid;
	int pfd[2] = {0, 0};
	int status;
	long sz = 0;

	fflush(stdout);
	if (pipe(pfd)<0 || (pid = fork())<0) {
		if (pfd[0]) {
			close(pfd[0]);
			close(pfd[1]);
		}
		dumsg(toodeep, NULL);
		return (0);
	}
	if (pid) {
		close(pfd[1]);
		while (wait(&status) >= 0)
			;
		if (status || read(pfd[0], &sz, sizeof(long)) != sizeof(long))
			sz = 0;
		close(pfd[0]);
		return (sz);
	}
	for (i=3; i<nfile; i++)
		if (i != pfd[1])
			close(i);
	sz = duentry(ep);
	write(pfd[1], &sz, sizeof(long));
	close(pfd[1]);
	exit(0);
}

/*
 * Print out disc usage summary
 */
static int du(char *name)
{
	char *ep, *cp;
	uint32_t size;
	int nondir;

	ltp = linktab;
	cp = name;
	ep = fname;
	while (*cp)
		*ep++ = *cp++;
	*ep = '\0';
	if (stat(name, &sb) < 0)
		duerr("nonexistent", name);
	nondir = (sb.st_mode&S_IFMT) != S_IFDIR;
	size = duentry(ep);
	if (sflag || nondir) {
		writes(1, _ltoa(size));
		write(1, "\t", 1);
		writes(1, fname);
		write(1, "\n", 1);
	}
	return (0);
}
	
void main(int argc, char *argv[])
{
	int i;
	char *ap;
	int estat;
	
	int nfile = sysconf(_SC_OPEN_MAX);

	while (argc>1 && *argv[1]=='-') {
		for (ap = &argv[1][1]; *ap != '\0'; ap++)
			switch (*ap) {
			case 'a':
				aflag = 1;
				break;

			case 's':
				sflag = 1;
				break;

			default:
				usage();
			}
		argc--;
		argv++;
	}
	if (argc < 2)
		estat = du(".");
	else {
		estat = 0;
		for (i=1; i<argc; i++)
			estat |= du(argv[i]);
	}
	exit (estat);
}

