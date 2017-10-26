/*
 * Z-80 assembler.
 * Command line processing
 * and main driver.
 */
#include	"as.h"

FILE	*ifp;
FILE	*ofp;
FILE	*lfp;
char	cb[NCODE];
char	eb[NERR];
char	ib[NINPUT];
char	*cp;
char	*ep;
char	*ip;
int	lflag;
VALUE	laddr;
int	lmode;
VALUE	dot[NSEGMENT];
int	segment = CODE;
SYM	*phash[NHASH];
SYM	*uhash[NHASH];
int	pass;
int	line;
jmp_buf	env;


/*
 * Make up a file name.
 * The "sfn" is the source file
 * name and "dft" is the desired file
 * type. The finished name is copied
 * into the "dfn" buffer.
 */
void mkname(char *dfn, char *sfn, char *dft)
{
	char *p1;
	char *p2;
	int c;

	p1 = sfn;
	while (*p1 != 0)
		++p1;
	while (p1!=sfn && p1[-1]!='/')
		--p1;
	p2 = dfn;
	while ((c = *p1++)!=0 && c!='.')
		*p2++ = c;
	*p2++ = '.';
	p1 = dft;
	while ((*p2++ = *p1++) != 0);
}

int main(int argc, char *argv[])
{
	char *ifn;
	char *p;
	int i;
	int c;
	char fn[NFNAME];

	ifn = NULL;
	for (i=1; i<argc; ++i) {
		p = argv[i];
		if (*p == '-') {
			while ((c = *++p) != 0) {
				switch (c) {
				case 'l':
					++lflag;
					break;

				default:
					fprintf(stderr, "Bad option %c\n", c);
					exit(BAD);
				}
			}
		} else if (ifn == NULL)
			ifn = p;
		else {
			fprintf(stderr, "Too many source files\n");
			exit(BAD);
		}
	}
	if (ifn == NULL) {
		fprintf(stderr, "No source file\n");
		exit(BAD);
	}
	if ((ifp=fopen(ifn, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open\n", ifn);
		exit(BAD);
	}
	mkname(fn, ifn, "hex");
	if ((ofp=fopen(fn, "w")) == NULL) {
		fprintf(stderr, "%s: cannot create\n", fn);
		exit(BAD);
	}
	if (lflag != 0) {
		mkname(fn, ifn, "lis");
		if ((lfp=fopen(fn, "w")) == NULL) {
			fprintf(stderr, "%s: cannot create\n", fn);
			exit(BAD);
		}
	}
	syminit();
	for (pass=0; pass<2; ++pass) {
		outpass();
		line = 0;
		memset(dot, 0, sizeof(dot));
		fseek(ifp, 0L, 0);
		while (fgets(ib, NINPUT, ifp) != NULL) {
			++line;
			cp = &cb[0];
			ep = &eb[0];
			ip = &ib[0];
			if (setjmp(env) == 0)
				asmline();
			if (pass != 0)
				list();
		}
	}
	outeof();
	exit(GOOD);
}

