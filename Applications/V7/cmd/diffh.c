/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define C 3
#define RANGE 30
#define LEN 255
#define INF 16384

char *text[2][RANGE];
long lineno[2] = { 1, 1 };	/*no. of 1st stored line in each file */

int ntext[2];			/*number of stored lines in each */
long n0, n1;			/*scan pointer in each */
int bflag;
int debug = 0;
FILE *file[2];

void error(const char *s, const char *t)
{
	fprintf(stderr, "diffh: %s%s\n", s, t);
	exit(1);
}

void progerr(const char *s)
{
	error("program error ", s);
}


void movstr(const char *s, char *t)
{
	while ((*t++ = *s++) != 0)
		continue;
}


void range(long a, int b)
{
	if (b == INF)
		printf("%ld,$", a);
	else if (b == 0)
		printf("%ld", a);
	else
		printf("%ld,%ld", a, a + b);
}


void change(long a, int b, long c, int d, char *s)
{
	range(a, b);
	printf("%s", s);
	range(c, d);
	printf("\n");
}

/*stub for resychronization beyond limits of text buf*/
int hardsynch(void)
{
	change(n0, INF, n1, INF, "c");
	printf("---change record omitted\n");
	error("can't resynchronize", "");
	return (0);
}

	/* return pointer to line n of file f */
char *getl(int f, long n)
{
	char *t;
	int delta, nt;
      again:
	delta = n - lineno[f];
	nt = ntext[f];
	if (delta < 0)
		progerr("1");
	if (delta < nt)
		return (text[f][delta]);
	if (delta > nt)
		progerr("2");
	if (nt >= RANGE)
		progerr("3");
	if (feof(file[f]))
		return (NULL);
	t = text[f][nt];
	if (t == 0) {
		t = text[f][nt] = malloc(LEN + 1);
		if (t == NULL) {
			if (hardsynch())
				goto again;
			else
				progerr("5");
		}
	}
	t = fgets(t, LEN, file[f]);
	if (t != NULL)
		ntext[f]++;
	return (t);
}

	/*remove thru line n of file f from storage */
void clrl(int f, long n)
{
	register int i, j;
	j = n - lineno[f] + 1;
	for (i = 0; i + j < ntext[f]; i++)
		movstr(text[f][i + j], text[f][i]);
	lineno[f] = n + 1;
	ntext[f] -= j;
}


int output(int a, int b)
{
	register int i;
	char *s;
	if (a < 0)
		change(n0 - 1, 0, n1, b, "a");
	else if (b < 0)
		change(n0, a, n1 - 1, 0, "d");
	else
		change(n0, a, n1, b, "c");
	for (i = 0; i <= a; i++) {
		s = getl(0, n0 + i);
		if (s == NULL)
			break;
		printf("< %s", s);
		clrl(0, n0 + i);
	}
	n0 += i - 1;
	if (a >= 0 && b >= 0)
		printf("---\n");
	for (i = 0; i <= b; i++) {
		s = getl(1, n1 + i);
		if (s == NULL)
			break;
		printf("> %s", s);
		clrl(1, n1 + i);
	}
	n1 += i - 1;
	return (1);
}

int cmp(const char *s, const char *t)
{
	if (debug)
		printf("%s:%s\n", s, t);
	for (;;) {
		if (bflag && isspace(*s) && isspace(*t)) {
			while (isspace(*++s));
			while (isspace(*++t));
		}
		if (*s != *t || *s == 0)
			break;
		s++;
		t++;
	}
	return (*s - *t);
}

/* synch on C successive matches*/
int easysynch(void)
{
	int i, j;
	register int k, m;
	char *s0, *s1;
	for (i = j = 1; i < RANGE && j < RANGE; i++, j++) {
		s0 = getl(0, n0 + i);
		if (s0 == NULL)
			return (output(INF, INF));
		for (k = C - 1; k < j; k++) {
			for (m = 0; m < C; m++)
				if (cmp(getl(0, n0 + i - m),
					getl(1, n1 + k - m)) != 0)
					goto cont1;
			return (output(i - C, k - C));
		      cont1:;
		}
		s1 = getl(1, n1 + j);
		if (s1 == NULL)
			return (output(INF, INF));
		for (k = C - 1; k <= i; k++) {
			for (m = 0; m < C; m++)
				if (cmp(getl(0, n0 + k - m),
					getl(1, n1 + j - m)) != 0)
					goto cont2;
			return (output(k - C, j - C));
		      cont2:;
		}
	}
	return (0);
}

FILE *dopen(const char *f1, const char *f2)
{
	FILE *f;
	char b[100], *bptr;
	const char *eptr;
	struct stat statbuf;
	if (cmp(f1, "-") == 0) {
		if (cmp(f2, "-") == 0)
			error("can't do - -", "");
		else
			return (stdin);
	}
	if (stat(f1, &statbuf) == -1)
		error("can't access ", f1);

	/* FIXME: buffer length is not sanely checked, should
	   strdup and then modify the copy in situ */
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
		for (bptr = b; (*bptr = *f1++) != 0; bptr++);
		*bptr++ = '/';
		for (eptr = f2; *eptr != 0; eptr++) {
			if (*eptr == '/' && eptr[1] != 0 && eptr[1] != '/')
				f2 = eptr + 1;
		}
		while ((*bptr++ = *f2++) != 0);
		f1 = b;
	}
	f = fopen(f1, "r");
	if (f == NULL)
		error("can't open", f1);
	return (f);
}



int main(int argc, const char *argv[])
{
	char *s0, *s1;
	if (*argv[1] == '-') {
		argc--;
		argv++;
		while (*++argv[0])
			if (*argv[0] == 'b')
				bflag++;
	}
	if (argc != 3)
		error("must have 2 file arguments", "");
	file[0] = dopen(argv[1], argv[2]);
	file[1] = dopen(argv[2], argv[1]);
	for (;;) {
		s0 = getl(0, ++n0);
		s1 = getl(1, ++n1);
		if (s0 == NULL || s1 == NULL)
			break;
		if (cmp(s0, s1) != 0) {
			if (!easysynch() && !hardsynch())
				progerr("5");
		} else {
			clrl(0, n0);
			clrl(1, n1);
		}
	}
	if (s0 == NULL && s1 == NULL)
		return 0;
	if (s0 == NULL)
		output(-1, INF);
	if (s1 == NULL)
		output(INF, -1);
	return 0;
}
