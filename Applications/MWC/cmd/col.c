/*
 * Col(1). Virtual typewriter, performs motions physical typewriters cannot,
 * like reverse line feeds. Also filters control characters.
 * Two global variables, LineNo and ColNo, have the current line and column
 * numbers. Both start at zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	Unfetch(c)	Unfetched = (c)

#define	PAGESIZE	256	/* Default Page size. */
#define	IBUFSIZE	800	/* Number of possible columns. */
#define	EBUFSIZ		4	/* Increment size for buf in EXTRA struct. */

/*
 * Fetch returns the control characters \n, \b, \t, \r, HVT, VT, HLF, LF.
 * We choose HVT and HLF to make a switch statement more compact in Main().
 */
#define	OSTRK		037	/* Flag for overstruck chars. */
#define	ESC		033	/* Escape character. */
#define	SO		017	/* Shift out to alternate char set. */
#define	SI		016	/* Shift in from alternate char set. */
#define	HVT		014	/* Half vertical tab. */
#define	VT		013	/* Vertical tab, or Rev. Line Feed. */
#define	HLF		007	/* Half line feed. */
#define	LF		006	/* Line feed (but not return!). */
#define	EOT		004	/* End of line signal to InsChar. */
#define	BOT		003	/* Start of line signal to InsChar. */

#define	not		!
#define	or		||
#define	and		&&
#define	TRUE		1
#define	FALSE		0
#define	NOTREACHED	return

typedef uchar bool;

typedef struct extra {
	struct extra *Next;
	int Posn;
	uchar Howmany;
	char Ebuf[EBUFSIZ];
} EXTRA;

typedef struct {
	int Len;		/* Number of valid columns in Line. */
	char *Line;
	struct extra *Extra;
} LINE;


/*
 * Error Messages.
 */
char Usage[] = "Usage: col [-bdfx] [-pnum]";
char BackScroll[] = "Scrolling backwards over top of page window.";
char Confused[] = "Seem to have lost Overstruck characters.";

/*
 * Flags.
 */
bool Bflag = FALSE;		/* Command line option 'b'. */
bool Dflag = FALSE;		/* Command line option 'd'. */
bool Fflag = FALSE;		/* Command line option 'f'. */
bool Pflag = FALSE;		/* Command line option 'p'. */
bool Xflag = FALSE;		/* Command line option 'x'. */

/*
 * External Variables.
 */
char Ibuf[BUFSIZ];		/* Buffer for input lines. */

LINE *Page;			/* Page window. */
LINE *CurLine;			/* Ptr to *(Page[LineNo % PageSize]). */

int PageSize = PAGESIZE;	/* Actual size of Page. */
int Ibuflen;			/* Current length of line in Ibuf. */
int Unfetched;			/* storage for unfetched char */
int Top;			/* Line number of top of Page window. */
int Bottom;			/* Line number of bottom of Page window. */
int Wmark;			/* High-water-mark of Lineno. */
int LineNo;			/* Current line number in Page. */
int ColNo;			/* Current column number. */

void Warning(const char *cp)
{
	fputs(cp, stderr);
	putc('\n', stderr);
	return;
}

void Fatal(const char *cp)
{
	Warning(cp);
	exit(1);
}

void OOM(void)
{
	Fatal("Out of memory");
}

/*
 * Process command line arguments.
 */

void Aarghh(int ac, char *av[])
{
	register char *cp;
	register int c;

	while ((cp = *++av) != NULL) {
		if (*cp++ != '-') {
			Fatal(Usage);
			NOTREACHED;
		}
		while ((c = *cp++) != '\0')
			switch (c) {
			case 'b':
				Bflag = TRUE;
				break;
			case 'd':
				Dflag = TRUE;
				break;
			case 'f':
				Fflag = TRUE;
				break;
			case 'x':
				Xflag = TRUE;
				break;
			case 'p':
				Pflag = TRUE;
				if ((PageSize = 2 * atoi(cp)) <= 0)
					Fatal("Bad page length");
				break;
			default:
				Fatal(Usage);
				NOTREACHED;
			}
	}
}

void Init(void)
{
	register LINE *lp;

	lp = CurLine = Page = (LINE *) malloc(sizeof(LINE) * PageSize);
	if (lp == NULL)
		OOM();
	lp += PageSize - 1;
	while (lp-- > Page) {
		lp->Len = 0;
		lp->Line = NULL;
		lp->Extra = NULL;
	}
	Bottom = PageSize;
	return;
}


/*
 * Fetch grabs input characters and returns them after filtering.
 */
int Fetch(void)
{
	static int acset = 0;	/* alternate character set flag */
	register int c, c1;

	if ((c = Unfetched) != '\0') {
		Unfetched = '\0';
		return (c);
	}
	for (;;) {
		if ((c = getchar()) > ' ' && c < 0177)
			return (acset ? c | 0200 : c);
		switch (c) {
		case ' ':
		case '\t':
		case '\n':
		case '\b':
		case '\r':
		case VT:
		case EOF:
			return (c);
		case SO:
			acset = 1;
			continue;
		case SI:
			acset = 0;
			continue;
		case ESC:
			switch (c1 = getchar()) {
			case '7':
				return (VT);
			case '8':
				return (HVT);
			case '9':
				return (HLF);
			case 'B':
				return (LF);
			}
			ungetc(c1, stdin);
			continue;
		}
	}
}



/*
 * Handle overstruck characters.
 */
void Overstrike(int c)
{
	register EXTRA *ep;
	register EXTRA **epp;

	/*
	 * Find the right EXTRA struct in CurLine.
	 */
	epp = &CurLine->Extra;
	for (ep = *epp; ep != NULL; epp = &ep->Next, ep = *epp) {
		if (ep->Posn != ColNo)
			continue;
		/*
		 * We found it, check for overflow and add the char c.
		 */
		if (ep->Howmany % EBUFSIZ == 0)
			ep = *epp = (EXTRA *) realloc((char *) ep,
						      sizeof(EXTRA) +
						      ep->Howmany);
		if (ep == NULL)
			OOM();
		ep->Ebuf[ep->Howmany++] = c;
		return;
	}

	/*
	 * We didn't find it, so make it, and install the char c.
	 */
	ep = (EXTRA *) malloc(sizeof(EXTRA));
	if (ep == NULL)
		OOM();
	ep->Howmany = 1;
	ep->Posn = ColNo;
	ep->Ebuf[0] = c;
	ep->Next = CurLine->Extra;
	CurLine->Extra = ep;
	return;
}

/*
 * Insert the character c into Ibuf at column ColNo.
 */

void InsChar(int c)
{

	/*
	 * If (c == BOT) or (c == EOT) we open or close the line in Ibuf.
	 * Otherwise we are really adding a character to Ibuf.
	 */
	if (c == BOT) {
		register LINE *lp = CurLine;
		if ((Ibuflen = lp->Len) != 0) {
			strncpy(Ibuf, lp->Line, Ibuflen);
			free(lp->Line);
		}
		return;
	} else if (c == EOT) {
		register LINE *lp = CurLine;
		if ((lp->Len = Ibuflen) != 0) {
			lp->Line = malloc(Ibuflen);
			if (lp->Line == NULL)
				OOM();
			strncpy(lp->Line, Ibuf, Ibuflen);
		}
		return;
	}

	/*
	 * The case of appending a char to the end of Ibuf. Very common.
	 * Note that in this case Ibuf[ColNo] is virgin territory.
	 */
	if (ColNo == Ibuflen) {
		Ibuf[Ibuflen++] = c;
		return;
	}

	/*
	 * The case of adding a char beyond the end of Ibuf. We must pad the
	 * intervening space with spaces.
	 */
	if (ColNo > Ibuflen) {
		register char *ibuf = Ibuf;
		while (Ibuflen < ColNo)
			ibuf[Ibuflen++] = ' ';
		ibuf[Ibuflen++] = c;
		return;
	}


	/*
	 * The remaining case is adding a char into the interior of Ibuf. If
	 * the present char is a space or if Bflag is set we just insert c,
	 * otherwise we have to overstrike.
	 */
	{
		register char *cp = Ibuf + ColNo;
		register int c1;

		if (Bflag or(c1 = *cp) == ' ') {
			*cp = c;
			return;
		}
		if (c1 != OSTRK) {
			Overstrike(c1);
			*cp = OSTRK;
		}
		Overstrike(c);
	}
	return;
}

/*
 * Ostrikeout puts out all the characters overstruck in position ColNo in the
 * LINE lp. It pays attention to alternate character sets.
 */
bool Ostrikeout(LINE * lp, int col, bool acset)
{
	EXTRA *ep;

	/*
	 * Find the EXTRA struct for position 'col' and remove it from
	 * the EXTRA list.
	 */
	{
		register EXTRA **epp = &lp->Extra;
		register EXTRA *e;
		register int n = col;

		for (e = *epp; e != NULL; epp = &e->Next, e = *epp) {
			if (e->Posn != n)
				continue;
			*epp = e->Next;
			ep = e;
			goto FOUNDIT;
		}
		/*
		 * Didn't find it in the list.
		 */
		putchar(' ');
		Warning(Confused);
		return (acset);
	}

	/*
	 * Now that we found it, write out all the characters in ep->Ebuf,
	 * paying attention to alternate char sets, then free ep.
	 */
      FOUNDIT:
	{
		register int c;
		register bool ac = acset;
		register int count = ep->Howmany;
		register char *cp = ep->Ebuf;

		while (count-- > 0) {
			if ((c = *cp++) & 0200) {
				if (not ac) {
					ac = TRUE;
					putchar(SO);
				}
				putchar(c & ~0200);
			} else {
				if (ac) {
					ac = FALSE;
					putchar(SI);
				}
				putchar(c);
			}
			if (count > 0)
				putchar('\b');
		}
		free(ep);
		return (ac);
	}
}

/*
 * PutHalf() outputs the half-line lp with no vertical motion at the end.
 * Alternate character sets are handled here. Entabbing is handled by Tab.
 * Overstrikes are handled by Ostrikeout(), which also may have to handle
 * alternate character sets.
 */
void PutHalf(LINE * lp)
{
	register int c;
	register int colno;
	register bool acset = FALSE;

	/*
	 * Note that since lp->Len is the number of valid columns, the number
	 * of the last valid column is (lp->Len - 1). That's why the test is
	 * "<" instead of "<=".
	 */
	for (colno = 0; colno < lp->Len; ++colno)
		switch (c = lp->Line[colno]) {
		case OSTRK:
			acset = Ostrikeout(lp, colno, acset);
			break;
		case ' ':
			if (Xflag) {
				putchar(' ');
				break;
			}
			/* Entab white space. */
			c = colno;
			while (lp->Line[c] == ' ')
				if (++c % 8 == 0) {
					putchar('\t');
					colno = c;
				}
			while (colno < c) {
				putchar(' ');
				++colno;
			}
			--colno;
			break;
		default:
			if (c & 0200) {
				if (not acset) {
					acset = TRUE;
					putchar(SO);
				}
				putchar(c & ~0200);
			} else {
				if (acset) {
					acset = FALSE;
					putchar(SI);
				}
				putchar(c);
			}
			break;
		}
	if (acset)
		putchar(SI);
	free(lp->Line);
	lp->Len = 0;
	lp->Line = NULL;
	lp->Extra = NULL;
	return;
}

/*
 * Output one full line, which is possibly two half lines. All control for
 * Dflag and Fflag takes place here.
 */
void PutLine(int n)
{
	static char HCR[] = { '\r', ESC, '9', '\0' };	/* Half Crg. Return */
	register LINE *lp;

	lp = Page + n % PageSize;
	if (lp->Len != 0)
		PutHalf(lp);
	++lp;

	if (lp->Len == 0) {
		if (Dflag and not Fflag) {
			putchar('\n');
			putchar('\n');
		} else
			putchar('\n');
	} else {
		if (Fflag) {
			fputs(HCR, stdout);
			PutHalf(lp);
			fputs(HCR, stdout);
		} else {
			putchar('\n');
			PutHalf(lp);
			putchar('\n');
		}
	}
	return;
}

void VertMove(int c)
{
	static bool warnflag = FALSE;	/* Warn of move over top of page. */

	Unfetch(c);
	for (;;) {
		switch (c = Fetch()) {
		case VT:
			LineNo -= 2;
			break;
		case LF:
			LineNo += 2;
			break;
		case HVT:
			LineNo -= 1;
			break;
		case HLF:
			LineNo += 1;
			break;
		case '\n':
			ColNo = 0;
			LineNo += 2;
			break;
		default:
			Unfetch(c);
			CurLine = Page + LineNo % PageSize;
			return;
		}

		/*
		 * Have we scrolled over the top of the Page window?
		 */
		if (LineNo < Top)
			if (not warnflag) {
				Warning(BackScroll);
				warnflag = TRUE;
			}

		/*
		 * Update the water mark if needed.
		 */
		if (LineNo > Wmark)
			Wmark = LineNo;


		/*
		 * If we've scrolled past the bottom of the window then we
		 * put out the Top line and move the window down.
		 */
		if (Bottom <= LineNo) {
			PutLine(Top);
			Top += 2;
			Bottom += 2;
		}
	}
}

void Flush(void)
{
	register int i;

	for (i = Top; i <= Wmark; i += 2)
		PutLine(i);
	return;
}



int main(int ac, char *av[])
{
	register int c;

	Aarghh(ac, av);
	Init();
	InsChar(BOT);
	while ((c = Fetch()) != EOF)
		switch (c) {
		case '\b':
			if (ColNo > 0)
				--ColNo;
			break;
		case '\r':
			ColNo = 0;
			break;
		case '\t':
			ColNo = (ColNo & ~07) + 8;
			break;
		case '\n':
			ColNo = 0;
		case VT:
		case LF:
		case HVT:
		case HLF:
			InsChar(EOT);	/* Close current line. */
			VertMove(c);
			InsChar(BOT);	/* Open current line. */
			break;
		default:
			if (c != ' ')
				InsChar(c);
			++ColNo;
			break;
		}
	InsChar(EOT);		/* Close current line. */
	Flush();
	return (0);
}
