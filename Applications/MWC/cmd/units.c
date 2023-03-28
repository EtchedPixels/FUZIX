/*
 * $Header: /newbits/bin/units.c,v 1.2 89/02/22 05:09:42 bin Exp $
 * $Log:	/newbits/bin/units.c,v $
 * Revision 1.2	89/02/22  05:09:42 	bin
 * Changed memory allocation to work better on the atari.
 * Changed where it looks for units and binary units, using path.
 *
 * FIXME: Check unit codes versus man page/POSIX/etc
 * FIXME: split compiler from units for size/stdio and to remove setuidness
 * 
 */
/*static	char	*revision = "$Revision 1.1 $";
static	char *header =
	"$Header: /newbits/bin/units.c,v 1.2 89/02/22 05:09:42 bin Exp $"; */

/*
 * units -- do multiplicative unit conversions
 * td 80.09.04
 * Modified to keep the intermediate format in a file and
 * update it automatically when it has changed.
 * (NOTE: program is setuid to bin)
 * rec 84.08.13
 * Changed Waiting for Godot ... to Rebuilding %s from %s.
 * (Nota Bene:
 *	cc -o units -f units.c
 *	chown bin /bin/units
 *	chmod 4755 /bin/units
 * )
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>

#define PATHSIZE 64
#define	NDIM	12
#define	NUNITS	900
#define	NBUF	256		/* Length of longest line */
#define	UMAGIC	0123456		/* Magic number written in binary header */

/*
 * Header for the units file.
 * This  tells that it is the real
 * thing and how much to read in.
 */
typedef	struct	HDR {
	unsigned h_magic;
	unsigned h_nunits;
	unsigned h_ssize;		/* String space size */
}	HDR;

HDR	hdr;

typedef	struct	UNIT {
	char	*u_name;
	double	u_val;
	char	u_dim[NDIM];
}	UNIT;

UNIT	*units;
struct	stat	sb;
char	ufile[] = "units";
char	binufile[] = "binunits";
const char *unamep;
const char *bunamep;
char	buf[NBUF];
char	inbuf[BUFSIZ];

int nunits;
int	uflag;			/* Update information only */
FILE *fd;
int peekc = EOF;
int lastc = EOF;
int lineno = 1;

/*
 * get contiguous memory.
 *
 * FIXME: kill stdio & malloc usage
 */
char *alloc(unsigned int nb)
{
#if 0 
	register char *rp;
	if((rp = sbrk(nb)) == ((char *)-1))
		errx(1, "out of memory");
	return (rp);
#else 
	char *rp = malloc(nb);
	if (rp == NULL)
		errx(1, "out of memory");
	return rp;
#endif
}

int nextc(void)
{
	register int c;

	if (peekc != EOF) {
		c = peekc;
		peekc = EOF;
		return (c);
	}
	if (lastc == '\n')
		lineno++;
	lastc = getc(fd);
	if (lastc == '#') {	/* Eat a comment */
		do {
			lastc = getc(fd);
		} while(lastc!='\n' && lastc!=EOF);
	}
	return (lastc);
}

char *getname(void)
{
	register char *s, *t;
	register int c;
	register char *v;

	do {
		c = nextc();
	} while(c==' ' || c=='\n' || c=='\t');
	s = buf;
	while(c!=' ' && c!='\t' && c!='\n' && c!=EOF) {
		*s++ = c;
		c = nextc();
	}
	*s = '\0';
	peekc = c;
	v = t = alloc(strlen(buf)+1);
	s = buf;
	while ((*t++ = *s++) != 0)
		;
	return (v);
}

/*
 * Attempt to read in the already-stored
 * binary information.  Return non-zero if
 * successful.
 *
 * if(binary and ascii)
 *	 if(binary out of date)
 *		 rebuild
 *	 else
 *		use binary
 * if(binary and no ascii)
 *	use binary
 * if(ascii and no binary)
 *	rebuild
 * crash
 */
int binary(void)
{
	register char *sstart;
	register int n;
	register int bfd;
	time_t timeasc;

	unamep  = "/usr/lib/units/units.src";
	bunamep = "/usr/lib/units/units.dat";
	
	if(!stat(unamep, &sb)) { /* binary and ascii found */
		timeasc = sb.st_mtime;
		if(stat(bunamep, &sb) || (timeasc > sb.st_mtime))
			return(0);	/* bin file out of date */
	}
	if(uflag)	/* update only */
		return(0);
	if((bfd = open(bunamep, 0)) < 0)
		return (0);
	if(read(bfd, &hdr, sizeof(hdr)) != sizeof(hdr))
		goto bad;
	if(hdr.h_magic != UMAGIC)
		goto bad;
	nunits = hdr.h_nunits;
	sstart = alloc(hdr.h_ssize);
	if (read(bfd, sstart, hdr.h_ssize) != hdr.h_ssize)
		goto bad;
	units = (UNIT *)alloc(n = nunits*sizeof(UNIT));
	if (read(bfd, units, n) != n)
		goto bad;
	for (n=0; n!=nunits; n++)
		units[n].u_name += (long)sstart;
	close(bfd);
	return (1);
bad:
	close(bfd);
	return (0);
}

/*
 * Check for any occurrences of
 * the character `c' in string `s'.
 *
 * FIXME: use strchr
 */

int anyc(char c, char *s)
{
	while (*s != '\0')
		if (c == *s++)
			return (1);
	return (0);
}

/*
 * Return non-zero if string `s' is the
 * same or plural as string `t'.
 */
int eqplural(char *s, char *t)
{
	while (*t != '\0')
		if (*s++ != *t++)
			return (0);
	return (*s=='\0' || (*s++=='s' && *s=='\0'));
}

double ipow(double d, int n)
{
	double v;

	v = 1.;
	if (n < 0) {
		d = 1./d;
		n = -n;
	}
	while (n) {
		v *= d;
		--n;
	}
	return (v);
}

struct{
	const char *prefix;
	double factor;
}pre[]={
/*FIXME: Check v POSIX and man units */
{"femto",	1e-15},
{"pico",	1e-12},
{"nano",	1e-9},
{"micro",	1e-6},
{"milli",	1e-3},
{"centi",	1e-2},
{"deci",	1e-1},
{"hemi",	.5},
{"demi",	.5},
{"semi",	.5},
{"sesqui",	1.5},
{"deca",	1e1},
{"hecto",	1e2},
{"kilo",	1e3},
{"myria",	1e5},
{"mega",	1e6},
{"giga",	1e9},
{"tera",	1e12},
{NULL,		1.0}
};

/*
 * Return the string stripped of its
 * prefix (if any).  Set factor
 * to the multiplicative factor indicated by
 * the prefix found.
 */
const char *prefix(const char *str, double *factor)
{
	register const char *s, *t;
	register int i;

	for (i=0; pre[i].prefix!=NULL; i++) {
		s = pre[i].prefix;
		t = str;
		while (*s != '\0')
			if (*s++ != *t++)
				break;
		if (*s == '\0') {
			*factor = *factor * pre[i].factor;
			return (t);
		}
	}
	return(NULL);
}

int getunit(UNIT *u, char *prompt)
{
	register int c;
	register char *s;
	register int i;
	int j, expon, digit, div, pow;
	double factor;

Again:
	if (prompt != NULL)
		printf("%s", prompt);
	u->u_val = 1.;
	for (i=0; i != NDIM; i++)
		u->u_dim[i] = 0;
	div = 0;
	pow = 1;
	for(;;)switch(c=nextc()){
	case ' ':
	case '\t':
		break;
	case '\n':
		return (1);
	case EOF:
		return (0);
	case '0':case '1':case '2':case '3':case '4':
	case '5':case '6':case '7':case '8':case '9':
	case '.':case '-':case '+':
		/*
		 * a palpable number
		 */
		s = buf;
		if (c == '+')
			c = nextc();
		digit = 0;
		while (c>='0' && c<='9') {
			*s++ = c;
			c = nextc();
			digit++;
		}
		if (c == '.') {
			*s++ = c;
			while ((c=nextc())>='0' && c<='9') {
				*s++ = c;
				digit++;
			}
		}
		if (!digit) {
		Badnumber:
			*s = '\0';
			fprintf(stderr, "Bad number `%s'\n", buf);
			goto Bad;
		}
		if (c=='e' || c=='E') {
			*s++ = 'e';
			c = nextc();
			if (c == '+')
				c = nextc();
			else if (c == '-') {
				*s++ = c;
				c = nextc();
			}
			if (c<'0' || '9'<c)
				goto Badnumber;
			do {
				*s++ = c;
				c = nextc();
			} while('0'<=c && c<='9');
		}
		*s = '\0';
		peekc = c;
		factor = atof(buf);
		if (div) {
			if (factor == 0.) {
				fprintf(stderr, "Divide check\n");
				goto Bad;
			}
			u->u_val /= factor;
			div = 0;
		} else
			u->u_val *= factor;
		break;

	case '/':	/* divide by next unit */
		if (div) {
		Baddiv:
			fprintf(stderr, "Two division signs in a row\n");
			goto Bad;
		}
		div++;
		break;

	case '!':	/* primitive unit */
		i = 0;
		if ((c = nextc())<'0' || c>'9') {
			fprintf(stderr, "`!' must precede a number\n");
			goto Bad;
		}
		do {
			i = i*10+c-'0';
			c = nextc();
		} while('0'<=c && c<='9');
		peekc = c;
		if (i<0 || NDIM<=i) {
			printf("Primitive unit out of range [0,%d]\n", NDIM-1);
			goto Bad;
		}
		u->u_dim[i]++;
		break;

	default:
		s = buf;
		do {
			*s++ = c;
			c = nextc();
		} while(c!=EOF && !anyc(c, "/0123456789+-. \t\n"));
		*s = '\0';
		s = buf;
		if (strcmp(s, "per") == 0) {
			if (div)
				goto Baddiv;
			div++;
			break;
		}
		if (strcmp(s, "square")==0 || strcmp(s, "sq")==0) {
			pow *= 2;
			break;
		}
		if (strcmp(s, "cubic")==0 || strcmp(s, "cu")==0) {
			pow *= 3;
			break;
		}
		factor = 1.;
		do {
			for (i=0; i!=nunits; i++)
				if (eqplural(s, units[i].u_name))
					break;
		} while(i==nunits && (s=prefix(s, &factor))!=NULL);
		if (i == nunits) {
			fprintf(stderr, "Unrecognised unit %s\n", buf);
			goto Bad;
		}
		if (c=='+' || c=='-') {
			if (c == '-')
				div = !div;
			expon = 0;
			if ((c = nextc())<'0' || c>'9') {
				printf("+ or - must be followed by digits\n");
				goto Bad;
			}
			do {
				expon = expon*10+c-'0';
				c = nextc();
			} while('0'<=c && c<='9');
		} else
			expon = 1;
		expon *= pow;
		pow = 1;
		peekc = c;
		if (div) {
			expon = -expon;
			div = 0;
		}
		u->u_val *= ipow(factor*units[i].u_val, expon);
		for (j=0; j!=NDIM; j++)
			u->u_dim[j] += units[i].u_dim[j]*expon;
	}
Bad:
	while (c!='\n' && c!=EOF)
		c = nextc();
	if (prompt!=NULL)
		goto Again;
	printf("line %d\n", lineno);
	return (1);
}

/*
 * Update units information by reading the
 * units file.
 */
void update(void)
{
	register char *name;
	register int i;
	register char *sstart, *send;
	register int bfd;

	fprintf(stderr, "Rebuilding %s from %s ...\n", bunamep, unamep);
	if ((fd = fopen(unamep, "r")) == NULL)
		err(1, "can't open unit file `%s'", ufile);
	setbuf(fd, inbuf);
	units = (UNIT *)alloc(NUNITS*sizeof(UNIT));
	sstart = alloc(0);
	for (nunits=0; nunits!=NUNITS; nunits++) {
		name = getname();
		for (i=0; i!=nunits; i++)
			if (strcmp(units[i].u_name, name) == 0)
				fprintf(stderr, "`%s' redefined, line %d\n",
					name, lineno);
		units[nunits].u_name = name;
		if (!getunit(&units[nunits], NULL))
			break;
	}
	send = alloc(0);
	if (!feof(fd))
		errx(1, "too many units");
	fclose(fd);
	/*
	 * Write out, if possible, binary
	 * information for faster response next time.
	 */
	if ((bfd = creat(bunamep, 0644)) >= 0) {
		hdr.h_magic = UMAGIC;
		hdr.h_nunits = nunits;
		hdr.h_ssize = send-sstart;
		if (write(bfd, &hdr, sizeof(hdr)) != sizeof(hdr))
			goto bad;
		if (write(bfd, sstart, hdr.h_ssize) != hdr.h_ssize)
			goto bad;
		for (i=0; i!=nunits; i++)
			units[i].u_name -= (long)sstart;/* Rel. address */
		write(bfd, units, nunits*sizeof(UNIT));
		for (i=0; i!=nunits; i++)
			units[i].u_name += (long)sstart;
	bad:
		close(bfd);
	}
}

void punit(UNIT *u)
{
	register int i;

	printf("%g", u->u_val);
	for (i=0; i!=NDIM; i++)
		if (u->u_dim[i] == 1)
			printf(" %s", units[i].u_name);
		else if (u->u_dim[i] > 0)
			printf(" %s+%d", units[i].u_name, u->u_dim[i]);
		else if (u->u_dim[i] < 0)
			printf(" %s-%d", units[i].u_name, -u->u_dim[i]);
	printf("\n");
}



/*
 * Initialise by reading in units information
 * either in binary or ascii form and updating
 * the binary information.
 */
void init(void)
{
	if (!binary())
		update();
	printf("%d units\n", nunits);
	peekc = EOF;
	if (uflag)
		exit(0);
}

int main(int argc, char *argv[])
{
	UNIT have, want;
	register int i;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	if (argc>1 && *argv[1]=='-') {
		if (argv[1][1]=='u' && argv[1][2]=='\0')
			uflag++;
		else {
			fprintf(stderr, "Usage: units [-u]\n");
			exit(1);
		}
	}
	init();
	setbuf(fd = stdin, inbuf);
Again:
	if (!getunit(&have, "You have: ")
	|| !getunit(&want, "You want: "))
		exit(0);
	for (i=0; i!=NDIM; i++)
		if (have.u_dim[i] != want.u_dim[i]) {
			printf("Conformability\n");
			punit(&have);
			punit(&want);
			goto Again;
		}
	printf("* %g\n/ %g\n", have.u_val/want.u_val, want.u_val/have.u_val);
	goto Again;
}
