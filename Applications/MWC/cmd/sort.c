/*
 * The sort command.
 * It does unique sorting, merges, and
 * ordinary sorts with zillions of
 * ways of specifying the sort keys.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>

#define MAXUCHAR	256	/* FIXME: where should this come from ? */
#define MAXINT		32767

#define	NREC	400		/* Longest key record */
#define	NSEL	20		/* Number of records in selection list */
#define	NPOS	20		/* Number of positionals */
#define	NTFILE	6		/* Number of intermediate files */
#define	MORDER		(NTFILE-1)	/* Order of polyphase merge */
#define	NDIST	(sizeof(dists)/sizeof(dists[0]))
#define	NCSET	(MAXUCHAR+1)	/* Size of char set */
#define	NSBRK	1024		/* Amount to add at a time */
#define BADSBRK ((char *) -1)	/* fail from sbrk() */

#define	rfree(p) {p->r_next=frlist;frlist=p;}	/* Free a run */

/* Key ordering flags (global and field skips) */
#define	KBLANK	01		/* Ignore leading blanks */
#define	KDICT	02		/* Dictionary order (letters, digits, blanks) */
#define	KFOLD	04		/* Fold upper case onto lower case */
#define	KIGNORE	010		/* Ignore non-ascii characters */
#define	KNUM	020		/* Numeric sort - skip leading blanks */
#define	KREV	040		/* Reverse order of sort */

/*
 * Mapping table to speed up folding.
 * Initialised by `sortinit'.
 */
char tabfold[NCSET];

/*
 * The temp file structure.  One for each file
 * contains the list-head for the runs and
 * the FILE stream pointer.
 */
typedef struct TFILE {
	struct RUN *tf_runs;
	FILE *tf_fp;
	int tf_nrun;
	off_t tf_start;
} TFILE;

TFILE tfiles[NTFILE];

/*
 * The structure of each run.  contains
 * a pointer to the next and the start
 * and length of the run.
 */
typedef struct RUN {
	struct RUN *r_next;
	int r_length;		/* Number of records in run */
} RUN;

/*
 * Entries in positional (+m.n-m.n) parameters
 */
typedef struct POS {
	int p_sflags;		/* Start flags */
	int p_sm;		/* fields */
	int p_sn;		/* chars */
	int p_eflags;		/* End flags */
	int p_em;		/* Ending fields */
	int p_en;		/* Ending chars */
} POS;

POS pos[NPOS];
POS *posp = &pos[-1];

/*
 * Numeric field breakout structure.
 */
typedef struct NUM {
	int n_sign;		/* Sign */
	int n_magn;		/* Integer magnitude */
	int n_fmagn;		/* Fractional magnitude */
	const char *n_bgn;	/* beginning */
	const char *n_end;	/* ending */
} NUM;

char **flist;
char *deflist[] = {
	"-", NULL
};

/*
 * This is the best distribution of runs for
 * the polyphase merge.  It is a sort of n-way
 * distribution of the Fibonacci number sequence.
 * Each level contains a total number of runs
 * and a way to subdivide them to this.  Dummy
 * runs are added to round out the initial distribution.
 * Each successive distribution vector (m0,m1,m2,m3,m4)
 * is obtained by cross product with this matrix:
 *	1 1 1 1 1
 *	1 0 0 0 0
 *	0 1 0 0 0
 *	0 0 1 0 0
 *	0 0 0 1 0
 * which can be generalised to any order of merge (other
 * than 5-way).
 * Also no more entries are given as this is
 * likely already overkill for sorting during
 * the lifetime of most machines.
 */
struct dists {
	int d_totruns;		/* Total of next 5 elements */
	int d_runs[MORDER];	/* initial distribution */
} dists[] = {
	    { 1, { 1, 0, 0, 0, 0 }},
	    { 5, { 1, 1, 1, 1, 1 }},
	    { 9, { 2, 2, 2, 2, 1 }},
	    { 17, { 4, 4, 4, 3, 2 }},
	    { 33, { 8, 8, 7, 6, 4 }},
	    { 65, { 16, 15, 14, 12, 8 }},
	    { 129, { 31, 30, 28, 24, 16 }},
	    { 253, { 61, 59, 55, 47, 31 }},
	    { 497, { 120, 116, 108, 92, 61 }},
	    { 977, { 236, 228, 212, 181, 120 }},
	    { 1921, { 464, 448, 417, 356, 236 }},
	    { 3777, { 912, 881, 820, 700, 464 }},
	    { 7425, { 1793, 1732, 1612, 1376, 912 }},
	    { 14597, { 3525, 3405, 3169, 2705, 1793 }},
	    { 28697, { 6930, 6694, 6230, 5318, 3525 }}
};

struct dists *savedsp;		/* Save current distribution level */

char *outname;			/* Output other than stdout */
char *tempdir;			/* Other than default temp directory */
char template[100];
char obuf[BUFSIZ];
char ibuf[BUFSIZ];

/*
 * Structure for each input record
 * in natural selection.  First
 * an insertion sort fills the
 * tree and then all records
 * are input and output until
 * too many are out of order.
 */
typedef struct SEL {
	char *s_inb;		/* Input buffer pointer */
	int s_length;		/* Length of run in selection */
	FILE *s_fp;		/* File pointer of run */
} SEL;

RUN *frlist;			/* Free run list */
SEL sel[NSEL];
SEL lastrec;
char inbuf[NSEL + 1][NREC];
#define	LASTREC	lastrec.s_inb	/* Buffer pointer of last written record */
long inlined;			/* Record number of input line, for error recovery */
char temperr[] = "Temporary file open error";
char tmpwerr[] = "Temporary file write error";
char nomem[] = "Out of memory";
int pid;			/* Current process ID */
char tabc;			/* Tab character */
int cflag;			/* Check ordering only */
int mflag;			/* Merge only */
int uflag;			/* Unique sort */
int kflags;			/* Flags to control order to sort */

int kcompar(const char *s1, const char *s2);

int (*compar) (const char *, const char *) = kcompar;

/*
 * Initialise tables that speed up
 * special orderings.
 */
void sortinit(void)
{
	int c;
	char *cp;

	cp = tabfold;
	for (c = 0; c < NCSET; c++)
		*cp++ = (isascii(c) && isupper(c)) ? tolower(c) : c;
}

/*
 * Exit, removing the tempfiles.
 */
void rmexit(int s)
{
	int c;
	char tempname[120];

	for (c = 'a'; c < 'a' + NTFILE; c++) {
		sprintf(tempname, template, c);
		unlink(tempname);
	}
	_exit(s);
}

/*
 * Protect from the specified signal number.
 * This makes that signal call the cleanup
 * routine, unless that signal was already ignored.
 */
void protect(int signo)
{
	if (signal(signo, SIG_IGN) != SIG_IGN)
		signal(signo, rmexit);
}

void serr(char *x, ...)
{
	va_list va;
	va_start(va, x);
	fputs("sort: ", stderr);
	vfprintf(stderr, x, va);
	fputs(".\n", stderr);
	va_end(va);
	rmexit(1);
}

/*
 * This allocator simply does sbrk calls so that
 * it can grow (easily) the space as it needs without
 * threading it.  It also checks and prints an error
 * if out of space.
 */
char *alloc(unsigned int nb)
{
	static int *rp, *ep;
	int *cp;

	if ((nb = (nb + sizeof(int) - 1) / sizeof(int)) == 0)
		serr(nomem);
	if (rp == NULL || rp + nb >= ep) {
		if ((ep = (int *) sbrk(NSBRK)) == BADSBRK)
			serr(nomem);
		if (rp == NULL)
			rp = ep;
		ep += NSBRK / sizeof(int);
	}
	cp = rp;
	rp += nb;
	return (cp);
}

/*
 * Allocate a new run.  Uses our own threaded free list
 * and sbrk-style alloc when none of these.
 */
RUN *ralloc(void)
{
	RUN *p;

	if ((p = frlist) != NULL) {
		frlist = p->r_next;
		return (p);
	}
	return ((RUN *) alloc(sizeof(RUN)));
}

/*
 * Make temporary file ` n '.
 * Called as they are first needed.
 */
int maketemp(int n)
{
	static int first = 1;
	char tempname[120];
	FILE *fp;

	if (pid == 0) {
		pid = getpid();
		sprintf(template, "%s/sort%d%%c",
			tempdir == NULL ? "/tmp" : tempdir, pid);
	}
	sprintf(tempname, template, n + 'a');
	if ((fp = fopen(tempname, "w")) == NULL) {
		if (first && tempdir == NULL) {
			sprintf(template, "/usr/tmp/sort%d%%c", pid);
			sprintf(tempname, template, n + 'a');
			if ((fp = fopen(tempname, "w")) == NULL)
				serr(temperr);
		} else
			serr(temperr);
	}
	fclose(fp);
	if ((tfiles[n].tf_fp = fopen(tempname, "r+w")) == NULL)
		serr(temperr);
	setbuf(tfiles[n].tf_fp, alloc(BUFSIZ));
	first = 0;
}

/*
 * Get the next input character.  This
 * automatically goes from one file to the
 * next on EOF and only returns EOF at real
 * end of file.
 */

int sgetc(void)
{
	static FILE *fp;
	int c;

      again:
	if (fp == NULL)
		if (*flist == NULL)
			return (EOF);
		else {
			if ((*flist)[0] == '-' && (*flist)[1] == '\0')
				fp = stdin;
			else if ((fp = fopen(*flist, "r")) == NULL)
				fprintf(stderr, "sort: cannot open %s\n",
					*flist);
			flist++;
			setbuf(fp, ibuf);
			goto again;
		}
	if ((c = getc(fp)) == EOF) {
		if (fp != stdin)
			fclose(fp);
		fp = NULL;
		goto again;
	}
	return (c);
}

/*
 * Get a string from sort input.  NULL on EOF, leave
 * trailing newlines on.
 */
char *sgets(char *as)
{
	unsigned max = NREC;
	int c;
	char *s;

	s = as;
	while (--max > 0 && (c = sgetc()) != EOF)
		if ((*s++ = c) == '\n') {
			inlined += 1;
			break;
		}
	if (max == 0)
		serr("input record #%ld exceeds maximum length %d",
		     inlined + 1, NREC);
	*s = '\0';
	return (c == EOF && s == as ? NULL : as);
}


/* Errors and usage messages */
void usage(void)
{
	fprintf(stderr,
		"Usage: sort [options] [+pos1 [-pos2]] ... [file ...]\n");
	fprintf(stderr,
		"Options: [-mubdfinr] [-tx] [-T directory] [-o name]\n");
	exit(1);
}

/*
 * Read in the ordering options into
 * the int that is referenced by `flagp'
 * from the string `s'.  Used both for
 * global options and with skip options.
 */
void opts(char *s, int *flagp)
{
	while (*s)
		switch (*s++) {
		case 'b':
			*flagp |= KBLANK;
			break;

		case 'd':
			*flagp |= KDICT;
			break;

		case 'f':
			*flagp |= KFOLD;
			break;

		case 'i':
			*flagp |= KIGNORE;
			break;

		case 'n':
			*flagp |= KNUM;
			break;

		case 'r':
			*flagp |= KREV;
			break;

		default:
			usage();
		}
}

/*
 * Do the 5-way (MORDER) merge on one set
 * of runs which are described in the `sel'
 * struct array.
 */
void mergeread(void)
{
	SEL *sp;
	SEL *minp;
	int neof = 0;

	for (sp = sel; sp < &sel[MORDER]; sp++) {
		if (sp->s_length == 0) {
			neof++;
			sp->s_inb = NULL;
		} else {
			fgets(sp->s_inb, NREC, sp->s_fp);
			sp->s_length--;
		}
	}
	while (neof < MORDER) {
		minp = NULL;
		for (sp = sel; sp < &sel[MORDER]; sp++) {
			if (sp->s_inb == NULL)
				continue;
			if (minp == NULL) {
				minp = sp;
				continue;
			}
			if ((*compar) (sp->s_inb, minp->s_inb) <= 0)
				minp = sp;
		}
		fputs(minp->s_inb, tfiles[NTFILE - 1].tf_fp);
		if (minp->s_length-- == 0) {
			minp->s_inb = NULL;
			neof++;
		} else
			fgets(minp->s_inb, NREC, minp->s_fp);
	}
}

/*
 * Copy each run file to the appropriate temp file
 * given by the run number (`runno'). * Also, check during the input for the
	file 's
 * being sorted properly.
 * If `ifp' is NULL, this creates an empty * (distinguished from dummy) run. */

RUN *copyfile(FILE * ifp, int runno)
{
	RUN *arp, *rp;
	int c;
	TFILE *tfp;
	FILE *ofp;

	tfp = &tfiles[runno];
	tfp->tf_nrun++;
	if ((ofp = tfp->tf_fp) == NULL) {
		maketemp(runno);
		ofp = tfp->tf_fp;
	}
	arp = ralloc();
	arp->r_next = NULL;
	arp->r_length = 0;
	if (tfp->tf_runs == NULL)
		tfp->tf_runs = arp;
	else {
		for (rp = tfp->tf_runs; rp->r_next != NULL;
		     rp = rp->r_next);
		rp->r_next = arp;
	}
	if (ifp == NULL)
		return (arp);
	while ((c = getc(ifp)) != EOF) {
		if (c == '\n')
			arp->r_length++;
		putc(c, ofp);
	}
	fflush(ofp);
	if (ferror(ofp))
		serr(tmpwerr);
	return (arp);
}


/*
 * Do one step of the polyphase merge.  The calling
 * routine has arranged that the output is
 * position MORDER and the inputs are the first
 * MORDER positions.
 */
void mergestep(void)
{
	int min;
	int i;
	RUN *rp;
	FILE *ofp;
	int len;
	RUN *orp;

	rewind(tfiles[MORDER].tf_fp);
	tfiles[MORDER].tf_start = 0;
	min = tfiles[0].tf_nrun;
	fseek(tfiles[0].tf_fp, tfiles[0].tf_start, 0);
	for (i = 1; i < MORDER; i++) {
		fseek(tfiles[i].tf_fp, tfiles[i].tf_start, 0);
		if (tfiles[i].tf_nrun < min)
			min = tfiles[i].tf_nrun;
	}
	while (min-- > 0) {
		orp = copyfile(NULL, MORDER);
		ofp = tfiles[MORDER].tf_fp;
		len = 0;
		for (i = 0; i < MORDER; i++) {
			if ((rp = tfiles[i].tf_runs) != NULL) {
				tfiles[i].tf_runs = rp->r_next;
				len += rp->r_length;
				sel[i].s_length = rp->r_length;
				rfree(rp);
			} else
				sel[i].s_length = 0;
			tfiles[i].tf_nrun--;
			sel[i].s_inb = inbuf[i];
			sel[i].s_fp = tfiles[i].tf_fp;
		}
		orp->r_length = len;
		mergeread();
	}
	for (i = 0; i < MORDER; i++)
		tfiles[i].tf_start = ftell(tfiles[i].tf_fp);
}

/*
 * Merge the data
 * The algorithm is polyphase merge from
 * Knuth.
 */
int merge(void)
{
	int i, j;
	int nr;
	TFILE temptf;

	nr = 0;
	j = 0;
	for (i = 0; i < MORDER; i++)
		if (tfiles[i].tf_nrun) {
			j = i;
			nr += tfiles[i].tf_nrun;
		}
	if (nr <= 1) {
		tfiles[NTFILE - 1] = tfiles[j];
		return (0);
	}
	for (i = 0; i < NTFILE; i++)
		if (tfiles[i].tf_fp == NULL)
			maketemp(i);
	for (;;) {
		mergestep();
		nr = 0;
		for (i = 0; i < MORDER; i++) {
			nr += tfiles[i].tf_nrun;
			if (tfiles[i].tf_nrun == 0)
				j = i;
		}
		if (nr <= 1)
			break;
		/*
		 * Exchange output and
		 * zeroed one so output
		 * is always in a fixed place.
		 */
		temptf = tfiles[MORDER];
		tfiles[MORDER] = tfiles[j];
		tfiles[j] = temptf;
	}
	return (0);
}

/*
 * Fill out the current distribution level
 * with dummy runs.
 */
void dummyruns(void)
{
	struct dists *dsp;
	TFILE *tfp;
	int i;

	dsp = savedsp;
	for (i = 0; i < MORDER; i++)
		for (tfp = &tfiles[i]; tfp->tf_nrun < dsp->d_runs[i];)
			tfp->tf_nrun++;
}

/*
 * Calculate the next run number to use,
 * based on the number that we already have.
 * The dummy runs go to the left (largest
 * number so they are used the most often)
 * Dummy runs are installed by another routine
 * after all runs are entered.
 */
int nextrun(void)
{
	struct dists *dsp;
	int i;

	for (dsp = dists; dsp < &dists[NDIST]; dsp++) {
		for (i = MORDER - 1; i >= 0; i--) {
			if (tfiles[i].tf_nrun < dsp->d_runs[i]) {
				savedsp = dsp;
				return (i);
			}
		}
	}
	serr("Ridiculously many runs");
}


/*
 * Copy the runs into the temp-files
 * for already-sorted but not merged data.
 */
int copyruns(void)
{
	char **flp = flist;
	char *fn;
	FILE *fp;
	int s = 0;

	while ((fn = *flp++) != NULL) {
		if (fn[0] == '-' && fn[1] == '\0')
			fp = stdin;
		else if ((fp = fopen(fn, "r")) == NULL) {
			fprintf(stderr, "sort: cannot open `%s'\n", fn);
			s = 1;
			continue;
		}
		setbuf(fp, ibuf);
		if (copyfile(fp, nextrun()) == NULL)
			return (1);
		if (fp != stdin)
			fclose(fp);
	}
	return (s);
}

/*
 * Insert the item at position `n'
 * into the sel table in sorted order.
 */

void insert(int n)
{
	SEL *sp1, *sp2;
	char *tmp;

	sp2 = &sel[n];
	for (sp1 = &sel[0]; sp1 < sp2; sp1++)
		if ((*compar) (sp1->s_inb, sp2->s_inb) > 0) {
			tmp = sp2->s_inb;
			for (; sp2 > sp1; sp2--)
				sp2->s_inb = (sp2 - 1)->s_inb;
			sp1->s_inb = tmp;
			break;
		}
}


/*
 * Use selection to create the initial runs.
 */
int selection(void)
{
	TFILE *tfp;
	RUN *rp;
	int i;
	int nsel;

	for (;;) {
		for (i = 0; i < NSEL; i++)
			sel[i].s_inb = inbuf[i];
		LASTREC = NULL;
		rp = copyfile(NULL, i = nextrun());
		tfp = &tfiles[i];
		for (nsel = 0; sgets(inbuf[nsel]) != NULL;) {
			insert(nsel);
			if (++nsel >= NSEL)
				break;
		}
		for (i = 0; i < nsel; i++) {
			if (uflag) {
				if (LASTREC != NULL)
					if ((*compar)
					    (LASTREC, sel[i].s_inb) == 0)
						continue;
				LASTREC = sel[i].s_inb;
			}
			fputs(sel[i].s_inb, tfp->tf_fp);
			rp->r_length++;
		}
		if (nsel < NSEL)
			break;
	}
	return (0);
}


/*
 * Reversed version of strcmp.  If this
 * were more common, it could be written
 * out in full to save the extra routine call.
 */
int rstrcmp(const char *s1, const char *s2)
{
	return (-strcmp(s1, s2));
}


/*
 * Actually figure out what kind of
 * sorting we have to do.
 */
int sort(void)
{
	FILE *fp;

	/*
	 * Optimisation for simple sorts
	 * to cut compare time down.
	 */
	if (posp < &pos[0] && (kflags & ~KREV) == 0) {
		if (kflags & KREV)
			compar = rstrcmp;
		else
			compar = strcmp;
	}
	if (cflag) {
		char *b1, *b2;

		if (mflag || outname != NULL || uflag)
			fprintf(stderr,
				"Checking only--some options ignored\n");
		b1 = NULL;
		b2 = inbuf[0];
		while (sgets(b2) != NULL) {
			if (b1 == NULL) {
				b2 = inbuf[1];
				b1 = inbuf[0];
				continue;
			}
			if ((*compar) (b1, b2) > 0) {
				fprintf(stderr,
					"sort: out of order at:\n");
				fprintf(stderr, "%s", b2);
				return (1);
			}
			if (b2 == inbuf[1]) {
				b1 = inbuf[1];
				b2 = inbuf[0];
			} else {
				b1 = inbuf[0];
				b2 = inbuf[1];
			}
		}
		return (0);
	}
	if (mflag) {
		if (copyruns())
			return (1);
	} else {
		if (selection())
			return (1);
	}
	dummyruns();
	if (merge())
		return (1);
	if (outname != NULL) {
		if (freopen(outname, "w", stdout) != stdout)
			serr("Cannot open output `%s'", outname);
	}
	fp = tfiles[MORDER].tf_fp;
	rewind(fp);
	LASTREC = NULL;
	while (fgets(inbuf[0], NREC, fp) != NULL) {
		if (uflag) {
			if (LASTREC == NULL)
				LASTREC = inbuf[1];
			else if ((*compar) (LASTREC, inbuf[0]) == 0)
				continue;
			strcpy(LASTREC, inbuf[0]);
		}
		fputs(inbuf[0], stdout);
	}
	fflush(stdout);
	if (ferror(stdout))
		serr("Write error on `%s'",
		     outname == NULL ? "(stdout)" : outname);
	fclose(fp);
	return (0);
}

/*
 * Read in the skip (either the `+' or
 * the `-' kind).  Syntax check it and
 * store it away.
 */
void readskip(char *s)
{
	int n;
	int plusskip = 0;

	plusskip = *s++ == '+';
	if (plusskip) {
		if (++posp >= &pos[NPOS])
			serr("Too many positional parameters");
		posp->p_em = MAXINT;
	}
	for (n = 0; isdigit(*s);)
		n = n * 10 + *s++ - '0';
	if (plusskip)
		posp->p_sm = n;
	else
		posp->p_em = n;
	if (*s == '.') {
		s++;
		for (n = 0; isdigit(*s);)
			n = n * 10 + *s++ - '0';
		if (plusskip)
			posp->p_sn = n;
		else
			posp->p_en = n;
	}
	opts(s, plusskip ? &posp->p_sflags : &posp->p_eflags);
}


int main(int argc, char *argv[])
{
	char *ap;
	char dummyop[2];

	setbuf(stdin, ibuf);
	setbuf(stdout, obuf);
	setbuf(stderr, NULL);

	protect(SIGINT);
	protect(SIGHUP);
	protect(SIGPIPE);
	protect(SIGTERM);

	while (argc > 1 && (*argv[1] == '-' || *argv[1] == '+')) {
		if (*argv[1] == '+') {
			readskip(argv[1]);
			argv++;
			argc--;
			if (argc > 1 && argv[1][0] == '-'
			    && isdigit(argv[1][1])) {
				readskip(argv[1]);
				argv++;
				argc--;
			}
			continue;
		}
		for (ap = &argv[1][1]; *ap != '\0'; ap++)
			switch (*ap) {

				/*
				 * Non-ordering options.
				 */
			case 'c':	/* Check ordering only */
				cflag = 1;
				break;

			case 'm':	/* Merge only */
				mflag = 1;
				break;

			case 'o':	/* Output other than stdout */
				if (outname != NULL)
					serr("Only one output name allowed");
				if (--argc < 2)
					usage();
				argv++;
				outname = argv[1];
				break;

			case 'T':	/* Alternative temp directory */
				if (tempdir != NULL)
					serr("Only one `-T' allowed");
				if (--argc < 2)
					usage();
				argv++;
				tempdir = argv[1];
				break;

			case 'u':	/* Unique sort */
				uflag = 1;
				break;

				/*
				 * Lexicographic ordering options.
				 */
			case 't':	/* Tab character */
				if ((tabc = *++ap) == '\0')
					usage();
				break;

			default:
				dummyop[0] = *ap;
				dummyop[1] = '\0';
				opts(dummyop, &kflags);
			}
		argc--;
		argv++;
	}
	if (argc > 1)
		flist = argv + 1;
	else
		flist = deflist;
	sortinit();
	rmexit(sort());
}

/*
 * Parse a numeric field into sign, magnitude, integer and fractional parts.
 */
int numpars(const char *p, const char *ep, NUM * np)
{
	const char *bp;
	const char *fbgn;
	int fsum;

	bp = p;
	fbgn = NULL;
	fsum = 0;
	np->n_sign = 1;
	np->n_magn = 0;
	np->n_fmagn = 16000;
	np->n_bgn = NULL;
	for (; p < ep; p += 1)
		if (*p == '0') {
			if (fbgn != NULL) {
				if (fsum == 0)
					np->n_fmagn -= 1;
			} else if (np->n_magn != 0)
				np->n_magn += 1;
		} else if (isdigit(*p)) {
			if (fbgn != NULL)
				fsum += 1;
			else if (np->n_magn++ == 0)
				np->n_bgn = p;
		} else if (*p == '-') {
			if (p != bp)
				break;
			np->n_sign = -1;
		} else if (*p == '.') {
			if (fbgn != NULL || p + 1 == ep)
				break;
			fbgn = p + 1;
		} else {
			break;
		}
	np->n_end = p;
	if (fsum == 0) {
		np->n_fmagn = 0;
		fbgn = NULL;
	}
	if (np->n_bgn == NULL)
		np->n_bgn = fbgn;
	if (np->n_bgn == NULL)
		np->n_bgn = p;
}

/*
 * Compare for the finally found field.
 * This takes into account all of the options
 * and the end and start of each string.
 */
int fcompar(const char *s1, const char *e1, const char *s2, const char *e2,
	    int flags)
{
	const char *p1, *p2;
	int ret = 0;

	p1 = s1;
	p2 = s2;
	if (flags & (KBLANK | KNUM)) {
		while (*p1 == ' ' || *p1 == '\t')
			p1++;
		while (*p2 == ' ' || *p2 == '\t')
			p2++;
	}
	if (flags & KNUM) {
		NUM n1, n2;

		numpars(p1, e1, &n1);
		numpars(p2, e2, &n2);
		/* Compare integer magnitudes and signs */
		ret = n1.n_sign * n1.n_magn - n2.n_sign * n2.n_magn;
		if (ret != 0)
			ret = (ret < 0) ? -1 : 1;
		else {
			/* Compare integer parts */
			p1 = n1.n_bgn;
			p2 = n2.n_bgn;
			while (--n1.n_magn >= 0 && ret == 0)
				if (*p1 > *p2)
					ret = n1.n_sign;
				else if (*p1 < *p2)
					ret = -n1.n_sign;
				else {
					p1 += 1;
					p2 += 1;
				}
		}
		if (ret == 0)
			/* Compare fractional magnitudes and signs */
			ret =
			    n1.n_sign * n1.n_fmagn -
			    n2.n_sign * n2.n_fmagn;
		if (ret != 0)
			ret = ret < 0 ? -1 : 1;
		else {
			/* Compare fractional parts */
			e1 = n1.n_end;
			e2 = n2.n_end;
			while (p1 < e1 && p2 < e2 && ret == 0)
				if (*p1 > *p2)
					ret = n1.n_sign;
				else if (*p1 < *p2)
					ret = -n1.n_sign;
				else {
					p1 += 1;
					p2 += 1;
				}
		}
		if (ret == 0) {
			if (p1 < e1)
				ret = n1.n_sign;
			else
				ret = -n1.n_sign;
		}
	} else {
		int c1, c2;

		if (flags & (KDICT | KIGNORE)) {
			for (;;) {
				for (; p1 < e1; p1++) {
					if (flags & KIGNORE
					    && !isprint(*p1))
						continue;
					if (flags & KDICT)
						if (!(isspace(*p1)
						      || isalnum(*p1)))
							continue;
					if (flags & KFOLD)
						c1 = tabfold[*p1++];
					else
						c1 = *p1++;
					break;
				}
				for (; p2 < e2; p2++) {
					if (flags & KIGNORE
					    && !isprint(*p2))
						continue;
					if (flags & KDICT)
						if (!(isalnum(*p2)
						      || isspace(*p2)))
							continue;
					if (flags & KFOLD)
						c2 = tabfold[*p2++];
					else
						c2 = *p2++;
					break;
				}
				if (p1 >= e1 || p2 >= e2)
					break;
				if (c1 != c2)
					break;
			}
		} else if (flags & KFOLD) {
			for (;;) {
				c1 = tabfold[*p1++];
				c2 = tabfold[*p2++];
				if (p1 >= e1 || p2 >= e2)
					break;
				if (c1 != c2)
					break;
			}
		} else {
			for (;;) {
				c1 = *p1++;
				c2 = *p2++;
				if (p1 >= e1 || p2 >= e2)
					break;
				if (c1 != c2)
					break;
			}
		}
		if (p1 <= e1 && p2 <= e2) {
			if (c1 < c2)
				ret--;
			else if (c1 > c2)
				ret++;
		} else if (p1 > e1) {
			ret++;
		} else
			ret--;
	}
	if (flags & KREV)
		return (-ret);
	return (ret);
}

/*
 * Skip fields and space, returning the new pointer.
 * Arguments are `s' for string start, `m' and `n' from
 * the positional `m.n' format.
 * `f' is the flags - only `b' is
 * significant here.
 */
const char *fskip(const char *s, int m, int n, int f)
{
	while (m--) {
		if (tabc) {
			while (*s != tabc && *s != '\0')
				s++;
			if (*s != '\0')
				s++;
			else
				break;
		} else {
			while (*s == ' ' || *s == '\t')
				s++;
			while (*s != ' ' && *s != '\t' && *s != '\0')
				s++;
			if (*s == '\0')
				break;
			if (m == 0)
				while (*s == ' ' || *s == '\t')
					s++;
		}
	}
	if (f & KBLANK)
		while (*s == ' ' || *s == '\t')
			s++;
	while (n--) {
		if (*s == '\0')
			break;
		s++;
	}
	return (s);
}


/*
 * Compare keys in strings `s1' and `s2'
 * taking into account all of the key selection
 * options and positional fields.
 * All comparison routines return -1 or <, 0 for equal,
 * and 1 for >.
 */
int kcompar(const char *s1, const char *s2)
{
	POS *pp;
	const char *ep1, *ep2;
	int ret = 0;
	const char *p1, *p2;

	if (posp < &pos[0]) {
		for (ep1 = s1; *ep1++ != '\0';);
		ep1--;
		for (ep2 = s2; *ep2++ != '\0';);
		ep2--;
		return (fcompar(s1, ep1, s2, ep2, kflags));
	}
	for (pp = &pos[0]; pp <= posp; pp++) {
		int sflags, eflags;

		if ((sflags = pp->p_sflags) == 0)
			sflags = kflags;
		if ((eflags = pp->p_eflags) == 0)
			eflags = kflags;
		p1 = fskip(s1, pp->p_sm, pp->p_sn, sflags);
		p2 = fskip(s2, pp->p_sm, pp->p_sn, sflags);
		ep1 = fskip(s1, pp->p_em, pp->p_en, eflags);
		ep2 = fskip(s2, pp->p_em, pp->p_en, eflags);
		ret = fcompar(p1, ep1, p2, ep2, sflags | eflags);
		if (ret)
			break;
	}
	return (ret);
}
