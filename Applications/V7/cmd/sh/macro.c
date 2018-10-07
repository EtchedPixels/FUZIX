/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier.  All rights reserved.  */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

static CHAR quote;		/* used locally */
static CHAR quoted;		/* used locally */

static int getch(char);
static void comsubst(void);
static void flush(int);

static char *copyto(char endch)
{
	register CHAR c;

	while ((c = getch(endch)) != endch && c)
		pushstak(c | quote);
	zerostak();

	if (c != endch)
		error(badsub);
}

static int skipto(char endch)
{
	/* skip chars up to } */
	register char c;
	while ((c = readc()) && c != endch) {
		switch (c) {

		case SQUOTE:
			skipto(SQUOTE);
			break;

		case DQUOTE:
			skipto(DQUOTE);
			break;

		case DOLLAR:
			if (readc() == BRACE)
				skipto('}');
		}
	}
	if (c != endch) {
		error(badsub);
	}
}

static int getch(char endch)
{
	register char d;

retry:
	d = readc();
	if (!subchar(d))
		return (d);

	if (d == DOLLAR) {
		register int c;
		if ((c = readc(), dolchar(c))) {
			NAMPTR n = (NAMPTR) NIL;
			int dolg = 0;
			BOOL bra;
			register const char *argp;
			register const char *v;
			CHAR idb[2];
			char *id = idb;

			if ( (bra = (c == BRACE)) )
				c = readc();

			if (letter(c)) {
				argp = (STRING) relstak();
				while (alphanum(c)) {
					pushstak(c);
					c = readc();
				}
				zerostak();
				n = lookup(absstak(argp));
				setstak(argp);
				v = n->namval;
				id = (char *)n->namid;
				peekc = c | MARK;;
			} else if (digchar(c)) {
				*id = c;
				idb[1] = 0;
				if (astchar(c)) {
					dolg = 1;
					c = '1';
				}
				c -= '0';
				v = ((c == 0) ? (const char *)cmdadr :
				         (c <= dolc) ? dolv[c]
					 : (dolg = 0, NULL));
			} else if (c == '$') {
				v = pidadr;
			} else if (c == '!') {
				v = pcsadr;
			} else if (c == '#') {
				v = dolladr;
			} else if (c == '?') {
				v = exitadr;
			} else if (c == '-') {
				v = flagadr;
			} else if (bra) {
				error(badsub);
			} else {
				goto retry;
			}
			c = readc();

			if (!defchar(c) && bra)
				error(badsub);

			argp = 0;
			if (bra) {
				if (c != '}') {
					argp = (STRING) relstak();
					if ((v == 0) ^ (setchar(c)))
						copyto('}');
					else
						skipto('}');

					argp = absstak(argp);
				}
			} else {
				peekc = c | MARK;
				c = 0;
			}
			if (v) {
				if (c != '+') {
					for (;;) {
						while ( (c = *v++) )
							pushstak(c | quote);

						if (dolg == 0 || (++dolg > dolc))
							break;
						else {
							v = dolv[dolg];
							pushstak(SP | (*id == '*' ? quote : 0));
						}
					}
				}
			} else if (argp) {
				if (c == '?') {
					failed(id,
					       *argp ? argp : badparam);
				} else if (c == '=') {
					if (n)
						assign(n, argp);
					else
						error(badsub);
				}
                        } else if (flags & setflg) {
				failed(id, badparam);
			}
			goto retry;
		} else {
			peekc = c | MARK;
		}
	} else if (d == endch) {
		return (d);
	} else if (d == SQUOTE) {
		comsubst();
		goto retry;
	} else if (d == DQUOTE) {
		quoted++;
		quote ^= QUOTE;
		goto retry;
	}
	return d;
}

char *macro(char *as)
{
	/* Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	register BOOL savqu = quoted;
	register CHAR savq = quote;
	FILEHDR fb;

	push((void *)&fb);/*FIXME*/
	estabf(as);
	usestak();
	quote = 0;
	quoted = 0;
	copyto(0);
	pop();
	if (quoted && (stakbot == staktop)) {
		pushstak(QUOTE);
	}
	quote = savq;
	quoted = savqu;
	return (fixstak());
}

static void comsubst(void)
{
	/* command substn */
	FILEBLK cb;
	register char d;
	register STKPTR savptr = fixstak();

	usestak();
	while ((d = readc()) != SQUOTE && d)
		pushstak(d);

	{
		register char *argc;
		trim(argc = fixstak());
		push(&cb);
		estabf(argc);
	}
	{
		register TREPTR t =
		    makefork(FPOU, cmd(EOFSYM, MTFLG | NLFLG));
		int pv[2];

		/* this is done like this so that the pipe
		 * is open only when needed
		 */
		chkpipe(pv);
		initf(pv[INPIPE]);
		execute(t, 0, 0, pv);
		close(pv[OTPIPE]);
	}
	tdystak(savptr);
	staktop = movstr(savptr, stakbot);

	while ( (d = readc()) )
		pushstak(d | quote);

	await(0);

	while (stakbot != staktop) {
		if ((*--staktop & STRIP) != NL) {
			++staktop;
			break;
		}
	}
	pop();
}

#define CPYSIZ	512

void subst(int in, int ot)
{
	register char c;
	static6502 FILEBLK fb;
	register int count = CPYSIZ;

	push(&fb);
	initf(in);
	/* DQUOTE used to stop it from quoting */
	while ( (c = (getch(DQUOTE) & STRIP)) ) {
		pushstak(c);
		if (--count == 0) {
			flush(ot);
			count = CPYSIZ;
		}
	}
	flush(ot);
	pop();
}

static void flush(int ot)
{
	write(ot, stakbot, staktop - stakbot);
	if (flags & execpr)
		write(output, stakbot, staktop - stakbot);
	staktop = stakbot;
}
