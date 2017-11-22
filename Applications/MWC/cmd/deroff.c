/*
 * Strip nroff/troff control lines and eqn
 * and tbl sequences from input.
 * Also, optionally, produce the
 * output as a set of words.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define	NLINE	500		/* Input line length */
#define	NFNEST	15		/* Depth of .so file nesting */

FILE *ofiles[NFNEST];
FILE **ofpp = &ofiles[0];
char **flist;			/* list of files to open */

typedef struct FNAME {
	struct FNAME *fn_next;
} FNAME;

#define FN_NAME(x)	((char *)((x) + 1))

FNAME *fnames;

char line[NLINE];

int delim1 = EOF;		/* Start embedded eqn */
int delim2 = EOF;		/* End embedd eqn */
int skipcnt;			/* Number of lines to skip */
int skiptitle;			/* Skip title text (until next nroff command) */
int sflag;			/* Divide into sentences */
int wflag;			/* Divide output into words */
int xflag;			/* Extra knowledge of macros (style/diction) */

int ineqn;			/* Inside embedded eqn escape */


/*
 * Open input files (for .so, .nx, and from 
 * command line).  Do not open any files twice.
 */
FILE *dopen(char *fname)
{
	register FNAME *fnp;
	register FILE *fp;

	for (fnp = fnames; fnp != NULL; fnp = fnp->fn_next)
		if (strcmp(FN_NAME(fnp), fname) == 0)
			return (NULL);
	if ((fp = fopen(fname, "r")) == NULL)
		fprintf(stderr, "deroff: cannot open `%s'\n", fname);
	else if ((fnp =
		  (FNAME *) malloc(sizeof(FNAME) + strlen(fname) + 2)) !=
		 NULL) {
		fnp->fn_next = fnames;
		fnames = fnp;
		strcpy(FN_NAME(fnp), fname);
	}
	return (fp);
}


/*
 * Get a character from the next file stream.
 */
int dgetc(void)
{
	register int c;

      again:
	if (*ofpp == NULL || (c = getc(*ofpp)) == EOF) {
		if (*ofpp != stdin && *ofpp != NULL)
			fclose(*ofpp);
		if (ofpp > ofiles) {
			ofpp--;
			goto again;
		}
		while (*flist != NULL)
			if ((*ofpp = dopen(*flist++)) != NULL)
				goto again;
		return (EOF);
	}
	return (c);
}

/*
 * Like fgets, only always reads using `dgetc'
 * into a buffer of `NLINE' characters.
 */
char *dgets(char *as)
{
	register unsigned n = NLINE;
	register char *s;
	register int c;

	s = as;
	while (--n > 0 && (c = dgetc()) != EOF)
		if ((*s++ = c) == '\n')
			break;
	*s = '\0';
	return (c == EOF && s == as ? NULL : as);
}

/*
 * Output for that line which isn't an nroff
 * control line.  This has to look for embedded
 * eqn stuff and back-slash troff/nroff escapes.
 * The embedded escapes are all handled to some degree
 * but such things as nested quotes (e.g. \w or \h)
 * do not quite work.  However, these occur almost never
 * in text so it should be sufficient.
 */
void output(char *l)
{
	register int c;
	register int inword = 0;
	register int hyphen = 0;

	if (skipcnt) {
		skipcnt--;
		return;
	}
	if (skiptitle)
		return;
	while ((c = *l++) != '\0') {
		if (ineqn) {
			if (c == delim2)
				ineqn = 0;
			continue;
		}
		if (c == delim1) {
			ineqn = 1;
			continue;
		}
		if (c == '\\') {
			if ((c = *l++) == '\0')
				break;
			switch (c) {
			case '0':	/* digit width space */
			case '|':	/* Narrow space */
			case '&':	/* Non-printing, 0-width char */
			case '!':	/* Transparent line indicator */
			case '%':	/* Optional hyphenation char */
			case 't':	/* Non-interpreted tab */
			case 'u':	/* Up 1/2 */
			case 'd':	/* Down 1/2 */
			case 'a':	/* Non-interpeted leader */
			case 'c':	/* Interrupt text processing */
			case 'p':	/* Break and spread */
			case 'r':	/* Rerverse vertical motion */
			case '{':	/* Begin conditional */
			case '}':	/* End conditional */
				c = ' ';
				break;

			case 'e':	/* Current escape */
				c = '\\';
				break;
			case '$':	/* argument */
				if (*l != '\0')
					l++;
			case '^':	/* Half narrow space */
				continue;

			case '(':	/* Char named `xx' */
				if (*l != '\0')
					l++;
				if (*l != '\0')
					l++;
				c = ' ';
				break;

			case 'z':	/* Zero-width character */
				if (*l != '\0')
					c = *l++;
				break;

			case 'k':	/* Mark input place in `x' */
			case 'n':	/* Expand reggister x */
			case '*':	/* Interpolate string */
			case 'f':	/* Change font */
				if (*l != '\0')
					if ((c = *l++) == '(') {
						if (*l != '\0')
							l++;
						if (*l != '\0')
							l++;
					}
				continue;

			case 's':	/* Change point size */
				if (*l == '\0')
					continue;
				if ((c = *l++) == '-' || c == '+') {
					if (*l == '\0')
						continue;
					c = *l++;
				}
				while (*l != '\0' && isdigit(c))
					c = *l++;
				break;

			case 'x':	/* Extra line space */
			case 'w':	/* Width function */
			case 'v':	/* Local vertical motion */
			case 'o':	/* Overstrike function */
			case 'L':	/* Vertical line */
			case 'l':	/* Horizontal line */
			case 'h':	/* Local horizontal motion */
			case 'b':	/* Bracket-builder */
				if ((c = *l) != '\0')
					while (*l != '\0' && *l != c)
						l++;
				continue;

			case '"':	/* Beginning of comment */
				while (*l != '\0')
					l++;
				continue;
			}
		}
		if (wflag) {
			if (c == '\n')
				continue;
			if (!inword)
				if (isalpha(c))
					inword = 1;
				else
					continue;
			if (c == '-' && !hyphen) {
				hyphen = 1;
				continue;
			}
			hyphen = 0;
			if (c == '\'')
				continue;
			if (!isalpha(c) && !isdigit(c)) {
				inword = 0;
				putchar('\n');
				continue;
			}
		}
		putchar(c);
	}
	if (wflag && inword && !hyphen)
		putchar('\n');
}


/*
 * Skip centred lines, in extended mode,
 * by setting a skip counter on text.
 */
void centre(char *l)
{
	if ((skipcnt = atoi(l)) == 0)
		skipcnt = 1;
}

/*
 * Skip displays in extended mode.
 */
void display(void)
{
	if (!xflag)
		return;
	while (dgets(line) != NULL) {
		if (strncmp(line, ".KE", 3) == 0)
			break;
		if (strncmp(line, ".DE", 3) == 0)
			break;
	}
}


/*
 * Process eqn directives.
 * Currently, this simply looks for
 * .EN lines as the terminator
 * and delim lines to set the eqn delimiters.
 */
void eqn(void)
{
	register char *cp;

	while (dgets(line) != NULL) {
		if (strncmp(line, ".EN", 3) == 0)
			break;
		if (strncmp(line, "delim", 5) == 0) {
			for (cp = line + 5; *cp == ' ' || *cp == '\t';
			     cp++);
			if (*cp == '\n' || *cp == '\0')
				continue;
			if (strncmp(cp, "off", 3) == 0) {
				delim1 = EOF;
				delim2 = EOF;
				continue;
			}
			delim1 = *cp++;
			delim2 = *cp;
		}
	}
}

/*
 * In extended knowledge mode (-ms macros),
 * remove footnotes.  This mode is for
 * style and diction.
 */
void footnote(void)
{
	if (!xflag)
		return;
	while (dgets(line) != NULL)
		if (strncmp(line, ".FE", 3) == 0)
			break;
}

/*
 * Include a file as per the `.so' request
 * line.
 */
void dotso(char *fname)
{
	if (++ofpp > &ofiles[NFNEST - 1]) {
		fprintf(stderr, "deroff: .so nested too deep--%s\n",
			fname);
		ofpp--;
		return;
	}
	if ((*ofpp = dopen(fname)) == NULL)
		ofpp--;
}

/*
 * Process included files.
 * The first argument is the pointer to where
 * the filename is (it may have junk before and after it)
 * The second is 's' for .so and 'n' for .nx.
 */
void include(char *fn, char type)
{
	register int c;
	register char *ep;

	while (*fn == ' ' || *fn == '\t')
		fn++;
	for (ep = fn; (c = *ep) != '\0'; ep++)
		if (c == '\n' || c == ' ' || c == '\t' || c == '\\')
			break;
	*ep = '\0';
	if (type == 's')
		dotso(fn);
	else
		*ofpp = dopen(fn);
}

/*
 * Remove a macro defintion.
 */
void macdef(void)
{
	while (dgets(line) != NULL)
		if (strcmp(line, "..\n") == 0)
			break;
}

/*
 * Throw away nofilled text as with footnotes above.
 */
void nofill(void)
{
	if (!xflag)
		return;
	while (dgets(line) != NULL)
		if (strncmp(line, ".fi", 3) == 0)
			break;
}



/*
 * Process tbl directives.  At this time,
 * all this does is look for the terminating
 * .TE to end tables.
 */
void tbl(void)
{
	while (dgets(line) != NULL)
		if (strncmp(line, ".TE", 3) == 0)
			break;
}

/*
 * If in extended mode, skip titles and author's
 * names. Set a flag to skip until next nroff command.
 */
void titles(void)
{
	if (xflag)
		skiptitle = 1;
}

/*
 * Process nroff control lines.
 * Remove EQN, TBL, macro defintions.
 * Process .so and .nx here.
 * Other lines have the rest of the line used.
 */
void nroff(char *l)
{
	skiptitle = 0;
	if (l[1] == 'E' && l[2] == 'Q')
		eqn();
	else if (l[1] == 'T' && l[2] == 'S')
		tbl();
	else if (l[1] == 'F' && l[2] == 'S')
		footnote();
	else if (l[1] == 'c' && l[2] == 'e')
		centre(l);
	else if (l[1] == 'n' && l[2] == 'f')
		nofill();
	else if (l[1] == 'D' && l[2] == 'S')
		display();
	else if (l[1] == 'K' && (l[2] == 'F' || l[2] == 'S'))
		display();
	else if (l[1] == 'T' && l[2] == 'L')
		titles();
	else if (l[1] == 'A' && (l[2] == 'I' || l[2] == 'U'))
		titles();
	else if (l[2] == 'H' && (l[1] == 'S' || l[1] == 'N'))
		titles();
	else if (l[1] == 'n' && l[2] == 'x')
		include(line + 3, 'n');
	else if (l[1] == 's' && l[2] == 'o')
		include(line + 3, 's');
	else if (l[1] == 'd' && l[2] == 'e')
		macdef();
	else if (l[1] == 'd' && l[2] == 's')
		return;
	else {
		while (*l != ' ' && *l != '\t' && *l != '\0')
			l++;
		while (*l == ' ' || *l == '\t')
			l++;
		if (*l != '\0')
			output(l);
	}
}

/*
 * Read until end-of-file
 * and process the special nroff/troff/eqn/tbl
 * lines in the file.
 */
void deroff(void)
{

	while (dgets(line) != NULL) {
		if (!ineqn && line[0] == '.') {
			nroff(line);
			continue;
		}
		output(line);
	}
}

void usage(void)
{
	fprintf(stderr, "Usage: deroff [ -w ] [ -x ] [file ...]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	register char *ap;

	while (argc > 1 && *argv[1] == '-') {
		for (ap = &argv[1][1]; *ap != '\0'; ap++)
			switch (*ap) {
			case 's':
				sflag = 1;
				break;

			case 'w':
				wflag = 1;
				break;

			case 'x':
				xflag = 1;
				break;

			default:
				usage();
			}
		argv++;
		argc--;
	}
	if (argc < 2)
		ofiles[0] = stdin;
	flist = &argv[1];
	deroff();
	exit(0);
}
