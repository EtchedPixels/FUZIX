/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

/*	diff - differential file comparison
*
*	Uses an algorithm due to Harold Stone, which finds
*	a pair of longest identical subsequences in the two
*	files.
*
*	The major goal is to generate the match vector J.
*	J[i] is the index of the line in file1 corresponding
*	to line i file0. J[i] = 0 if there is no
*	such line in file1.
*
*	Lines are hashed so as to work in core. All potential
*	matches are located by sorting the lines of each file
*	on the hash (called value_____). In particular, this
*	collects the equivalence classes in file1 together.
*	Subroutine equiv____  replaces the value of each line in
*	file0 by the index of the first element of its 
*	matching equivalence in (the reordered) file1.
*	To save space equiv_____ squeezes file1 into a single
*	array member______ in which the equivalence classes
*	are simply concatenated, except that their first
*	members are flagged by changing sign.
*
*	Next the indices that point into member______ are unsorted_______   into
*	array class_____ according to the original order of file0.
*
*	The cleverness lies in routine stone______. This marches
*	through the lines of file0, developing a vector klist_____
*	of "k-candidates". At step i a k-candidate is a matched
*	pair of lines x,y (x in file0 y in file1) such that
*	there is a common subsequence of lenght k
*	between the first i lines of file0 and the first y 
*	lines of file1, but there is no such subsequence for
*	any smaller y. x is the earliest possible mate to y
*	that occurs in such a subsequence.
*
*	Whenever any of the members of the equivalence class of
*	lines in file1 matable to a line in file0 has serial number 
*	less than the y of some k-candidate, that k-candidate 
*	with the smallest such y is replaced. The new 
*	k-candidate is chained (via pred____) to the current
*	k-1 candidate so that the actual subsequence can
*	be recovered. When a member has serial number greater
*	that the y of all k-candidates, the klist is extended.
*	At the end, the longest subsequence is pulled out
*	and placed in the array J by unravel_______.
*
*	With J in hand, the matches there recorded are
*	check_____ed against reality to assure that no spurious
*	matches have crept in due to hashing. If they have,
*	they are broken, and "jackpot " is recorded--a harmless
*	matter except that a true match for a spuriously
*	mated line may now be unnecessarily reported as a change.
*
*	Much of the complexity of the program comes simply
*	from trying to minimize core utilization and
*	maximize the range of doable problems by dynamically
*	allocating what is needed and reusing what is not.
*	The core requirements for problems larger than somewhat
*	are (in words) 2*length(file0) + length(file1) +
*	3*(number of k-candidates installed),  typically about
*	6n words for files of length n. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define	prints(s)	fputs(s,stdout)

#define HALFLONG 16
#define low(x)	(x&((1L<<HALFLONG)-1))
#define high(x)	(x>>HALFLONG)
FILE *input[2];

struct cand {
	int x;
	int y;
	int pred;
} cand;
struct line {
	int serial;
	int value;
} *file[2], line;
int len[2];
struct line *sfile[2];		/*shortened by pruning common prefix and suffix */
int slen[2];
int pref, suff;			/*length of prefix and suffix */
int *class;			/*will be overlaid on file[0] */
int *member;			/*will be overlaid on file[1] */
int *klist;			/*will be overlaid on file[0] after class */
struct cand *clist;		/* merely a free storage pot for candidates */
int clen = 0;
int *J;				/*will be overlaid on class */
long *ixold;			/*will be overlaid on klist */
long *ixnew;			/*will be overlaid on file[1] */
int opt;			/* -1,0,1 = -e,normal,-f */
int status = 2;
int anychange = 0;
char *empty = "";
int bflag;
char buf[512];

char *tempfile;			/*used when comparing against std input */
char *dummy;			/*used in resetting storage search ptr */

void done(void)
{
	unlink(tempfile);
	exit(status);
}

void sdone(int sig)
{
	done();
}

void mesg(const char *s, const char *t)
{
	fprintf(stderr, "diff: %s%s\n", s, t);
}

void noroom(void)
{
	mesg("files too big, try -h\n", empty);
	done();
}

char *talloc(size_t n)
{
	register char *p;
	p = malloc(n);
	if (p == NULL)
		noroom();
	return (p);
}

char *ralloc(char *p, size_t n)
{
	char *q = realloc(p, n);
	if (q == NULL)
		noroom();
	return (q);
}

void sort(struct line *a, int n)
{				/*shellsort CACM #201 */
	struct line w;
	register int j, m;
	struct line *ai;
	register struct line *aim;
	int k;
	for (j = 1; j <= n; j *= 2)
		m = 2 * j - 1;
	for (m /= 2; m != 0; m /= 2) {
		k = n - m;
		for (j = 1; j <= k; j++) {
			for (ai = &a[j]; ai > a; ai -= m) {
				aim = &ai[m];
				if (aim < ai)
					break;	/*wraparound */
				if (aim->value > ai[0].value ||
				   (aim->value == ai[0].value &&
				    aim->serial > ai[0].serial))
					break;
				w.value = ai[0].value;
				ai[0].value = aim->value;
				aim->value = w.value;
				w.serial = ai[0].serial;
				ai[0].serial = aim->serial;
				aim->serial = w.serial;
			}
		}
	}
}

void unsort(struct line *f, int l, int *b)
{
	register int *a;
	register int i;
	a = (int *) talloc((l + 1) * sizeof(int));
	for (i = 1; i <= l; i++)
		a[f[i].serial] = f[i].value;
	for (i = 1; i <= l; i++)
		b[i] = a[i];
	free((char *) a);
}

int skipline(int f)
{
	register int i;
	for (i = 1; getc(input[f]) != '\n'; i++);
	return (i);
}

void filename(char **pa1, char **pa2)
{
	register char *a1, *b1, *a2;
	struct stat stbuf;
	int i, f;
	a1 = *pa1;
	a2 = *pa2;
	if (stat(a1, &stbuf) != -1
	    && ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
		b1 = *pa1 = talloc(100);
		while ((*b1++ = *a1++) != 0);
		b1[-1] = '/';
		a1 = b1;
		while ((*a1++ = *a2++) != 0)
			if (*a2 && *a2 != '/' && a2[-1] == '/')
				a1 = b1;
	} else if (a1[0] == '-' && a1[1] == 0 && tempfile == 0) {
		signal(SIGHUP, sdone);
		signal(SIGINT, sdone);
		signal(SIGPIPE, sdone);
		signal(SIGTERM, sdone);
		*pa1 = tempfile = tmpnam("diffXXXXX");
		if ((f =
		     open(tempfile, O_CREAT | O_EXCL | O_WRONLY,
			  0600)) < 0) {
			mesg("cannot create ", tempfile);
			done();
		}
		while ((i = read(0, buf, 512)) > 0)
			write(f, buf, i);
		close(f);
	}
}

/* hashing has the effect of
 * arranging line in 7-bit bytes and then
 * summing 1-s complement in 16-bit hunks 
*/

int readhash(FILE * f)
{
	long sum;
	register unsigned shift;
	register int space;
	register int t;
	sum = 1;
	space = 0;
	if (!bflag)
		for (shift = 0; (t = getc(f)) != '\n'; shift += 7) {
			if (t == -1)
				return (0);
			sum += (long) t << (shift %= HALFLONG);
	} else
		for (shift = 0;;) {
			switch (t = getc(f)) {
			case -1:
				return (0);
			case '\t':
			case ' ':
				space++;
				continue;
			default:
				if (space) {
					shift += 7;
					space = 0;
				}
				sum += (long) t << (shift %= HALFLONG);
				shift += 7;
				continue;
			case '\n':
				break;
			}
			break;
		}
	sum = low(sum) + high(sum);
	return ((short) low(sum) + (short) high(sum));
}

void prepare(int i, const char *arg)
{
	register struct line *p;
	register int j, h;
	if ((input[i] = fopen(arg, "r")) == NULL) {
		mesg("cannot open ", arg);
		done();
	}
	p = (struct line *) talloc(3 * sizeof(line));
	for (j = 0; (h = readhash(input[i])) != 0;) {
		p = (struct line *) ralloc((char *) p,
					   (++j + 3) * sizeof(line));
		p[j].value = h;
	}
	len[i] = j;
	file[i] = p;
	fclose(input[i]);
}

void prune(void)
{
	register int i, j;
	for (pref = 0; pref < len[0] && pref < len[1] &&
	     file[0][pref + 1].value == file[1][pref + 1].value; pref++);
	for (suff = 0; suff < len[0] - pref && suff < len[1] - pref &&
	     file[0][len[0] - suff].value == file[1][len[1] - suff].value;
	     suff++);
	for (j = 0; j < 2; j++) {
		sfile[j] = file[j] + pref;
		slen[j] = len[j] - pref - suff;
		for (i = 0; i <= slen[j]; i++)
			sfile[j][i].serial = i;
	}
}

void equiv(struct line *a, int n, struct line *b, int m, int *c)
{
	register int i, j;
	i = j = 1;
	while (i <= n && j <= m) {
		if (a[i].value < b[j].value)
			a[i++].value = 0;
		else if (a[i].value == b[j].value)
			a[i++].value = j;
		else
			j++;
	}
	while (i <= n)
		a[i++].value = 0;
	b[m + 1].value = 0;
	j = 0;
	while (++j <= m) {
		c[j] = -b[j].serial;
		while (b[j + 1].value == b[j].value) {
			j++;
			c[j] = b[j].serial;
		}
	}
	c[j] = -1;
}

int newcand(int x, int y, int pred)
{
	register struct cand *q;
	clist =
	    (struct cand *) ralloc((char *) clist, ++clen * sizeof(cand));
	q = clist + clen - 1;
	q->x = x;
	q->y = y;
	q->pred = pred;
	return (clen - 1);
}

int search(int *c, int k, int y)
{
	register int i, j, l;
	int t;
	if (clist[c[k]].y < y)	/*quick look for typical case */
		return (k + 1);
	i = 0;
	j = k + 1;
	while ((l = (i + j) / 2) > i) {
		t = clist[c[l]].y;
		if (t > y)
			j = l;
		else if (t < y)
			i = l;
		else
			return (l);
	}
	return (l + 1);
}

void unravel(int p)
{
	register int i;
	register struct cand *q;
	for (i = 0; i <= len[0]; i++)
		J[i] = i <= pref ? i :
		    i > len[0] - suff ? i + len[1] - len[0] : 0;
	for (q = clist + p; q->y != 0; q = clist + q->pred)
		J[q->x + pref] = q->y + pref;
}


int stone(int *a, int n, int *b, int *c)
{
	register int i, k, y;
	int j, l;
	int oldc, tc;
	int oldl;
	k = 0;
	c[0] = newcand(0, 0, 0);
	for (i = 1; i <= n; i++) {
		j = a[i];
		if (j == 0)
			continue;
		y = -b[j];
		oldl = 0;
		oldc = c[0];
		do {
			if (y <= clist[oldc].y)
				continue;
			l = search(c, k, y);
			if (l != oldl + 1)
				oldc = c[l - 1];
			if (l <= k) {
				if (clist[c[l]].y <= y)
					continue;
				tc = c[l];
				c[l] = newcand(i, y, oldc);
				oldc = tc;
				oldl = l;
			} else {
				c[l] = newcand(i, y, oldc);
				k++;
				break;
			}
		} while ((y = b[++j]) > 0);
	}
	return (k);
}

/* check does double duty:
1.  ferret out any fortuitous correspondences due
to confounding by hashing (which result in "jackpot")
2.  collect random access indexes to the two files */

void check(char *argv[])
{
	register int i, j;
	int jackpot;
	long ctold, ctnew;
	char c, d;
	input[0] = fopen(argv[1], "r");
	input[1] = fopen(argv[2], "r");
	j = 1;
	ixold[0] = ixnew[0] = 0;
	jackpot = 0;
	ctold = ctnew = 0;
	for (i = 1; i <= len[0]; i++) {
		if (J[i] == 0) {
			ixold[i] = ctold += skipline(0);
			continue;
		}
		while (j < J[i]) {
			ixnew[j] = ctnew += skipline(1);
			j++;
		}
		for (;;) {
			c = getc(input[0]);
			d = getc(input[1]);
			ctold++;
			ctnew++;
			if (bflag && isspace(c) && isspace(d)) {
				do {
					if (c == '\n')
						break;
					ctold++;
				} while (isspace(c = getc(input[0])));
				do {
					if (d == '\n')
						break;
					ctnew++;
				} while (isspace(d = getc(input[1])));
			}
			if (c != d) {
				jackpot++;
				J[i] = 0;
				if (c != '\n')
					ctold += skipline(0);
				if (d != '\n')
					ctnew += skipline(1);
				break;
			}
			if (c == '\n')
				break;
		}
		ixold[i] = ctold;
		ixnew[j] = ctnew;
		j++;
	}
	for (; j <= len[1]; j++) {
		ixnew[j] = ctnew += skipline(1);
	}
	fclose(input[0]);
	fclose(input[1]);
/*
	if(jackpot)
		mesg("jackpot",empty);
*/
}

void range(int a, int b, char *separator)
{
	printf("%d", a > b ? b : a);
	if (a < b) {
		printf("%s%d", separator, b);
	}
}

void fetch(long *f, int a, int b, FILE * lb, char *s)
{
	register int i, j;
	register int nc;
	for (i = a; i <= b; i++) {
		fseek(lb, f[i - 1], 0);
		nc = f[i] - f[i - 1];
		prints(s);
		for (j = 0; j < nc; j++)
			putchar(getc(lb));
	}
}

void change(int a, int b, int c, int d)
{
	if (a > b && c > d)
		return;
	anychange = 1;
	if (opt != 1) {
		range(a, b, ",");
		putchar(a > b ? 'a' : c > d ? 'd' : 'c');
		if (opt != -1)
			range(c, d, ",");
	} else {
		putchar(a > b ? 'a' : c > d ? 'd' : 'c');
		range(a, b, " ");
	}
	putchar('\n');
	if (opt == 0) {
		fetch(ixold, a, b, input[0], "< ");
		if (a <= b && c <= d)
			prints("---\n");
	}
	fetch(ixnew, c, d, input[1], opt == 0 ? "> " : empty);
	if (opt != 0 && c <= d)
		prints(".\n");
}

void output(char *argv[])
{
	int m;
	register int i0, i1, j1;
	int j0;
	input[0] = fopen(argv[1], "r");
	input[1] = fopen(argv[2], "r");
	m = len[0];
	J[0] = 0;
	J[m + 1] = len[1] + 1;
	if (opt != -1)
		for (i0 = 1; i0 <= m; i0 = i1 + 1) {
			while (i0 <= m && J[i0] == J[i0 - 1] + 1)
				i0++;
			j0 = J[i0 - 1] + 1;
			i1 = i0 - 1;
			while (i1 < m && J[i1 + 1] == 0)
				i1++;
			j1 = J[i1 + 1] - 1;
			J[i1] = j1;
			change(i0, i1, j0, j1);
	} else
		for (i0 = m; i0 >= 1; i0 = i1 - 1) {
			while (i0 >= 1 && J[i0] == J[i0 + 1] - 1
			       && J[i0] != 0)
				i0--;
			j0 = J[i0 + 1] - 1;
			i1 = i0 + 1;
			while (i1 > 1 && J[i1 - 1] == 0)
				i1--;
			j1 = J[i1 - 1] + 1;
			J[i1] = j1;
			change(i1, i0, j1, j0);
		}
	if (m == 0)
		change(1, 0, 1, len[1]);
}

int main(int argc,  char *argv[])
{
	register int k;
	char **args;

	args = argv;
	if (argc > 3 && *argv[1] == '-') {
		argc--;
		argv++;
		for (k = 1; argv[0][k]; k++) {
			switch (argv[0][k]) {
			case 'e':
				opt = -1;
				break;
			case 'f':
				opt = 1;
				break;
			case 'b':
				bflag = 1;
				break;
			case 'h':
				execv("/usr/lib/diffh", (char**)args);
				mesg("cannot find diffh", empty);
				done();
			}
		}
	}
	if (argc != 3) {
		mesg("arg count", empty);
		done();
	}

	filename(&argv[1], &argv[2]);
	filename(&argv[2], &argv[1]);
	prepare(0, argv[1]);
	prepare(1, argv[2]);
	prune();
	sort(sfile[0], slen[0]);
	sort(sfile[1], slen[1]);

	member = (int *) file[1];
	equiv(sfile[0], slen[0], sfile[1], slen[1], member);
	member =
	    (int *) ralloc((char *) member, (slen[1] + 2) * sizeof(int));

	class = (int *) file[0];
	unsort(sfile[0], slen[0], class);
	class =
	    (int *) ralloc((char *) class, (slen[0] + 2) * sizeof(int));

	klist = (int *) talloc((slen[0] + 2) * sizeof(int));
	clist = (struct cand *) talloc(sizeof(cand));
	k = stone(class, slen[0], member, klist);
	free((char *) member);
	free((char *) class);

	J = (int *) talloc((len[0] + 2) * sizeof(int));
	unravel(klist[k]);
	free((char *) clist);
	free((char *) klist);

	ixold = (long *) talloc((len[0] + 2) * sizeof(long));
	ixnew = (long *) talloc((len[1] + 2) * sizeof(long));
	check(argv);
	output(argv);
	status = anychange;
	done();
}
