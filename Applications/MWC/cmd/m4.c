#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define	TMPFILE	"/tmp/m4%dxxxxxx"
#define	TMPMIN	6

#define	isletter(c)		(isalpha(c)||c=='_')
#define	BQUOTE	'`'
#define	EQUOTE	'\''
#define	HASHSZ	199
#define	STRBLK	16		/* block size for dynamic string storage */
#if STRBLK < 12
need room for decimal rep of long int
#endif
 enum { FUNC, MACD, DSKF, MSTR };
enum { EX, MY, DV, MD, AD, SB, EQ, NE, LE, GE, LT, GT, AN, OR, RP, NL };

typedef struct {		/* for storing variable-length strings */
	int s_refc;		/* references--when zero space is freed */
	int s_hash;		/* raw hash value (sum of chars in body) */
	char *s_body;		/* start of contents */
	char *s_next;		/* end of contents */
	char *s_last;		/* end of storage */
} STRING;

typedef union ifr {		/* input-source stack frame */
	struct {
		union ifr *i_back;
		char i_cbuf;	/* unget buffer for lookahead */
		int i_type;	/* if type is MSTRing */
		STRING *i_pstr;
		char *i_pchr;	/* next char */
	} ifb;
	struct {
		union ifr *i_back;
		char i_cbuf;
		int i_type;	/* if type is DSKFile */
		FILE *i_fp;
	} ifdk;
} IFRAME;

typedef struct fstk {		/* file information stack frame */
	struct fstk *f_back;
	STRING *f_name;		/* pointer to name (NULL if stdin) */
	int f_flag;		/* flag = 0 until EOF is reached */
	int f_line;		/* line counter */
} FFRAME;

typedef struct ofr {		/* argument collection stack frame */
	struct ofr *o_back;
	STRING *o_pstr;
} OFRAME;

typedef struct ent {		/* symbol table entry */
	struct ent *e_next;
	int e_type;		/* FUNC or MACD */
	union {
		void (*e_pfun) (STRING ** s);
		STRING *e_pstr;
	} e_at;
	STRING *e_name;
} ENTRY;

/*
 * output file info
 */
struct {
	char *name;
	FILE *fp;
} outfile[10] = { {
NULL, stdout}};

/*
 * op table for eval
 */
struct opdata {
	int optype;
	char keychar;
	char secchar;
	int prec;
} optable[] = {
#define	SI	8		/* precedence of unary + and - */
	{
	EX, '^', '\0', 7}, {
	EX, '*', '*', 7}, {
	MY, '*', '\0', 6}, {
	DV, '/', '\0', 6}, {
	MD, '%', '\0', 6}, {
	AD, '+', '\0', 5}, {
	SB, '-', '\0', 5}, {
	EQ, '=', '=', 4}, {
	GE, '>', '=', 4}, {
	GT, '>', '\0', 4}, {
	NE, '!', '=', 4}, {
	LE, '<', '=', 4}, {
	LT, '<', '\0', 4},
#define	NT	3		/* precedence of unary ! */
	{
	AN, '&', '&', 2}, {
	AN, '&', '\0', 2}, {
	OR, '|', '|', 1}, {
	OR, '|', '\0', 1}, {
	RP, ')', '\0', 10}, {
	NL, '\0', '\0', 0},
#define	LP	0		/* precedence of open paren */
};

ENTRY *e_root[HASHSZ];		/* pointers to symbol table hash buckets */
OFRAME *ostkptr;		/* output stack pointer */
IFRAME *istkptr;		/* input stack pointer */
FFRAME *fstkptr;		/* file info stack pointer */
FILE *offp = stdout;		/* current output file pointer */
int ofnum;			/* current diversion number */
int lstdchr = '\n';		/* last char from stdin */
int single;			/* single argument flag */
int dnlflag;			/* delete to newline flag */

char bqt = BQUOTE;
char eqt = EQUOTE;

/* VARARGS */
void errorp(int f, const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	
	fprintf(stderr, "m4: ");
	if (fstkptr != NULL) {
		if (fstkptr->f_name != NULL)
			fprintf(stderr, "%s: ", fstkptr->f_name->s_body);
		fprintf(stderr, "%d: ", fstkptr->f_line);
	}
	vfprintf(stderr, s, ap);
	putc('\n', stderr);
	if (f) {
		while (lstdchr != EOF && lstdchr != '\n')
			lstdchr = getchar();
		exit(1);
	}
	va_end(ap);
}

char *alloc(int n)
{
	char *x;

	if ((x = malloc(n)) != NULL)
		return (x);
	errorp(1, "out of space");
}

STRING *makestr(void)
{
	register STRING *a;

	a = (STRING *) alloc(sizeof(STRING));
	a->s_body = a->s_next = alloc(STRBLK);
	a->s_last = a->s_body + STRBLK - 1;
	a->s_refc = 1;
	a->s_hash = 0;
	*a->s_next = '\0';
	return (a);
}

void decstr(STRING * a)
{
	if (a == NULL)
		return;
	if (--a->s_refc)
		return;
	free(a->s_body);
	free(a);
}

int cmpstr(STRING * a, STRING * b)
{
	if (a == NULL && b == NULL)
		return (1);
	if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
		return (0);
	return (!strcmp(a->s_body, b->s_body));
}

void appendstr(STRING * a, char c)
{
	register char *r, *s, *t;
	int size;

	if (a->s_next < a->s_last) {
		*a->s_next++ = c;
		*a->s_next = '\0';
	} else {
		r = s = alloc(size =
			      a->s_last - (t = a->s_body) + 1 + STRBLK);
		while ((*s++ = *t++) != 0);
		*(a->s_next = s) = '\0';
		*--s = c;
		free(a->s_body);
		a->s_last = (a->s_body = r) + size - 1;
	}
	a->s_hash += c;
}

void pushinp(char t)
{
	IFRAME *itemp;

	itemp = istkptr;
	istkptr = (IFRAME *) alloc(sizeof(IFRAME));
	istkptr->ifb.i_back = itemp;
	istkptr->ifb.i_type = t;
	istkptr->ifb.i_cbuf = '\0';
}

int pushfile(char *s)
{
	FILE *fp;
	register STRING *a = NULL;
	register FFRAME *ftemp = fstkptr;
	char c;

	if (ftemp != NULL && ftemp->f_flag) {
		decstr(ftemp->f_name);
		fstkptr = ftemp->f_back;
		free(ftemp);
	}
	if (strcmp(s, "-") == 0)
		fp = stdin;
	else if ((fp = fopen(s, "r")) == NULL)
		return (0);
	pushinp(DSKF);
	istkptr->ifdk.i_fp = fp;
	ftemp = (FFRAME *) alloc(sizeof(FFRAME));
	ftemp->f_back = fstkptr;
	ftemp->f_line = 1;
	ftemp->f_flag = 0;
	if (fp != stdin && !(fstkptr == NULL && single)) {
		a = makestr();
		while ((c = *s++) != 0)
			appendstr(a, c);
	}
	ftemp->f_name = a;
	fstkptr = ftemp;
	return (1);
}

void pushstr(STRING * a)
{
	if (a == NULL || *a->s_body == '\0')
		return;
	pushinp(MSTR);
	istkptr->ifb.i_pstr = a;
	istkptr->ifb.i_pchr = a->s_body;
	++a->s_refc;
}

void pushnum(long x)
{
	register STRING *a;

	a = makestr();
	sprintf(a->s_body, "%ld", x);
	pushstr(a);
	decstr(a);
}

int popinp(void)
{
	IFRAME *itemp;

	itemp = istkptr;
	switch (istkptr->ifb.i_type) {
	case MSTR:
		decstr(istkptr->ifb.i_pstr);
		break;
	case DSKF:
		if (istkptr->ifdk.i_fp != stdin)
			fclose(istkptr->ifdk.i_fp);
		fstkptr->f_flag = 1;
		break;
	}
	istkptr = istkptr->ifb.i_back;
	free(itemp);
	return (istkptr != NULL);
}

void pushout(STRING * b)
{
	OFRAME *otemp;

	otemp = (OFRAME *) alloc(sizeof(OFRAME));
	otemp->o_back = ostkptr;
	otemp->o_pstr = b;
	ostkptr = otemp;
	++b->s_refc;
}

void popout(void)
{
	OFRAME *otemp;

	otemp = ostkptr;
	ostkptr = ostkptr->o_back;
	decstr(otemp->o_pstr);
	free(otemp);
}

void macro(STRING * ps, STRING ** pps)
{
	STRING *x;
	register char *b, *v;

	x = makestr();
	b = ps->s_body;
	while (*b) {
		if (*b != '$')
			appendstr(x, *b++);
		else if (!isdigit(*++b))
			appendstr(x, '$');
		else if ((ps = pps[*b++ - '0']) != 0)
			for (v = ps->s_body; *v; v++)
				appendstr(x, *v);
	}
	pushstr(x);
	decstr(x);
}


void outc(char c)
{
	if (offp != NULL)
		putc(c, offp);
}

void outputc(char c)
{
	if (ostkptr != NULL)
		appendstr(ostkptr->o_pstr, c);
	else
		outc(c);
}

void outputs(char *s)
{
	register char *t, c;

	for (t = s; (c = *t++) != 0; outputc(c));
}

int nxch(void)
{
	register int c = 0;
	FFRAME *ftemp;

	if (istkptr == NULL)
		return ('\0');
	if ((ftemp = fstkptr)->f_flag) {
		decstr(ftemp->f_name);
		fstkptr = ftemp->f_back;
		free(ftemp);
	}
	switch (istkptr->ifb.i_type) {
	case DSKF:
		if ((c = istkptr->ifdk.i_cbuf) != 0)
			istkptr->ifdk.i_cbuf = '\0';
		else
			c = getc(istkptr->ifdk.i_fp);
		if (istkptr->ifdk.i_fp == stdin)
			lstdchr = c;
		if (c == EOF)
			c = '\0';
		else if (c == '\n') {
			if (fstkptr->f_flag)
				++fstkptr->f_back->f_line;
			else
				++fstkptr->f_line;
		}
		break;
	case MSTR:
		if ((c = istkptr->ifb.i_cbuf) != 0)
			istkptr->ifb.i_cbuf = '\0';
		else
			c = *istkptr->ifb.i_pchr++;
		break;
	}
	if (c == '\0')
		return (popinp()? nxch() : c);
	else if (!dnlflag)
		return (c);
	if (c == '\n')
		dnlflag = 0;
	return (nxch());
}

ENTRY *find(STRING * a)
{
	register ENTRY *e;
	register int hash;

	hash = a->s_hash;
	for (e = e_root[hash % HASHSZ]; e != NULL; e = e->e_next)
		if (e->e_name->s_hash == hash
		    && strcmp(e->e_name->s_body, a->s_body) == 0)
			return (e);
	return (NULL);
}

int process(int pct)
{
	char lc = '\0';
	register int c, qct = 0;
	int i;
	STRING *a[10];
	register STRING *b;
	ENTRY *e;

	for (c = nxch(); pct && isspace(c); c = nxch());
	while (c != '\0') {
		if (qct)
			if (c == eqt) {
				if (--qct)
					outputc(c);
			} else {
				if (c == bqt)
					++qct;
				outputc(c);
		} else if (c == bqt)
			++qct;
		else if ((c == ')' && pct && !--pct)
			 || (c == ',' && pct == 1))
			return (c);
		else if (isletter(c) && !isletter(lc) && !isdigit(lc)) {
			b = makestr();
			do {
				appendstr(b, c);
				lc = c;
				c = nxch();
			} while (isletter(c) || isdigit(c));
			if ((e = find(b)) == NULL) {
				outputs(b->s_body);
				decstr(b);
				continue;
			}
			for (i = 9; i; a[i--] = NULL);
			a[0] = b;
			if (c != '(') {
				istkptr->ifb.i_cbuf = c;
				if (c == '\n'
				    && istkptr->ifb.i_type == DSKF) {
					if (fstkptr->f_flag)
						--fstkptr->f_back->f_line;
					else
						--fstkptr->f_line;
				}
			} else
				do {
					pushout(b = makestr());
					c = process(1);
					if (i++ < 9 && strlen(b->s_body))
						a[i] = b;
					else
						decstr(b);
					popout();
				} while (c != ')');
			if (e->e_type == MACD)
				macro(e->e_at.e_pstr, a);
			else
				(*e->e_at.e_pfun) (a);
			for (i = 0; i < 10; i++)
				decstr(a[i]);
			c = '\0';
		} else {
			if (c == '(' && pct)
				++pct;
			outputc(c);
		}
		lc = c;
		c = nxch();
	}
	if (pct)
		errorp(1, "unexpected EOF");
	return (c);
}

void buildin(char *s, void (*f) (STRING ** p))
{
	STRING *a;
	register int hash;
	register ENTRY *e;

	a = makestr();
	while (*s)
		appendstr(a, *s++);
	hash = a->s_hash % HASHSZ;
	e = (ENTRY *) alloc(sizeof(ENTRY));
	e->e_next = e_root[hash];
	e->e_type = FUNC;
	e->e_at.e_pfun = f;
	e->e_name = a;
	e_root[hash] = e;
}

void mchangequote(STRING ** pps)
{
	bqt = pps[1] ? *pps[1]->s_body : BQUOTE;
	eqt = pps[2] ? *pps[2]->s_body : EQUOTE;
}

void mdefine(STRING ** pps)
{
	register ENTRY *e;
	register char *s;
	register int hash;
	int c;
	static char illmac[] = "illegal macro name: %s";

	if (pps[1] == NULL || !isletter(*pps[1]->s_body)) {
		errorp(0, illmac, pps[1] != NULL ? pps[1]->s_body : NULL);
		return;
	}
	s = pps[1]->s_body + 1;
	while ((c = *s++) != 0)
		if (!(isletter(c) || isdigit(c))) {
			errorp(0, illmac, pps[1]->s_body);
			return;
		}
	if ((e = find(pps[1])) != NULL) {
		if (e->e_type == MACD)
			decstr(e->e_at.e_pstr);
	} else {
		e = (ENTRY *) alloc(sizeof(ENTRY));
		e->e_next = e_root[hash = pps[1]->s_hash % HASHSZ];
		e->e_name = pps[1];
		++pps[1]->s_refc;
		e_root[hash] = e;
	}
	e->e_type = MACD;
	if (pps[2]) {
		e->e_at.e_pstr = pps[2];
		++pps[2]->s_refc;
	} else
		e->e_at.e_pstr = makestr();
}

void mdivert(STRING ** pps)
{
	char *fn;
	int fd;

	if (pps == NULL)
		ofnum = 0;
	else
		ofnum = (pps[1] != NULL) ? atoi(pps[1]->s_body) : 0;
	if (ofnum > 0 && ofnum <= 9) {
		if (outfile[ofnum].fp == NULL) {
			outfile[ofnum].name = fn = alloc(15);
			sprintf(fn, TMPFILE, ofnum);
			fd = mkstemp(fn);
			if (fd == -1
			    || (outfile[ofnum].fp =
				fdopen(fd, "w")) == NULL)
				errorp(1, "m4: /tmp open error\n");
		}
	}
	if (ofnum >= 0 && ofnum <= 9)
		offp = outfile[ofnum].fp;
	else
		offp = NULL;
}

void mdivnum(STRING ** unused)
{
	pushnum((long) ofnum);
}

void mdnl(STRING ** unused)
{
	dnlflag = 1;
}

void outdef(ENTRY * e)
{
	if (e->e_type == MACD) {
		outputc(bqt);
		outputs(e->e_at.e_pstr->s_body);
		outputc(eqt);
	}
}

void mdumpdef(STRING ** pps)
{
	register ENTRY *e;
	register int i, f = 0;
	int hash;

	for (i = 1; i <= 9; ++i)
		if (pps[i] != NULL) {
			if ((e = find(pps[i])) != NULL)
				outdef(e);
			f = 1;
		}
	if (!f)
		for (hash = 0; hash < HASHSZ; ++hash)
			for (e = e_root[hash]; e; e = e->e_next) {
				outputc(bqt);
				outputs(e->e_name->s_body);
				outputc(eqt);
				outputc('\t');
				outdef(e);
				outputc('\n');
			}
}

void merrprint(STRING ** pps)
{
	register int i;

	for (i = 1; i <= 9; i++)
		if (pps[i])
			fprintf(stderr, pps[i]->s_body);
}

long calc(int pr, char **ps)
{
	register struct opdata *opptr;
	register char c;
	char *s;
	long val1, val2, l;
	int oplength;

	for (s = *ps; isspace(*s); ++s);
	if (isdigit(c = *s++))
		for (val1 = c - '0'; isdigit(c = *s); ++s)
			val1 = 10 * val1 + c - '0';
	else
		switch (c) {
		case '+':
			val1 = calc(SI, &s);
			break;
		case '-':
			val1 = -calc(SI, &s);
			break;
		case '!':
			val1 = !calc(NT, &s);
			break;
		case '(':
			val1 = calc(LP, &s);
			break;
		default:
			if (c == '\0')
				errorp(0, "eval: missing value");
			else
				errorp(0, "eval: invalid expression");
			s = NULL;
		}
	if ((*ps = s) == NULL)
		return (0);
	for (;;) {
		while (s != NULL && isspace(c = *s))
			++s;
		oplength = 1;
		for (opptr = optable; s != NULL; ++opptr)
			if (c != opptr->keychar) {
				if (opptr->keychar == '\0')
					s = NULL;
			} else if (opptr->secchar == '\0') {
				break;
			} else if (opptr->secchar == *(s + 1)) {
				++oplength;
				break;
			}
		if ((*ps = s) == NULL) {
			errorp(0, "eval: missing or unknown operator");
			return (0);
		}
		if (opptr->prec <= pr)
			return (val1);
		*ps = s += oplength;
		if (c == ')')
			return (val1);
		val2 = calc(opptr->prec, &s);
		if ((*ps = s) == NULL)
			return (0);
		switch (opptr->optype) {
		case EX:
			if (val2 < 0)
				val1 = 0;
			else {
				for (l = 1; val2; --val2)
					l *= val1;
				val1 = l;
			}
			break;
		case MY:
			val1 *= val2;
			break;
		case DV:
			val1 /= val2;
			break;
		case MD:
			val1 %= val2;
			break;
		case AD:
			val1 += val2;
			break;
		case SB:
			val1 -= val2;
			break;
		case EQ:
			val1 = (val1 == val2);
			break;
		case NE:
			val1 = (val1 != val2);
			break;
		case GE:
			val1 = (val1 >= val2);
			break;
		case LE:
			val1 = (val1 <= val2);
			break;
		case GT:
			val1 = (val1 > val2);
			break;
		case LT:
			val1 = (val1 < val2);
			break;
		case AN:
			val1 = (val1 && val2);
			break;
		case OR:
			val1 = (val1 || val2);
			break;
		}
	}
}

void meval(STRING ** pps)
{
	char *s;

	if (pps[1] == NULL)
		pushnum((long) 0);
	else {
		s = pps[1]->s_body;
		pushnum(calc(0, &s));
	}
}

void mifdef(STRING ** pps)
{
	if (pps[1] && find(pps[1])) {
		pushstr(pps[2]);
	} else
		pushstr(pps[3]);
}

int moreargs(STRING ** pps, int n)
{
	register int i;

	for (i = n; i <= 9; i++)
		if (pps[i] != NULL)
			return (1);
	return (0);
}

void mifelse(STRING ** pps)
{
	if (cmpstr(pps[1], pps[2]))
		pushstr(pps[3]);
	else if (moreargs(pps, 5))
		if (cmpstr(pps[4], pps[5]))
			pushstr(pps[6]);
		else if (moreargs(pps, 8))
			if (cmpstr(pps[7], pps[8]))
				pushstr(pps[9]);
			else
				return;
		else
			pushstr(pps[7]);
	else
		pushstr(pps[4]);
}

int doinclude(STRING ** pps)
{
	return (pushfile((pps[1] != NULL) ? pps[1]->s_body : NULL));
}

void msinclude(STRING ** pps)
{
	doinclude(pps);
}

void minclude(STRING ** pps)
{
	if (doinclude(pps))
		return;
	errorp(1, "cannot open %s",
	       pps[1] != NULL ? pps[1]->s_body : NULL);
}

void mincr(STRING ** pps)
{
	pushnum((long) ((pps[1] != NULL) ? atol(pps[1]->s_body) + 1 : 1));
}

void mdecr(STRING ** pps)
{
	pushnum((long) ((pps[1] != NULL) ? atol(pps[1]->s_body) - 1 : -1));
}

void mindex(STRING ** pps)
{
	register char *pc, *pf;
	register int ln;
	long v;

	if (pps[2] == NULL)
		v = 0;
	else if (pps[1] == NULL)
		v = -1;
	else {
		pc = pps[1]->s_body;
		ln = strlen(pf = pps[2]->s_body);
		while ((pc = index(pc, *pf)) != NULL
		       && strncmp(pc, pf, ln))
			++pc;
		v = (pc != NULL) ? (long) (pc - pps[1]->s_body) : -1;
	}
	pushnum(v);
}

void mlen(STRING ** pps)
{
	pushnum((long)
		((pps[1] != NULL) ? pps[1]->s_next - pps[1]->s_body : 0));
}

/*

Removed as any implementaton of this is implicitly insecure
void mmaketemp(STRING ** pps)
{
	register char *pc;
	if (pps[1] == NULL || strlen(pc = pps[1]->s_body) < TMPMIN)
		return;
	mktemp(pc);
	pushstr(pps[1]);
}
*/

void msubstr(STRING ** pps)
{
	register char *pc, *pb, *pe;
	int len, n, s;
	STRING *a;

	if (pps[1] == NULL)
		return;
	len = strlen(pc = pps[1]->s_body);
	if (pps[2] == NULL) {
		n = (pps[3] != NULL) ? atoi(pps[3]->s_body) : len;
		s = (n > 0) ? 0 : -1;
	} else {
		s = atoi(pps[2]->s_body);
		n = (pps[3] != NULL) ? atoi(pps[3]->s_body) : (s >
							       0) ? len :
		    -len;
	}
	if (n == 0)
		return;
	pe = pc + len;
	if (s < 0)
		s += len;
	if (n < 0) {
		s += n + 1;
		n = -n;
	}
	a = makestr();
	for (pb = pc + s; n--; ++pb)
		if (pb >= pc && pb <= pe)
			appendstr(a, *pb);
	pushstr(a);
	decstr(a);
}

void msyscmd(STRING ** pps)
{
	if (pps[1])
		system(pps[1]->s_body);
}

void mtranslit(STRING ** pps)
{
	register char *pc, *pt, *pr;
	char c = '\0';
	STRING *a;

	if (pps[1] == NULL || pps[2] == NULL)
		return;
	a = makestr();
	pc = pps[1]->s_body;
	do {
		pt = pps[2]->s_body;
		pr = (pps[3] != NULL) ? pps[3]->s_body : &c;
		do {
			if (*pc == *pt)
				break;

			if (*pr)
				++pr;
		} while (*++pt);
		if (*pt) {
			if (*pr)
				appendstr(a, *pr);
		} else
			appendstr(a, *pc);
	} while (*++pc);
	pushstr(a);
	decstr(a);
}

void mundefine(STRING ** pps)
{
	register ENTRY *e, *ep = NULL;
	register int hash;

	if (pps[1] == NULL)
		return;
	hash = pps[1]->s_hash;
	for (e = e_root[hash % HASHSZ]; e != NULL; e = e->e_next)
		if (e->e_name->s_hash == hash
		    && strcmp(e->e_name->s_body, pps[1]->s_body) == 0)
			break;
		else
			ep = e;
	if (e == NULL)
		return;
	if (e->e_type == MACD)
		decstr(e->e_at.e_pstr);
	decstr(e->e_name);
	if (ep != NULL)
		ep->e_next = e->e_next;
	else
		e_root[hash % HASHSZ] = e->e_next;
	free(e);
}

void undiv(int n)
{
	register int c;
	register FILE *fp;

	if (n > 0 && n <= 9 && (fp = outfile[n].fp) != NULL && ofnum != n) {
		fclose(fp);
		if ((fp = fopen(outfile[n].name, "r")) == NULL)
			errorp(1, "cannot open %s", outfile[n].name);
		while ((c = getc(fp)) != EOF)
			outc(c);
		fclose(fp);
		unlink(outfile[n].name);
		free(outfile[n].name);
		outfile[n].name = NULL;
		outfile[n].fp = NULL;
	}
}

void mundivert(STRING ** pps)
{
	register int i, f = 0;

	if (pps)
		for (i = 1; i <= 9; i++)
			if (pps[i]) {
				undiv(atoi(pps[i]->s_body));
				f = 1;
			}
	if (pps == NULL || f == 0)
		for (i = 1; i <= 9; i++)
			undiv(i);
}

int main(int argc, char *argv[])
{
	buildin("changequote", mchangequote);
	buildin("decr", mdecr);
	buildin("define", mdefine);
	buildin("divert", mdivert);
	buildin("divnum", mdivnum);
	buildin("dnl", mdnl);
	buildin("dumpdef", mdumpdef);
	buildin("errprint", merrprint);
	buildin("eval", meval);
	buildin("ifdef", mifdef);
	buildin("ifelse", mifelse);
	buildin("include", minclude);
	buildin("incr", mincr);
	buildin("index", mindex);
	buildin("len", mlen);
/*	buildin("maketemp", mmaketemp); */
	buildin("sinclude", msinclude);
	buildin("substr", msubstr);
	buildin("syscmd", msyscmd);
	buildin("translit", mtranslit);
	buildin("undefine", mundefine);
	buildin("undivert", mundivert);

	single = (argc == 2);
	if (argc > 1) {
		while (--argc)
			if (pushfile(*++argv))
				process(0);
			else
				errorp(0, "cannot open %s", *argv);
	} else {
		pushfile("-");
		process(0);
	}
	mdivert(NULL);
	mundivert(NULL);
	exit(0);
}
