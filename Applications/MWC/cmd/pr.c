/*
 * pr.c
 * Print files.
 * 7/5/94
 * All references to page length exclude possible margin lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <err.h>
#include <sys/types.h>
#include <errno.h>

#define	LSIZE	256		/* line size */
#define	LENGTH	(66-MARGIN)	/* default page length */
#define	WIDTH	80		/* default page width */
#define	TMAR	5		/* five lines of top margin */
#define	BMAR	5		/* five lines of bottom margin */
#define	HEADER	2		/* line # where header appears */

#define	MARGIN	(TMAR+BMAR)

#define	USAGE	"Usage: pr [ options ] [ file ...]\n"\
"Options:\n"\
"\t+skip\tSkip the first skip pages of input before printing\n"\
"\t-cols\tPrint the input in cols columns\n"\
"\t-h\tThe next argument is the header (replaces file name)\n"\
"\t-ln\tSet page size to n lines (default, 66)\n"\
"\t-m\tPrint each input file in a separate column\n"\
"\t-n\tNumber the output lines\n"\
"\t-sc\tSeparate each column with character c\n"\
"\t-t\tSuppress top and bottom margins and header\n"\
"\t-wn\tPage width is set to n columns (default, 80)\n"\
"A file named `-' means stdin.\n"

/*
 * info on input file streams
 * Unless -m is in effect, only f[0] is used.
 */
struct f {
	int f_ff;		/* '\f' received */
	FILE *f_stream;		/* input stream */
};


int ncol = 1,			/* # of columns */
    nskip,			/* # pages to skip of each file */
    length = LENGTH,		/* page length */
    width = WIDTH,		/* page width */
    lno,			/* current input line # */
    fwidth;			/* field width */
char schar,			/* separator char */
 tflag,				/* -t: no header or margins */
 mflag,				/* -m: multiple file output */
 nflag,				/* -n: line number output */
*date,				/* date string */
*header,			/* -h: header text */
**lines;			/* buffer addr for multicolumn */

struct f f[20];			/* input file info */

extern int page1(void (*putline) (char *, int));
extern int page2(void (*putline) (char *, int));

int (*page) (void (*fn) (char *, int)) = (void *)page1;

/*
 * read a line
 * Simple char processing is done, including tab expansion.  getline( )
 * will return an empty line if f_ff is set.  Lines are truncated to
 * fit the field width.
 */
int getline(struct f *fp, char *lbuf)
{
	int col;
	char *p;
	int c;

	col = 0;
	p = lbuf;
	if (feof(fp->f_stream) == 0 && fp->f_ff == 0) {
		if (nflag) {
			sprintf(p, "%4d: ", ++lno);
			p += 6;
		}
		for (;;) {
			switch (c = getc(fp->f_stream)) {
			case '\f':
				++fp->f_ff;
			case EOF:
				if (nflag && (p == &lbuf[6]))
					p = lbuf;
				break;
			case '\n':
				break;
			case '\r':
				continue;
			case '\t':
				do {
					if (p < &lbuf[LSIZE - 1]
					    && col < fwidth)
						*p++ = ' ';
				} while (++col & 7);
				continue;
			case '\b':
				if (col) {
					--col;
					if (p < &lbuf[LSIZE - 1]
					    && col < fwidth)
						*p++ = c;
				}
				continue;
			default:
				if (p < &lbuf[LSIZE - 1] && col < fwidth)
					*p++ = c;
				++col;
				continue;
			}
			break;
		}

	}
	*p = '\0';
	return (p - lbuf);
}


/*
 * write a line, incrementally
 * A line of output can be built by successive calls to putl( ).  A line
 * address of 0 puts the newline.  Simple char processing is done,
 * including tab optimization.  Each line segment is padded to the
 * field width, unless a there is a field separator char.
 */
void putl(char *lbuf, int schr)
{
	char *p;
	int c, nextxcol;
	static int col, xcol;

	if ((p = lbuf) == NULL) {
		col = 0;
		xcol = 0;
		putchar('\n');
		return;
	}
	nextxcol = xcol + fwidth;

	for (;;) {
		if ((c = *p++) == '\0') {
			if ((c = schr) == '\0')
				break;
			schr = 0;
			--p;
			nextxcol = xcol + 1;
		}
		if (c == ' ') {
			++xcol;
			continue;
		}
		if (c == '\b') {
			--xcol;
			continue;
		}
		while ((col | 7) + 1 <= xcol) {
			col = (col | 7) + 1;
			putchar('\t');
		}
		while (col < xcol) {
			++col;
			putchar(' ');
		}
		while (col > xcol) {
			--col;
			putchar('\b');
		}
		putchar(c);
		xcol = ++col;
	}

	xcol = nextxcol;
}


/*
 * throw away output line
 * Used when skipping the first pages of input.
 */
void nop(char *lbuf, int schr)
{

}

/*
 * print from open input streams
 * Control output of pages, perhaps provide header/footer margins and title.
 * The paging routine is expected to give an eof warning on the last page.
 */
void print(char *file)
{
	int i, pg, eof;

	for (pg = 1; pg <= nskip; ++pg)
		if ((*page) (nop))
			return;

	do {
		if (tflag == 0)
			for (i = 0; i < TMAR; ++i) {
				if (i != HEADER) {
					putchar('\n');
					continue;
				}
				printf
				    ("%.12s%.5s  %s  Page %d, line %ld\n",
				     date + 4, date + 19,
				     header ? header : file, pg,
				     (long) (pg -
					     1) * (mflag ? 1 : ncol) *
				     length + 1);
			}
		eof = (*page) (putl);
		if (tflag == 0)
			for (i = 0; i < BMAR; ++i)
				putchar('\n');
	} while (++pg, eof == 0);
}


/*
 * open input file
 * Failure to open is fatal.  openf( "-") returns stdin.
 */
FILE *openf(char *file)
{
	FILE *stream;

	lno = 0;
	if (file[0] == '-' && file[1] == '\0')
		return (stdin);
	if ((stream = fopen(file, "r")) == NULL)
		if (errno == EMFILE)
			errx(1, "too many files for -m");
		else
			err(1, "cannot open \"%s\"", file);
	return (stream);
}

/*
 * page one stream per column
 * This handles all cases of one column per page.
 * Formfeed advances output of that stream to the next page.
 * Eof is only indicated when all streams give this condition.
 */
static char lbuf[LSIZE];

int page1(void (*putline) (char *, int))
{
	int i, j;

	for (i = 0; i < length; ++i) {
		for (j = 0; j < ncol; ++j) {
			getline(&f[j], lbuf);
			(*putline) (lbuf, j < ncol - 1 ? schar : 0);
		}
		(*putline) ((char *) 0, 0);
	}

	i = 0;
	for (j = 0; j < ncol; ++j) {
		f[j].f_ff = 0;
		if (feof(f[j].f_stream))
			++i;
		else if (ungetc(getc(f[j].f_stream), f[j].f_stream) == EOF)
			++i;
	}
	return (i == j);
}


/*
 * page one stream across multiple columns
 * The first `ncol'-1 columns of text are buffered; the last is simply
 * read as needed.  Formfeeds advance output to the next column.
 */

int page2(void (*putline) (char *, int))
{
	int i, j, k;

	for (i = 0; i < (ncol - 1) * length; ++i) {
		if ((i % length) == 0)
			f[0].f_ff = 0;
		if (k = getline(&f[0], lbuf)) {
			if ((lines[i] = malloc(k + 1)) == NULL)
				errx(1, "out of space");
			strcpy(lines[i], lbuf);
		} else
			lines[i] = NULL;
	}
	f[0].f_ff = 0;

	for (i = 0; i < length; ++i) {
		for (j = 0; j < ncol - 1; ++j) {
			k = j * length + i;
			if (lines[k]) {
				(*putline) (lines[k], schar);
				free(lines[k]);
			} else
				(*putline) ("", schar);
		}
		getline(&f[0], lbuf);
		(*putline) (lbuf, 0);
		(*putline) ((char *) 0, 0);
	}

	if (feof(f[0].f_stream))
		return (1);
	if (ungetc(getc(f[0].f_stream), f[0].f_stream) == EOF)
		return (1);
	return (0);
}


/*
 * initialize & get options
 * Flags are recognized up to the first file name.  If multi-column (-N),
 * allocate line array.  If printing multiple files (-m), open all files.
 * There are two paging algorithms: one file per column (page1), and
 * many columns per file (page2).  The latter requires page buffering.
 * init( ) makes this selection.
 */
char **init(int ac, char *av[])
{
	int mar = MARGIN;
	static char obuf[BUFSIZ];
	time_t tvec;

	setbuf(stdout, obuf);

	while (++av, --ac) {
		if (av[0][0] == '+')
			if ((nskip = atoi(&av[0][1])) <= 0)
				errx(1, "bad skip");
			else
				continue;
		if (av[0][0] != '-')
			break;
		switch (av[0][1]) {
		case '\0':
			break;
		case 'l':
			length = atoi(&av[0][2]) - mar;
			continue;
		case 'w':
			width = atoi(&av[0][2]);
			continue;
		case 'h':
			if (av[0][2])
				header = &av[0][2];
			else {
				if (--ac <= 0)
					errx(1, "missing header arg");
				header = (++av)[0];
			}
			continue;
		case 's':
			if ((schar = av[0][2]) == '\0')
				schar = '\t';
			continue;
		case 'm':
			++mflag;
			continue;
		case 't':
			++tflag;
			length += mar;
			mar = 0;
			continue;
		case 'n':
			++nflag;
			continue;
		default:
			/* FIXME: trap ncol < 1 */
			if ('0' <= av[0][1] && av[0][1] <= '9')
				ncol = atoi(&av[0][1]);
			else {
				fprintf(stderr, USAGE);
				exit(1);
			}
			continue;
		}
		break;

	}

	f[0].f_stream = stdin;
	if (mflag && av[0]) {
		ncol = 0;
		do {
			f[ncol++].f_stream = openf((av++)[0]);
		} while (av[0]);
	}

	/*
	 * check that all options jive
	 */
	if (length <= 0)
		if (length == -mar) {	/* gunja artifice */
			length = 1;
			++tflag;
		} else
			errx(1, "length too small");
	fwidth = width / ncol;
	if (schar)
		--fwidth;
	if (fwidth <= 0 || (nflag && fwidth < 10))
		errx(1, "width too small");
	if (fwidth >= LSIZE - 1)
		errx(1, "too wide");
	/* FIXME: maths overflow */
	if (ncol > 1 && mflag == 0) {
		if ((lines =
		     malloc((ncol - 1) * length * sizeof(char *))) == NULL)
			errx(1, "insufficient core");
		page = (void *)page2;	/* Hack cast for SDCC */
	}

	time(&tvec);
	date = ctime(&tvec);

	return (av);
}


/*
 * paginate files to standard output
 * If no files are given, use standard input.  The file name "-" also
 * means standard input.
 */
int main(int argc, char *argv[])
{
	argv = init(argc, argv);

	if (*argv)
		while (*argv) {
			f[0].f_stream = openf(*argv);
			print(*argv++);
			fclose(f[0].f_stream);
	} else
		print("");

	return (0);
}

/* end of pr.c */
