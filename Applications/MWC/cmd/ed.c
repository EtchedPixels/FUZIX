/*
 * An editor.
 * External variables.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>

#include "ed.h"

int	cflag;				/* Print character counts */
int	mflag;				/* Allow multiple commands per line */
int	pflag;				/* Editor prompts */
int	oflag;				/* Behaves like old editor */
int	sflag;				/* Match patterns in single case */
int	tflag;				/* Set up for screen editor */
int	vflag;				/* Verbose error messages */
int	intflag;			/* Interrupt has been hit */
FILE	*tmp;				/* Temp file pointer */
long	tmpseek;			/* Free space seek ptr in tmp file */
int	rcurbno;			/* Current read block number */
int	wcurbno;			/* Current write block number */
LINE	*line;				/* Pointer to line table */
int	lnsize;				/* Current size of line table */
int	savechr;			/* Character that was ungetx'ed */
int	lastchr;			/* Last character we read */
char	*gcp;				/* Global input pointer */
char	linebuf[LBSIZE];		/* Line buffer */
char	codebuf[CBSIZE];		/* Code buffer */
char	tempbuf[TBSIZE];		/* Temporary buffer */
char	subsbuf[SBSIZE];		/* Substitute buffer */
char	globbuf[GBSIZE];		/* Buffer for global command */
char	rdbcbuf[DBSIZE];		/* Disk buffer cache */
char	wdbcbuf[DBSIZE];		/* Write buffer cache */
BRACE	brace[1+BRSIZE];		/* For remembering \( \) */
char	file[FNSIZE+1];			/* Filename */
LINE	marklin[MKSIZE];		/* Mark line table */
int	dotadd;				/* Address of the current line */
int	doladd;				/* Address of last line */
char	vcom;				/* Verify command */
int	saved;				/* File saved since last written */
FILE	*fp;				/* File pointer for readfil */
long	cct;				/* Number of chars read in append */
long	lct;				/* Number of lines read in append */
int	appflag;			/* In append mode */
int	addspec;			/* Number of addresses specified */
int	adderrr;			/* Error in computing address */
int	addpage;			/* An ampersand was found */
int	addques;			/* A question mark was found */
int	subnths;			/* Which substitute wanted */
int	subnewl;			/* A newline is being replaced */
int	subseek;			/* Seek position of new line */
int	suborig;			/* Seek position of old line */
char	*errstr;			/* Pointer to last error message */
char	*keyp;				/* Pointer to crypt key */

/*
 * Main, command parser and general routines.
 */
int main(int argc, char *argv[])
{
	char *cp;
	initialise();

	if ((cp=getenv("ED")) != NULL)
		setoptf(cp);
	signal(SIGHUP, sighang);
	signal(SIGINT, sigintr);
	signal(SIGQUIT, sigintr);

	setup(argc, argv);
	for (;;) {
		if (intflag) {
/*
			finit(stdout);
			clearerr(stdin);
			clearerr(stdout);
			clearerr(stderr);
*/
			savechr = '\0';
			intflag = 0;
			derror("Interrupt");
		}
		command();
	}
}

/*
 * Initialisation.
 */
void initialise(void)
{
	int i;
	char *bp;

	tmp = tmpfile();
	if (tmp == NULL) {
		terror("Cannot create temp file");
		exit(1);
	}

	mflag = 0;
	oflag = 0;
	pflag = 0;
	sflag = 0;
	tflag = 0;
	vflag = 0;
	cflag = 1;
	intflag = 0;
	tmpseek = CLSIZE;
	rcurbno = -1;
	wcurbno = -1;
	lnsize = LNSIZE;
	if ((line=(LINE *)calloc(lnsize, sizeof(LINE))) == NULL) {
		prints(stderr, "Cannot allocate line number table\n");
		exit(1);
	}
	savechr = '\0';
	lastchr = '\n';
	gcp = NULL;
	codebuf[0] = CSNUL;
	file[0] = '\0';
	for (i=0; i<MKSIZE; i++)
		marklin[i] = 0;
	dotadd = 0;
	doladd = 0;
	saved = 1;
	subseek = 0;
	keyp = NULL;
}

/*
 * Process command line arguments.
 */
void setup(int argc, char *argv[])
{
	char *cp;
	int i;
	int f;

	f = 0;
	for (i=1; i<argc; i++) {
		cp = argv[i];
		switch (*cp++) {
		case '-':
			switch (*cp++) {
			case '\0':
				cflag = 0;
				continue;
			case 't':
				tflag = 1;
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				continue;
			case 'u':
				setbuf(stdout, NULL);
				continue;
			case 'x':
				setkey();
				continue;
			default:
				usage();
			}
		case '+':
			setoptf(argv[i]);
			continue;
		default:
			if (f++ != 0)
				usage();
			if (strlen(argv[i]) > FNSIZE)
				derror("File name too long");
			else {
				strcpy(file, argv[i]);
				edit();
			}
		}
	}
}

/*
 * Print out a usage message.
 */
void usage(void)
{
	prints(stderr, "Usage: ed [-[x]] [+cmopsv] [file]\n");
	exit(1);
}

/*
 * Set a flag indicating that interrupt has been hit.
 * Used for SIGQUIT and SIGINT.
 */

void sigintr(int s)
{
	intflag++;
	signal(s, sigintr);
}

/*
 * If we get a hangup signal, write the file onto the file
 * `ed.hup' and leave.
 */

void sighang(int s)
{
	if (doladd != 0)
		wfile(1, doladd, "ed.hup", "w", 1);
	leave();
}

/*
 * Leave the editor.
 */
void leave(void)
{
	fclose(tmp);
	exit(0);
}

/*
 * Process a command.
 */
int command(void)
{
	int c1, n, a3;
	char name[FNSIZE+1];
	int a1, a2, c;

	for (;;) {
		addspec = 0;
		a1 = a2 = getaddr();
		if (intflag)
			return (0);
		if (adderrr)
			goto err;
		c = getx();
		if (addspec != 0) {
			while (c==',' || c==';') {
				a1 = a2;
				if (c == ';')
					dotadd = a1;
				a2 = getaddr();
				if (adderrr)
					goto err;
				if (addspec == 0) {
					derror("Missing address");
					goto err;
				}
				c = getx();
			}
		}
		if (c == '*') {
			if (addspec++ == 0) {
				a1 = 1;
				addspec++;
			}
			a2 = doladd;
			c = getx();
		}
		switch (c) {
		case EOF:
			if (intflag == 0) {
				if (saved != 0)
					leave();
				saved = 1;
				derror("File not saved");
			}
			return 0;
		case '\n':
			if (addques) {
				if (errstr) {
					prints(stdout, errstr);
					printc(stdout, '\n');
				}
				return (0);
			}
			if (addpage) {
				a1 = a2;
				a2 = a1 + PGSIZE;
				if (a2 > doladd)
					a2 = doladd;
				if (print(a1, a2) == 0)
					return (0);
				return (1);
			}
			if (addspec == 0)
				a2 = dotadd+1;
			if (print(a2, a2) == 0)
				return (0);
			return (1);
		case '!':
			if (rest() == 0)
				goto err;
			n = system(tempbuf);
			if (n < 0) {
				derror("Call failed");
				return (0);
			} else {
				prints(stdout, "!\n");
				return (1);
			}
		case '=':
			if (verify(0) == 0)
				goto err;
			if (addspec == 0)
				a2 = doladd;
			printd(stdout, a2);
			printc(stdout, '\n');
			break;
		case 'a':
			if (verify(1) == 0)
				goto err;
			getx();
			if (append(a2, readtty) == 0)
				return (0);
			ungetx('\n');
			break;
		case 'c':
			if (verify(1) == 0)
				goto err;
			getx();
			if (delete(a1, a2) == 0)
				return (0);
			if (append(a1-1, readtty) == 0)
				return (0);
			ungetx('\n');
			break;
		case 'd':
			if (verify(0) == 0)
				goto err;
			if (delete(a1, a2) == 0)
				goto err;
			break;
		case 'e':
			n = saved;
			if (getfile(n?file:name) == 0)
				return (0);
			if (n == 0) {
				saved = 1;
				derror("File not saved");
				return (0);
			}
			return (edit());
		case 'E':
			if (getfile(file) == 0)
				return (0);
			return (edit());
		case 'f':
			if ((c=getx()) == ' ') {
				ungetx(' ');
				getfile(file);
				c = '\n';
			}
			if (c != '\n') {
				derror("Bad command");
				goto err;
			}
			if (file[0] != '\0') {
				prints(stdout, file);
				printc(stdout, '\n');
			}
			return (1);
		case 'g':
			if (addspec == 0) {
				a1 = 1;
				a2 = doladd;
			}
			if (global(a1, a2, 0) == 0)
				goto err;
			if (verify(1) == 0)
				goto err;
			break;
		case 'i':
			if (verify(1) == 0)
				goto err;
			getx();
			if (doladd==0 && a2==0)
				a2 = 1;
			if (append(a2-1, readtty) == 0)
				return (0);
			ungetx('\n');
			break;
		case 'j':
			if (addspec < 2)
				a2 = a1+1;
			if (verify(0) == 0)
				goto err;
			if (join(a1, a2) == 0)
				goto err;
			break;
		case 'k':
			if ((c=getx()) == '\n') {
				derror("No mark name specified");
				return (0);
			}
			if (verify(0) == 0)
				goto err;
			if (a2<1 || a2>doladd) {
				derror("Illegal address range");
				goto err;
			}
			if (!isascii(c) || !islower(c)) {
				derror("Bad mark name");
				goto err;
			}
			marklin[c-'a'] = line[a2]|1;
			break;
		case 'l':
			if (verify(0) == 0)
				goto err;
			if (list(a1, a2) == 0)
				goto err;
			break;
		case 'm':
			a3 = getaddr();
			if (adderrr)
				goto err;
			if (verify(0) == 0)
				goto err;
			if (move(a1, a2, a3) == 0)
				goto err;
			break;
		case 'o':
			if (rest() == 0)
				goto err;
			if (tempbuf[0] == '\0')
				disoptf();
			else
				setoptf(tempbuf);
			return (1);
		case 'p':
		case 'P':
			if (verify(0) == 0)
				goto err;
			if (print(a1, a2) == 0)
				goto err;
			break;
		case 'q':
			if (verify(1) == 0)
				goto err;
			if (addspec != 0) {
				derror("Cannot specify address");
				goto err;
			}
			if (saved == 0) {
				saved = 1;
				derror("File not saved");
				goto err;
			}
			leave();
		case 'Q':
			if (verify(1) == 0)
				goto err;
			leave();
		case 'r':
			if (getfile(name) == 0)
				return (0);
			if (addspec == 0)
				a2 = doladd;
			if ((fp=xfopen(name, "r")) == NULL) {
				derror("Cannot open file");
				return (0);
			}
			n = 1;
			if (append(a2, readfil) == 0)
				n = 0;
			else if (ferror(fp)) {
				derror("Read error");
				n = 0;
			}
			fclose(fp);
			if (cflag != 0) {
				printl(stdout, !oflag ? cct : lct);
				printc(stdout, '\n');
			}
			return (n);
		case 's':
			if (subs1(a1, a2) == 0)
				goto err;
			if (verify(0) == 0)
				goto err;
			if (subs2(a1, a2) == 0)
				goto err;
			break;
		case 't':
			a3 = getaddr();
			if (adderrr)
				goto err;
			if (verify(0) == 0)
				goto err;
			if (copy(a1, a2, a3) == 0)
				goto err;
			break;
		case 'u':
			if (verify(0) == 0)
				goto err;
			if (a2<1 || a2>doladd) {
				derror("Illegal address range");
				goto err;
			}
			if ((line[a2]|1) != subseek) {
				derror("Cannot undo substitute");
				goto err;
			}
			line[a2] = suborig;
			subseek = 0;
			break;
		case 'v':
			if (addspec == 0) {
				a1 = 1;
				a2 = doladd;
			}
			if (global(a1, a2, 1) == 0)
				goto err;
			if (verify(1) == 0)
				goto err;
			break;
		case 'w':
		case 'W':
			if ((c1=getx()) != 'q')
				ungetx(c1);
			if (getfile(name) == 0)
				return (0);
			if (addspec == 0) {
				a1 = 1;
				a2 = doladd;
			}
			if (c=='W' && keyp!=NULL) {
				derror("Cannot append in encryption mode");
				goto err;
			}
			if (wfile(a1, a2, name, c=='w'?"w":"a", 0) && c1=='q')
				leave();
			return (1);
		case 'x':
			if (verify(1) == 0)
				goto err;
			if (setkey() == 0)
				goto err;
			break;
		default:
			derror("Bad command");
		err:
			while ((c=getx())!=EOF && c!='\n')
				;
			return (0);
		}
		if (intflag)
			return (0);
		switch (vcom) {
		case 'l':
			list(dotadd, dotadd);
			break;
		case 'p':
			print(dotadd, dotadd);
			break;
		}
		if ((c=getx()) == '\n')
			return (1);
		if (mflag == 0) {
			derror("Internal error");
			goto err;
		}
		ungetx(c);
	}
}

/*
 * Get an address.
 */
int getaddr(void)
{
	LINE seek;
	int absolute, abs, sign;
	int a, c, n;

	adderrr = 0;
	addpage = 0;
	addques = 0;
	absolute = 0;
	abs = 1;
	sign = 0;
	a = dotadd;
	for (;;) {
		switch (c=getx()) {
		case '.':
			n = dotadd;
			abs = 1;
			break;
		case '$':
			n = doladd;
			abs = 1;
			break;
		case '&':
			if (sign == 0) {
				addpage++;
				continue;
			}
			n = PGSIZE;
			abs = 0;
			break;
		case '\'':
			if (!isascii(c=getx()) || !islower(c)) {
				derror("Bad mark name");
				adderrr++;
				return (0);
			}
			seek = marklin[c-'a'];
			for (n=1; n<=doladd; n++) {
				if ((line[n]|1) == seek) {
					abs = 0;
					goto number;
				}
			}
			derror("Mark value not initialised");
			adderrr++;
			return (0);
		case '+':
			sign++;
			continue;
		case '^':
		case '-':
			--sign;
			continue;
		case '?':
			c = getx();
			ungetx(c);
			if (c == '\n') {
				addques++;
				return (0);
			}
			c = '?';
		case '/':
			if (compile(c) == 0) {
				adderrr++;
				return (0);
			}
			if (a<1 || a>doladd) {
				derror("Bad initial address");
				adderrr++;
				return (0);
			}
			n = a;
			abs = 1;
			do {
				if (intflag)
					return 0;
				if (c == '?') {
					if (--n < 1)
						n = doladd;
				} else {
					if (++n > doladd)
						n = 1;
				}
				if (n == a) {
					if (execute(n))
						break;
					derror("Search failed");
					adderrr++;
					return (0);
				}
			} while (execute(n) == 0);
			break;
		default:
			if (isascii(c) && isdigit(c)) {
				n = 0;
				do
					n = n*10 + c-'0';
				while (isascii(c=getx()) && isdigit(c));
				ungetx(c);
				abs = 0;
				break;
			}
			ungetx(c);
			if (sign) {
				a += sign;
				addspec++;
			}
			return (a);
		}
	number:
		if (sign == 0) {
			a = n;
			absolute = abs;
		} else {
			if ((absolute+=abs) > 1) {
				derror("Relocation address error");
				adderrr++;
				return (0);
			}
			a += sign>0 ? n : -n;
			sign = 0;
		}
		addspec++;
	}
}

/*
 * Get a character.
 */
int getx(void)
{
	int c;

	if (intflag != 0)
		return (EOF);
	if (savechr != '\0') {
		c = savechr;
		savechr = '\0';
		return (c);
	}
	if (gcp == NULL) {
		if (lastchr=='\n' && appflag==0) {
			if (pflag != 0) {
				printc(stderr, '*');
				fflush(stderr);
			}
			if (tflag != 0) {
				printc(stdout, '\000');
				printc(stdout, '\027');
				fflush(stdout);
			}
		}
		return (lastchr=getchar());
	}
	if ((c=*gcp++) != '\0')
		return (c);
	--gcp;
	return (EOF);
}

/*
 * Put back a character.
 */
void ungetx(int c)
{
	savechr = c;
}

/*
 * Given a line number, `a', get a line from the temp file
 * and store in the buffer `buffer'.  The number of characters read
 * including the null is returned.
 */
int egetline(int a, char *buffer)
{
	LINE l;
	int bno, off;
	char *bp, *dp, *cp;

	l = line[a];
	bno = blockn(l);
	off = offset(l);
	bp = buffer;
	for (;;) {
		if ((cp=getdisk(bno++)) == NULL)
			return (0);
		dp = &cp[off];
		cp += DBSIZE;
		while (dp < cp) {
			if ((*bp++=*dp++) == '\0')
				return (bp-buffer);
		}
		off = 0;
	}
}

/*
 * Given a buffer, `bp', containing a line and a number of
 * characters, `n', in the buffer, add the line onto the end of
 * the temp file.
 */
int putline(char *bp, int n)
{
	int bno, off, inc;
	char *dp;

	inc = (n+CLSIZE-1) & ~(CLSIZE-1);
	if (tmpseek+inc > (ULARGE/2)*CLSIZE) {
		derror("Temp file overflow");
		return (0);
	}
	bno = tmpseek / DBSIZE;
	off = tmpseek % DBSIZE;
	for (;;) {
		if (wcurbno>=0 && bno!=wcurbno)
			if (putdisk() == 0)
				return (0);
		wcurbno = bno;
		dp = &wdbcbuf[off];
		while (dp < &wdbcbuf[DBSIZE]) {
			*dp++ = *bp++;
			if (--n == 0) {
				tmpseek += inc;
				return (1);
			}
		}
		bno++;
		off = 0;
	}
}

/*
 * Given a block number, bring it into a disk buffer.  A pointer
 * to the buffer is returned.
 */
char *getdisk(int bno)
{
	if (bno == wcurbno)
		return (wdbcbuf);
	if (bno == rcurbno)
		return (rdbcbuf);
	lseek(fileno(tmp), (long) bno * DBSIZE, 0);
	if (read(fileno(tmp), rdbcbuf, sizeof(rdbcbuf)) != sizeof(rdbcbuf)) {
		terror("Read error");
		return (NULL);
	}
	rcurbno = bno;
	return (rdbcbuf);
}

/*
 * Write out the current disk buffer.
 */
int putdisk(void)
{
	lseek(fileno(tmp), (long) wcurbno * DBSIZE, 0);
	if (write(fileno(tmp), wdbcbuf, sizeof(wdbcbuf)) != sizeof(wdbcbuf)) {
		terror("Write error");
		return (0);
	}
	return (1);
}

/*
 * A diagnostic error.
 */
void derror(char *str)
{
	errstr = str;
	if (tflag != 0) {
		printc(stdout, '\000');
		printc(stdout, '\025');
		fflush(stdout);
	} else {
		printc(stderr, '?');
		if (vflag != 0)
			prints(stderr, str);
		printc(stderr, '\n');
	}
}

/*
 * A temp file error.
 */
void terror(char *str)
{
	errstr = str;
	if (tflag != 0) {
		printc(stdout, '\000');
		printc(stdout, '\025');
		fflush(stdout);
	} else {
		printc(stderr, '?');
		prints(stderr, str);
		printc(stderr, '\n');
	}
}

/*
 * Print out a decimal number.
 */
void printd(FILE *fp, int n)
{
	int c;

	if (n < 0) {
		printc(fp, '-');
		if ((n=-n) < 0)
			n = 0;
	}
	c = n%10 + '0';
	n /= 10;
	if (n != 0)
		printd(fp, n);
	printc(fp, c);
}

/*
 * Print out a long integer.
 */
void printl(FILE *fp, long n)
{
	int c;

	if (n < 0) {
		printc(fp, '-');
		if ((n=-n) < 0)
			n = 0;
	}
	c = n%10 + '0';
	n /= 10;
	if (n != 0)
		printl(fp, n);
	printc(fp, c);
}

/*
 * Print out an octal number.
 */
void printo(FILE *fp, int n)
{
	printc(fp, '0'+((n>>6)&03));
	printc(fp, '0'+((n>>3)&07));
	printc(fp, '0'+(n&07));
}

/*
 * Print out a string.
 */
void prints(FILE *fp, char *cp)
{
	while (*cp)
		printc(fp, *cp++);
}

/*
 * Print out a character.
 */
void printc(FILE *fp, char c)
{
	putc(c, fp);
	if (tflag==0 && c=='\n' && fp==stdout)
		fflush(fp);
}

/*
 * Commands.
 */

/*
 * Edit a file.  The filename is stored in `file'.
 */
int edit(void)
{
	int ret, i;

	if (doladd > 0)
		delete(1, doladd);
	tmpseek = CLSIZE;
	rcurbno = -1;
	wcurbno = -1;
	if ((fp=xfopen(file, "r")) == NULL) {
		derror("Cannot open file");
		return (0);
	}
	ret = 1;
	if (append(0, readfil) == 0)
		ret = 0;
	else if (ferror(fp)) {
		derror("Read error");
		ret = 0;
	}
	fclose(fp);
	for (i=0; i<MKSIZE; i++)
		marklin[i] = 0;
	saved = 1;
	if (cflag != 0) {
		printl(stdout, !oflag ? cct : lct);
		printc(stdout, '\n');
	}
	return (ret);
}

/*
 * Make sure that the previous command is terminated either
 * immediately by a newline or has a 'p' or a 'l' which is
 * then terminated by a newline.  The variable `vcom' is set
 * to the verify command.  A zero is returned if there were
 * bad characters before the newline, otherwise a one.
 */
int verify(int flag)
{
	int c;

	vcom = ' ';
	if (flag==0 && mflag!=0)
		return (1);
	if ((c=getx()) == '\n') {
		ungetx(c);
		return (1);
	}
	if (c=='l' || c=='p') {
		if (getx() == '\n') {
			ungetx('\n');
			vcom = c;
			return (1);
		}
	}
	derror("Bad command");
	return (0);
}

/*
 * Get a filename of the form required for the r command.
 * which is returned in `name'.  If no name is specified,
 * the one stored in `file' is copied.  If a file name is
 * specified and none exists in `file', it is copied to
 * `file'.  If an illegal name is given, an error is derrord.
 */
int getfile(char *name)
{
	int c;
	char *p1, *p2;

	switch (c=getx()) {
	case '\n':
		p1 = name;
		p2 = file;
		while (*p1++ = *p2++)
			;
		break;
	case ' ':
	case '\t':
		p1 = name;
		p2 = &name[FNSIZE];
		while ((c=getx()) == ' ' || c == '\t')
			;
		*p1++ = c;
		while ((c=getx()) != EOF && c != '\n') {
			if (p1 >= p2) {
				derror("File name too long");
				name[0] = '\0';
				return (0);
			}
			*p1++ = c;
		}
		*p1 = '\0';
		if (c == EOF) {
			derror("EOF in file name");
			name[0] = '\0';
			return (0);
		}
		if (file[0] == '\0') {
			p1 = file;
			p2 = name;
			while (*p1++ = *p2++)
				;
		}
		break;
	default:
		while (getx() != '\n')
			;
		derror("Bad command");
		return (0);
	}
	if (name[0] == '\0') {
		derror("Null file name");
		return (0);
	}
	return (1);
}

/*
 * Read the rest of the input line into `tempbuf'.
 */
int rest(void)
{
	int c;
	char *cp;

	cp = tempbuf;
	while ((c=getx())!=EOF && c!='\n') {
		if (cp >= &tempbuf[TBSIZE-1]) {
			derror("Line too long");
			return (0);
		}
		*cp++ = c;
	}
	*cp = '\0';
	return (1);
}

/*
 * Append text after the given address.  A line of text is gotten
 * by calling the given function until it returns NULL.
 */
int append(int a, int (*f)(void))
{
	int seek;
	int n;

	cct = 0;
	lct = 0;
	if (a<0 || a>doladd) {
		derror("Address out of range");
		return (0);
	}
	dotadd = a;
	while ((n=(*f)()) != 0) {
		if (intflag)
			return (1);
		seek = linead();
		if (putline(linebuf, n) == 0)
			return (0);
		saved = 0;
		cct += n;
		lct++;
		if (expand(dotadd) == 0)
			return (0);
		line[++dotadd] = seek;
	}
	return (1);
}

/*
 * Expand the line table and leave a hole at the given address.
 */
int expand(int a)
{
	LINE *lp1;
	LINE *lp2;
	int n;

	if (doladd+3 >= lnsize) {
		lp1 = (LINE *)realloc(line, (n=lnsize*2)*sizeof(LINE));
		if (lp1 == NULL) {
			derror("Line table overflow");
			return (0);
		}
		line = lp1;
		lnsize = n;
	}
	lp1 = &line[doladd+2];
	lp2 = &line[doladd+1];
	n = doladd++ - a;
	while (n--)
		*--lp1 = *--lp2;
	return (1);
}

/*
 * Get a line of text from the terminal.  The number of characters
 * including the null terminator at the end of the string is returned.
 * If end of file or a line containing only a single dot is found,
 * 0 is returned.
 */
int readtty(void)
{
	int c;
	char *lp;

	appflag++;
	lp = linebuf;
	while ((c=getx())!=EOF && c!='\n') {
		if (lp < &linebuf[LBSIZE-1])
			*lp++ = c;
	}
	--appflag;
	if (c==EOF && lp==linebuf) {
		lastchr = '\n';
		if (gcp == NULL)
			clearerr(stdin);
		return (0);
	}
	if (linebuf[0]=='.' && lp==linebuf+1)
		return (0);
	*lp++ = '\0';
	return (lp-linebuf);
}

/*
 * Get a line of text from the file open on file pointer `fp'.
 * Return the number of characters in the line including the
 * null terminator at the end.  On end of file, 0 is returned.
 */
int readfil(void)
{
	int c, n;
	char *lp;

	if ((c=getc(fp)) == EOF)
		return (0);
	lp = linebuf;
	n = LBSIZE-1;
	while (c!=EOF && c!='\n') {
		*lp++ = c;
		if (--n == 0)
			break;
		c = getc(fp);
	}
	*lp++ = '\0';
	return (lp-linebuf);
}

/*
 * Delete the given line range.
 */
int delete(int a1, int a2)
{
	LINE *p1, *p2;
	int n;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	p1 = &line[a1];
	p2 = &line[a2+1];
	n = doladd - a2;
	while (n--)
		*p1++ = *p2++;
	doladd -= (a2+1) - a1;
	if ((dotadd=a1) > doladd)
		--dotadd;
	saved = doladd==0;
	return (1);
}

/*
 * Concatenate the lines in the given range to form
 * a single line.
 */
int join(int a1, int a2)
{
	long seek;
	int bn, a;
	int n;
	char *lp, *tp;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	bn = 0;
	lp = linebuf;
	for (a=a1; a<=a2; a++) {
		if ((n=egetline(a, tempbuf)) == 0)
			return (0);
		if ((bn+=--n) >= LBSIZE-1) {
			derror("Temporary buffer overflow");
			return (0);
		}
		tp = tempbuf;
		while (n--)
			*lp++ = *tp++;
	}
	*lp++ = '\0';
	seek = linead();
	if (putline(linebuf, ++bn) == 0)
		return (0);
	line[a1] = seek;
	a = doladd - a2;
	for (n=0; n<a; n++)
		line[a1+1+n] = line[a2+1+n];
	dotadd = a1;
	doladd -= a2 - a1;
	return (1);
}

/*
 * List the given line range.  All non-printing characters are
 * escaped.
 */
int list(int a1, int a2)
{
	int a;
	int n;
	char *p;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	for (a=a1; a<=a2; a++) {
		if (intflag)
			return (1);
		if (egetline(a, linebuf) == 0)
			break;
		n = 0;
		for (p=linebuf; *p; p++) {
			if (n++ >= 72) {
				n = 0;
				prints(stdout, "\\\n");
			}
			switch (*p) {
			case '\b':
				prints(stdout, "-\b<");
				continue;
			case '\t':
				prints(stdout, "-\b>");
				continue;
			case '\\':
				prints(stdout, "\\\\");
				continue;
			default:
				if (isascii(*p) && !iscntrl(*p)) {
					printc(stdout, *p);
					continue;
				}
				printc(stdout, '\\');
				printo(stdout, *p);
				n += 3;
			}
		}
		prints(stdout, "\\n");
		printc(stdout, '\n');
		dotadd = a;
	}
	return (1);
}

/*
 * Take the text that is between lines `a1' and `a2' and
 * place it after line `a3'.
 */
int move(int a1,int a2,int a3)
{
	LINE l;
	int a, n, x;

	if (a1>a2 || a1<1 || a2>doladd || a3<0 || a3>doladd) {
		derror("Address out of range");
		return (0);
	}
	if (a3>=a1-1 && a3<=a2) {
		dotadd = a2;
		return (1);
	}
	if (a3 < a1) {
		for (a=a1, x=a3+1; a<=a2; a++, x++) {
			l = line[a];
			for (n=a; n>x; --n)
				line[n] = line[n-1];
			line[x] = l;
		}
		dotadd = a3 + a2+1 - a1;
	} else {
		for (a=a2, x=a3; a>=a1; --a, --x) {
			l = line[a];
			for (n=a; n<x; n++)
				line[n] = line[n+1];
			line[x] = l;
		}
		dotadd = a3;
	}
	saved = 0;
	return (1);
}

/*
 * Given a string describing a set of options, set them.
 */
void setoptf(char *sp)
{
	int t;

	t = 1;
	while (*sp != '\0') {
		switch (*sp++) {
		case '+':
			t = 1;
			continue;
		case '-':
			t = 0;
			continue;
		case 'c':
			cflag = t;
			continue;
		case 'm':
			mflag = t;
			continue;
		case 'o':
			oflag = t;
			continue;
		case 'p':
			pflag = t;
			continue;
		case 's':
			sflag = t;
			continue;
		case 'v':
			vflag = t;
			continue;
		default:
			derror("Bad option");
			return;
		}
	}
}

/*
 * Display options.
 */
void disoptf(void)
{
	if (cflag)
		printc(stdout, 'c');
	if (mflag)
		printc(stdout, 'm');
	if (oflag)
		printc(stdout, 'o');
	if (pflag)
		printc(stdout, 'p');
	if (sflag)
		printc(stdout, 's');
	if (vflag)
		printc(stdout, 'v');
	if (keyp != NULL)
		printc(stdout, 'x');
	printc(stdout, '\n');
}

/*
 * Print the given line range.
 */
int print(int a1, int a2)
{
	int a;
	char *p;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	for (a=a1; a<=a2; a++) {
		if (intflag)
			return (1);
		if (egetline(a, linebuf) == 0)
			break;
		p = linebuf;
		while (*p)
			printc(stdout, *p++);
		printc(stdout, '\n');
		dotadd = a;
	}
	return (1);
}

/*
 * Make a copy of the text between lines `a1' and `a2'
 * and place them after `a3'.
 */
int copy(int a1, int a2, int a3)
{
	int i, j, n;

	if (a1>a2 || a1<1 || a2>doladd || a3<0 || a3>doladd) {
		derror("Address out of range");
		return (0);
	}
	n = (a2+1) - a1;
	if (doladd+n+2 > lnsize) {
		derror("Line table overflow");
		return (0);
	}
	for (i=doladd; i>a3; --i)
		line[i+n] = line[i];
	for (i=0; i<n; i++) {
		if ((j=i+a1) > a3)
			j += n;
		if ((j=egetline(j, linebuf)) == 0)
			goto err;
		line[a3+1+i] = linead();
		if (putline(linebuf, j) == 0)
			goto err;
	}
	dotadd = a3+n;
	doladd += n;
	saved = 0;
	return (1);

err:
	for (i=a3+1; i<=doladd; i++)
		line[i] = line[i+n];
	return (0);
}

/*
 * Write the given line range onto the file whose name is
 * stored in `file'.  `perm' is the permission string we
 * want the file opened with.
 */
int wfile(int a1, int a2, char *name, char * perm, int sflag)
{
	int a;
	char *cp;

	if (doladd!=0 || addspec!=0) {
		if (a1>a2 || a1<1 || a2>doladd) {
			derror("Address out of range");
			return (0);
		}
	}
	if ((fp=xfopen(name, perm)) == NULL) {
		derror("Cannot open file");
		return (0);
	}
	cct = 0;
	lct = 0;
	for (a=a1; a<=a2; a++) {
		if (egetline(a, linebuf) == 0)
			break;
		cp = linebuf;
		while (*cp) {
			printc(fp, *cp++);
			cct++;
		}
		printc(fp, '\n');
		if (ferror(fp))
			break;
		cct++;
		lct++;
	}
	if (sflag==0 && cflag!=0) {
		printl(stdout, !oflag ? cct : lct);
		printc(stdout, '\n');
	}
	if ((a=ferror(fp)) == 0)
		saved = 1;
	else {
		if (sflag == 0)
			derror("Write error");
	}
	fclose(fp);
	sync();
	return (!a);
}

/*
 * Open a file.  If we are in encryption mode, the open unit is really a
 * file descriptor piped to the crypt command.
 */
FILE *xfopen(char *fn, char *mode)
{
#if COHERENT
	FILE *fp;
	int f;
	int u;
	int a;
	int b;
	int pv[2];

	if (keyp == NULL)
		fp = fopen(fn, mode);
	else {
		switch (mode[0]) {
		case 'a':
			return (NULL);
		case 'r':
			a = 0;
			b = 1;
			break;
		case 'w':
			a = 1;
			b = 0;
			break;
		}
		if (pipe(pv) < 0)
			return (NULL);
		if ((f=fork()) < 0) {
			close(pv[0]);
			close(pv[1]);
			return (NULL);
		}
		if (f == 0) {
			if ((u=(a==0)?open(fn, 0):creat(fn, 0644)) < 0)
				exit(1);
			dup2(u, a);
			dup2(pv[b], b);
			close(u);
			close(pv[0]);
			close(pv[1]);
			execl("/bin/crypt", "crypt", keyp, NULL);
			exit(1);
		}
		close(pv[b]);
		fp = fdopen(pv[a], mode);
	}
	return (fp);
#else
	return(fopen(fn, mode));
#endif
}

/*
 * Read a key from the standard input.
 */
int setkey(void)
{
#if COHERENT
	int n;

	keyp = getpass("key? ");
	if ((n=strlen(keyp)) > CKSIZE) {
		derror("Key too long");
		keyp = NULL;
		return (0);
	}
	if (n == 0)
		keyp = NULL;
	return (1);
#else
	derror("Cannot set key");
	return (0);
#endif
}

/*
 * Regular expressions and the substitute and global commands.
 */

/*
 * Global command.
 */

int global(int a1, int a2, int not)
{
	int a, c;
	char *gp;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	if (gcp != NULL) {
		derror("Global inside global not allowed");
		return (0);
	}
	if ((c=getx()) == '\n') {
		ungetx(c);
		derror("Syntax error");
		return (0);
	}
	if (compile(c) == 0)
		return 0;
	gp = globbuf;
	while ((c=getx()) != '\n') {
		if (gp >= &globbuf[GBSIZE-3]) {
			derror("Global buffer overflow");
			return (0);
		}
		if (c == '\\')
			if ((c=getx())!='\n' && c!='\\')
				*gp++ = '\\';
		*gp++ = c;
	}
	if (gp == globbuf)
		goto out;
	*gp++ = '\n';
	*gp = '\0';
	for (a=a1; a<=a2; a++) {
		if (intflag)
			goto out;
		if (execute(a) != not)
			line[a] |= 1;
	}
	a = 1;
	while (a <= doladd) {
		if (intflag)
			goto out;
		if ((line[a]&1) == 0) {
			a++;
			continue;
		}
		--line[a];
		dotadd = a;
		gcp = globbuf;
		while (*gcp != '\0')
			if (command() == 0)
				goto out;
		a = 1;
	}
out:
	for (a=1; a<=doladd; a++)
		line[a] &= ~1;
	gcp = NULL;
	ungetx('\n');
	return (1);
}

/*
 * Substitute part 1.
 * Check that address range is legal and parse substitute command.
 */
int subs1(int a1, int a2)
{
	int ec, c;
	char *sp;

	if (a1>a2 || a1<1 || a2>doladd) {
		derror("Address out of range");
		return (0);
	}
	subnewl = 0;
	subnths = 1;
	if (isascii(c=getx()) && isdigit(c)) {
		subnths = 0;
		while (isascii(c) && isdigit(c)) {
			subnths = subnths*10 + c-'0';
			c = getx();
		}
	}
	if ((ec=c) == '\n') {
		ungetx(c);
		derror("Syntax error");
		return (0);
	}
	if (compile(ec) == 0)
		return (0);
	sp = subsbuf;
	while ((c=getx()) != ec) {
		if (sp >= &subsbuf[SBSIZE-4]) {
			derror("Temporary buffer overflow");
			return (0);
		}
		switch (c) {
		case '\n':
			ungetx(c);
			derror("Syntax error");
			return (0);
		case '&':
			*sp++ = '\\';
			*sp++ = '0';
			continue;
		case '\\':
			if ((c=getx()) == '\\') {
				*sp++ = '\\';
				*sp++ = '\\';
				continue;
			}
			if (c >= '1'  &&  c <= '9') {
				*sp++ = '\\';
				*sp++ = c;
				continue;
			}
			if (c == '\n')
				subnewl++;
		default:
			*sp++ = c;
			continue;
		}
	}
	*sp++ = '\0';
	if ((c=getx()) == 'g')
		subnths = 0;
	else
		ungetx(c);
	return (1);
}

/*
 * Substitute (part 2).
 * Execute substitute command.
 */
int subs2(int a1, int a2)
{
	long seek, mark;
	int len, nth, err, a;
	char *mp, *sp;
	int n;
	char *lp, *rp;

	err = 1;
	for (a=a1; a<=a2; a++) {
		if (intflag)
			return (1);
		if (egetline(a, tempbuf) == 0)
			return (0);
		for (n=0; n<1+BRSIZE; n++) {
			brace[n].b_bp = NULL;
			brace[n].b_ep = NULL;
		}
		brace[0].b_ep = tempbuf;
		mp = tempbuf;
		lp = linebuf;
		nth = 0;
		for (;;) {
			rp = codebuf;
			if (*rp == CSSOL) {
				if (mp != tempbuf)
					break;
				rp++;
			}
			if ((rp=match(mp, rp)) == NULL) {
				if (*mp++ == '\0')
					break;
				continue;
			}
			nth++;
			if (subnths) {
				if (nth < subnths) {
					mp = rp;
					continue;
				}
				if (nth > subnths)
					goto done;
			}
			err = 0;
			saved = 0;
			dotadd = a;
			brace[0].b_bp = mp;
			mp = rp;
			rp = brace[0].b_ep;
			brace[0].b_ep = mp;
			len = mp - brace[0].b_bp;
			n = brace[0].b_bp - rp;
			if (lp+n >= &linebuf[LBSIZE-1])
				goto ovf;
			while (n--)
				*lp++ = *rp++;
			sp = subsbuf;
			while (*sp) {
				if (lp >= &linebuf[LBSIZE-4])
					goto ovf;
				if (*sp != '\\') {
					*lp++ = *sp++;
					continue;
				}
				if (*++sp == '\\') {
					*lp++ = *sp++;
					continue;
				}
				n = *sp++ - '0';
				if ((rp=brace[n].b_bp) == NULL)
					continue;
				n = brace[n].b_ep-rp;
				if (lp+n >= &linebuf[LBSIZE-4])
					goto ovf;
				while (n--)
					*lp++ = *rp++;
			}
			if (*mp == '\0')
				break;
			if (len == 0)
				mp++;
			if (subnths!=0 && nth==subnths)
				break;
		}
	done:
		if (dotadd != a)
			continue;
		rp = brace[0].b_ep;
		while (*rp) {
			if (lp >= &linebuf[LBSIZE-1])
				goto ovf;
			*lp++ = *rp++;
		}
		*lp++ = '\0';
		seek = linead();
		if (subnewl) {
			lp = linebuf;
			while (*lp != '\n')
				lp++;
			*lp++ = '\0';
		}
		if (putline(linebuf, lp-linebuf) == 0)
			return (0);
		suborig = line[a];
		if (suborig&1)
			--suborig;
		subseek = seek|1;
		line[a] = seek;
		mark = suborig|1;
		for (n=0; n<MKSIZE; n++)
			if (marklin[n] == mark)
				marklin[n] = subseek;
		if (subnewl != 0) {
			n = 1;
			do {
				mp = lp;
				while (*lp!='\n' && *lp!='\0')
					lp++;
				if (*lp == '\0')
					n = 0;
				*lp++ = '\0';
				seek = linead();
				if (putline(mp, lp-mp) == 0)
					return (0);
				if (expand(a) == 0)
					return (0);
				line[++a] = seek;
				dotadd = a;
				a2++;
			} while (n);
		}
	}
	if (err!=0 && gcp==NULL) {
		derror("Pattern not matched");
		return (0);
	}
	return (1);

ovf:
	derror("Temporary buffer overflow");
	return (0);
}

/*
 * Compile a regular expression.  `ec' is the character upon which
 * the regular expression ends.  If an error is encountered, the
 * pattern is restored and input characters are thrown away until
 * a new line is found.
 */
int compile(int ec)
{
	int bstack[BRSIZE], bcount, blevel, n;
	int c;
	char *cp, *lcp;

	bcpy(tempbuf, codebuf);
	bcount = 1;
	blevel = 0;
	cp = &codebuf[0];
	if ((c=getx()) == ec) {
		if (*cp == CSNUL) {
			derror("No saved pattern");
			goto err;
		}
		return (1);
	}
	if (c == '^') {
		*cp++ = CSSOL;
		c = getx();
	}
	while (c != ec) {
		if (c == '\n') {
			goto nwl;
		}
		if (cp > &codebuf[CBSIZE-4])
			goto ovf;
		switch (c) {
		case '*':
			goto syn;
		case '.':
			if ((c=getx()) != '*') {
				*cp++ = CSDOT;
				continue;
			}
			*cp++ = CMDOT;
			c = getx();
			continue;
		case '$':
			if ((c=getx()) != ec) {
				ungetx(c);
				c = '$';
				goto character;
			}
			*cp++ = CSEOL;
			continue;
		case '[':
			lcp = cp;
			if ((c=getx()) == '^')
				*cp++ = CSNCL;
			else {
				ungetx(c);
				*cp++ = CSCCL;
			}
			*cp++ = 0;
			if ((c=getx()) == ']')
				*cp++ = c;
			else
				ungetx(c);
			while ((c=getx()) != ']') {
				if (c == '\n')
					goto nwl;
				if (c!='-' || cp==lcp+2) {
					if (cp >= &codebuf[CBSIZE-4])
						goto ovf;
					*cp++ = c;
					if (sflag && isascii(c) && isallet(c))
						*cp++ = toother(c);
					continue;
				}
				if ((c=getx()) == '\n')
					goto nwl;
				if (c == ']') {
					*cp++ = '-';
					ungetx(c);
					continue;
				}
				if ((n=cp[-1]) > c)
					goto syn;
				while (++n <= c) {
					if (cp >= &codebuf[CBSIZE-4])
						goto ovf;
					*cp++ = n;
					if (sflag && isascii(c) && isallet(c))
						*cp++ = toother(c);
				}
			}
			if ((c=getx()) == '*') {
				(*lcp)++;
				c = getx();
			}
			if ((n=cp-(lcp+2)) > 255) {
				derror("Character class too large");
				goto err;
			}
			*++lcp = n;
			continue;
		case '\\':
			switch (c=getx()) {
			case '\n':
				goto nwl;
			case '(':
				if (bcount > BRSIZE) {
					derror("Too many \\(");
					goto err;
				}
				*cp++ = CSOPR;
				*cp++ = bstack[blevel++] = bcount++;
				c = getx();
				continue;
			case ')':
				if (blevel == 0)
					goto syn;
				*cp++ = CSCPR;
				*cp++ = bstack[--blevel];
				c = getx();
				continue;
			default:
				if (isascii(c) && isdigit(c)) {
					*cp++ = CSBRN;
					*cp++ = c-'0';
					c = getx();
					continue;
				}
			}
		default:
		character:
			if (sflag && isascii(c) && isallet(c)) {
				*cp++ = CSSCC;
				if (isupper(c))
					c = tolower(c);
			} else
				*cp++ = CSCHR;
			*cp++ = c;
			if ((c=getx()) == '*') {
				cp[-2]++;
				c = getx();
			}
		}
	}
	*cp++ = CSNUL;
	return (1);
ovf:
	derror("Code buffer overflow");
	bcpy(codebuf, tempbuf);
	return (0);
nwl:
	ungetx(c);
syn:
	derror("Syntax error");
err:
	bcpy(codebuf, tempbuf);
	return (0);
}

/*
 * Copy BSIZE buffer into dest from source
 */
void bcpy(char *dest, char *source)
{
	int n;

	n = CBSIZE;
	do
		*dest++ = *source++;
	while (--n > 0);
}

/*
 * Return 1 if the compiled expression in `codebuf' matches the line in
 * `linebuf', else 0.
 */
int execute(int a)
{
	int i;
	char *lp, *ep;

	if (egetline(a, linebuf) == 0)
		return (0);
	for (i=0; i<1+BRSIZE; i++) {
		brace[i].b_bp = NULL;
		brace[i].b_ep = NULL;
	}
	if (codebuf[0] == CSSOL)
		ep = match(lp=linebuf, &codebuf[1]);
	else {
		ep = NULL;
		lp = linebuf;
		do {
			if (ep=match(lp, codebuf))
				break;
		} while (*lp++);
	}
	if (ep) {
		brace[0].b_bp = lp;
		brace[0].b_ep = ep;
	}
	return (ep ? 1 : 0);
}

/*
 * Given a pointer to a compiled expression, `cp', and a pointer to
 * a line, `lp', return 1 if the expression matches, else 0.
 */
char *match(char *lp, char *cp)
{
	int n;
	char *llp, *lcp;

	for (;;) {
		switch (*cp++) {
		case CSNUL:
			return (lp);
		case CSEOL:
			if (*lp)
				return (NULL);
			return (lp);
		case CSOPR:
			brace[*cp++].b_bp = lp;
			continue;
		case CSCPR:
			brace[*cp++].b_ep = lp;
			continue;
		case CSBRN:
			n = *cp++;
			lcp = cp;
			cp = brace[n].b_bp;
			n = brace[n].b_ep - cp;
			if (n > LBSIZE)
				return (NULL);
			while (n-- > 0)
				if (*lp++ != *cp++)
					return (NULL);
			cp = lcp;
			continue;
		case CSDOT:
			if (*lp++ == '\0')
				return (NULL);
			continue;
		case CMDOT:
			llp = lp;
			while (*lp)
				lp++;
			goto star;
		case CSCHR:
			if (*cp++ != *lp++)
				return (NULL);
			continue;
		case CMCHR:
			llp = lp;
			while (*cp == *lp)
				lp++;
			cp++;
			goto star;
		case CSSCC:
			if (*cp++ == tolower(*lp++))
				continue;
			return (NULL);
		case CMSCC:
			llp = lp;
			while (*cp == tolower(*lp++))
				;
			cp++;
			goto star;
		case CSCCL:
			n = *cp++;
			while (*cp++ != *lp)
				if (--n == 0)
					return (NULL);
			lp++;
			cp += n-1;
			continue;
		case CMCCL:
			llp = lp;
			lcp = cp;
			while (*lp) {
				cp = lcp;
				n = *cp++;
				while (*cp++ != *lp)
					if (--n == 0)
						goto star;
				lp++;
			}
			cp = lcp + *lcp + 1;
			goto star;
		case CSNCL:
			if (*lp == '\0')
				return (NULL);
			n = *cp++;
			while (n--)
				if (*cp++ == *lp)
					return (NULL);
			lp++;
			continue;
		case CMNCL:
			llp = lp;
			lcp = cp;
			while (*lp) {
				cp = lcp;
				n = *cp++;
				while (n--) {
					if (*cp++ == *lp) {
						cp = lcp + *lcp + 1;
						goto star;
					}
				}
				lp++;
			}
			cp = lcp + *lcp + 1;
		star:
			do {
				if (lcp=match(lp, cp))
					return (lcp);
			} while (--lp >= llp);
			return (NULL);
		}
	}
}
