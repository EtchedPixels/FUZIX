/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * list file or directory
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#define	NFILES	1024
DIR	*dirf;
char	stdbuf[BUFSIZ];

struct lbuf {
	union {
		char	lname[31];
		char	*namep;
	} ln;
	char	ltype;
	short	lnum;
	short	lflags;
	short	lnl;
	uid_t	luid;
	gid_t	lgid;
	long	lsize;
	time_t	lmtime;
};

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, cflg;
int	rflg	= 1;
long	year;
int	flags;
uid_t	lastuid	= -1;
uid_t	lastgid = -1;
char	tbuf[16];
char	gbuf[16];
long	tblocks;
int	statreq;
struct	lbuf	*flist[NFILES];
struct	lbuf	**lastp = flist;
struct	lbuf	**firstp = flist;
char	*dotp	= ".";

#define	ISARG	0100000

int	m1[] = { 1, S_IRUSR>>0, 'r', '-' };
int	m2[] = { 1, S_IWUSR>>0, 'w', '-' };
int	m3[] = { 2, S_ISUID, 's', S_IXUSR>>0, 'x', '-' };
int	m4[] = { 1, S_IRUSR>>3, 'r', '-' };
int	m5[] = { 1, S_IWUSR>>3, 'w', '-' };
int	m6[] = { 2, S_ISGID, 's', S_IXUSR>>3, 'x', '-' };
int	m7[] = { 1, S_IRUSR>>6, 'r', '-' };
int	m8[] = { 1, S_IWUSR>>6, 'w', '-' };
int	m9[] = { 2, S_ISVTX, 't', S_IXUSR>>6, 'x', '-' };


int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

void selectpair(int *pairp)
{
	register int n;

	n = *pairp++;
	while (--n>=0 && (flags&*pairp++)==0)
		pairp++;
	putchar(*pairp);
}

void pmode(int aflag)
{
	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		selectpair(*mp++);
}

/* FIXME: fast path for 'no entry' */
int getname(uid_t uid, char *buf)
{
	struct passwd *pw;
	if (uid==lastuid)
		return(0);
	fputs("uidl\n", stderr);
	pw = getpwuid(uid);
	if (pw == NULL)
		return -1;
	strncpy(buf, pw->pw_name, 15);
	lastuid = uid;
	return(0);
}

int getgrp(gid_t gid, char *buf)
{
	struct group *gr;
	if (gid==lastgid)
		return(0);
	fputs("gidl\n", stderr);
	gr = getgrgid(gid);
	if (gr == NULL)
		return -1;
	strncpy(buf, gr->gr_name, 15);
	lastgid = gid;
	return(0);
}

long nblock(long size)
{
	return((size+511)>>9);
}

void pentry(struct lbuf *ap)
{
	struct { uint8_t dminor, dmajor;};
	register int t;
	register struct lbuf *p;
	register char *cp;

	p = ap;
	if (p->lnum == -1)
		return;
	if (iflg)
		printf("%5u ", p->lnum);
	if (sflg)
	printf("%4D ", nblock(p->lsize));
	if (lflg) {
		putchar(p->ltype);
		pmode(p->lflags);
		printf("%2d ", p->lnl);
		t = p->luid;
		if (getname(t, tbuf)==0)
			printf("%-8.8s ", tbuf);
		else
			printf("%-8d ", t);
		t = p->luid;
		if (getgrp(t, gbuf)==0)
			printf("%-8.8s ", gbuf);
		else
			printf("%-8d ", t);
		if (p->ltype=='b' || p->ltype=='c')
			printf("%3d,%3d", major((int)p->lsize), minor((int)p->lsize));
		else
			printf("%7ld", p->lsize);
		cp = ctime(&p->lmtime);
		if(p->lmtime < year)
			printf(" %-7.7s %-4.4s ", cp+4, cp+20); else
			printf(" %-12.12s ", cp+4);
	}
	if (p->lflags&ISARG)
		printf("%s\n", p->ln.namep);
	else
		printf("%.14s\n", p->ln.lname);
}


/* FIXME */
static char dfile[512];

/* FIXME: length checks */
char *makename(char *dir, char *file)
{
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	*dp++ = '/';
	fp = file;
	for (i=0; i < 30; i++)
		*dp++ = *fp++;
	*dp = 0;
	return(dfile);
}

struct lbuf *gstat(char *file, int argfl)
{
	struct stat statb;
	register struct lbuf *rep;
	static int nomocore;

	if (nomocore)
		return(NULL);
	rep = (struct lbuf *)malloc(sizeof(struct lbuf));
	if (rep==NULL) {
		fprintf(stderr, "ls: out of memory\n");
		nomocore = 1;
		return(NULL);
	}
	if (lastp >= &flist[NFILES]) {
		static int msg;
		lastp--;
		if (msg==0) {
			fprintf(stderr, "ls: too many files\n");
			msg++;
		}
	}
	*lastp++ = rep;
	rep->lflags = 0;
	rep->lnum = 0;
	rep->ltype = '-';
	if (argfl || statreq) {
		if (stat(file, &statb)<0) {
			printf("%s not found\n", file);
			statb.st_ino = -1;
			statb.st_size = 0;
			statb.st_mode = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
		rep->lnum = statb.st_ino;
		rep->lsize = statb.st_size;
		switch(statb.st_mode&S_IFMT) {

		case S_IFDIR:
			rep->ltype = 'd';
			break;

		case S_IFIFO:
			rep->ltype = 'p';
			break;

		case S_IFBLK:
			rep->ltype = 'b';
			rep->lsize = statb.st_rdev;
			break;

		case S_IFCHR:
			rep->ltype = 'c';
			rep->lsize = statb.st_rdev;
			break;
		}
		rep->lflags = statb.st_mode & ~S_IFMT;
		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
		if(uflg)
			rep->lmtime = statb.st_atime;
		else if (cflg)
			rep->lmtime = statb.st_ctime;
		else
			rep->lmtime = statb.st_mtime;
		tblocks += nblock(statb.st_size);
	}
	return(rep);
}

void dirread(char *dir)
{
	static struct dirent *dentry;
	register int j;
	register struct lbuf *ep;
	

	if ((dirf = opendir(dir)) == NULL) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	while((dentry = readdir(dirf)) != NULL) {
		if (dentry->d_ino==0
		 || aflg==0 && dentry->d_name[0]=='.' &&  (dentry->d_name[1]=='\0'
			|| dentry->d_name[1]=='.' && dentry->d_name[2]=='\0'))
			continue;
		ep = gstat(makename(dir, dentry->d_name), 0);
		if (ep==NULL)
			continue;
		if (ep->lnum != -1)
			ep->lnum = dentry->d_ino;
		for (j=0; j < 30; j++)
			ep->ln.lname[j] = dentry->d_name[j];
	}
	closedir(dirf);
}

int compar(const void *pp1, const void *pp2)
{
	register const struct lbuf *p1, *p2;

	p1 = *(struct lbuf **)pp1;
	p2 = *(struct lbuf **)pp2;
	if (dflg==0) {
		if (p1->lflags&ISARG && p1->ltype=='d') {
			if (!(p2->lflags&ISARG && p2->ltype=='d'))
				return(1);
		} else {
			if (p2->lflags&ISARG && p2->ltype=='d')
				return(-1);
		}
	}
	if (tflg) {
		if(p2->lmtime == p1->lmtime)
			return(0);
		if(p2->lmtime > p1->lmtime)
			return(rflg);
		return(-rflg);
	}
	return(rflg * strcmp(p1->lflags&ISARG? p1->ln.namep: p1->ln.lname,
				p2->lflags&ISARG? p2->ln.namep: p2->ln.lname));
}

int main(int argc, char *argv[])
{
	int i;
	register struct lbuf *ep, **ep1;
	register struct lbuf **slastp;
	struct lbuf **epp;
	struct lbuf lb;

	time(&lb.lmtime);
	year = lb.lmtime - 6L*30L*24L*60L*60L; /* 6 months ago */
	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {

		case 'a':
			aflg++;
			continue;

		case 's':
			sflg++;
			statreq++;
			continue;

		case 'd':
			dflg++;
			continue;

		case 'l':
			lflg++;
			statreq++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'c':
			cflg++;
			continue;

		case 'i':
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

		default:
			continue;
		}
		argc--;
	}
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}
	if (argc==0) {
		argc++;
		argv = &dotp - 1;
	}
	for (i=0; i < argc; i++) {
		if ((ep = gstat(*++argv, 1))==NULL)
			continue;
		ep->ln.namep = *argv;
		ep->lflags |= ISARG;
	}
	fputs("1sort\n", stderr);
	qsort(firstp, lastp - firstp, sizeof *lastp, compar);
	fputs("1write\n", stderr);
	slastp = lastp;
	for (epp=firstp; epp<slastp; epp++) {
		ep = *epp;
		if (ep->ltype=='d' && dflg==0 || fflg) {
			if (argc>1)
				printf("\n%s:\n", ep->ln.namep);
			lastp = slastp;
	fputs("read\n", stderr);
			dirread(ep->ln.namep);
	fputs("sort\n", stderr);
			if (fflg==0)
				qsort(slastp,lastp - slastp,sizeof *lastp,compar);
	fputs("sort done\n", stderr);
			if (lflg || sflg)
				printf("total %ld\n", tblocks);
			for (ep1=slastp; ep1<lastp; ep1++)
				pentry(*ep1);
		} else 
			pentry(ep);
	}
	exit(0);
}
