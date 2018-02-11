/*
 *	termcap.c	V1.1	20/7/87		agc	Joypace Ltd
 *
 *	Copyright Joypace Ltd, London, UK, 1987. All rights reserved.
 *	This file may be freely distributed provided that this notice
 *	remains attached.
 *
 *	A public domain implementation of the termcap(3) routines.
 *
 *
 *
 *	 Klamer Schutte	      V1.2    Nov. 1988
 *
 *   - Can match multiple terminal names		 [tgetent]
 *   - Removal of **area assignments			 [tgetstr]
 *
 *	 Terrence W. Holm     V1.3    May, Sep, Oct.  1988
 *
 *  - Correct when TERM != name and TERMCAP is defined	 [tgetent]
 *  - Correct the comparison for the terminal name 	 [tgetent]
 *  - Correct the value of ^x escapes              	 [tgetstr]
 *  - Added %r to reverse row/column			 [tgoto]
 *  - Fixed end of definition test			 [tgetnum/flag/str]
 *
 *	 Terrence W. Holm     V1.4    Jan. 1989
 *
 *   - Incorporated Klamer's V1.2 fixes into V1.3
 *   - Added %d, (old %d is now %2)			 [tgoto]
 *   - Allow '#' comments in definition file		 [tgetent]
 *
 * FIXME:
 *	- support the use of some kind of < 1024 byte buffer
 *	  interface
 *	- avoid the use of stdio (state machine search for either
 *	  [start of file]name: or \nname:)
 */

#include <termcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char *capab;		/* the capability itself */


/*
 *	tgetnum - get the numeric terminal capability corresponding
 *	to id. Returns the value, -1 if invalid.
 */

int tgetnum(char *id)
{
    register char *cp = capab;

    if (cp == (char *) NULL || id == (char *) NULL)
	return (-1);

    for (;;) {
	while (*cp++ != ':') {
	    if (cp[-1] == '\0')
		return (-1);
	}
	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0 && cp[2] == '#') {
	    return (atoi(cp + 3));
        }
    }
}


/*
 *	tgetflag - get the boolean flag corresponding to id. Returns -1
 *	if invalid, 0 if the flag is not in termcap entry, or 1 if it is
 *	present.
 */

int tgetflag(char *id)
{
    register char *cp = capab;

    if (cp == (char *) NULL || id == (char *) NULL)
	return (-1);

    for (;;) {
	while (*cp++ != ':')
	    if (cp[-1] == '\0')
		return (0);

	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0)
	    return (1);
    }
}


/*
 *	tgetstr - get the string capability corresponding to id and place
 *	it in area (advancing area at same time). Expand escape sequences
 *	etc. Returns the string, or NULL if it can't do it.
 */

char *tgetstr(char *id, char **area)
{
    register char *cp = capab;
    register char *wsp = *area;	/* workspace pointer  */

    if (cp == (char *) NULL || id == (char *) NULL)
	return ((char *) NULL);

    for (;;) {
	while (*cp++ != ':')
	    if (cp[-1] == '\0')
		return ((char *) NULL);

	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0 && cp[2] == '=') {
	    for (cp += 3; *cp && *cp != ':'; wsp++, cp++)
		switch (*cp) {
		case '^':
		    *wsp = *++cp - '@';
		    break;

		case '\\':
		    switch (*++cp) {
		    case 'E':
			*wsp = '\033';
			break;
		    case 'n':
			*wsp = '\n';
			break;
		    case 'r':
			*wsp = '\r';
			break;
		    case 't':
			*wsp = '\t';
			break;
		    case 'b':
			*wsp = '\b';
			break;
		    case 'f':
			*wsp = '\f';
			break;
		    case '0':
		    case '1':
		    case '2':
		    case '3':
			{
			    int i;
			    int t = 0;
			    for (i = 0; i < 3 && isdigit(*cp); ++i, ++cp)
				t = t * 8 + *cp - '0';
			    *wsp = t;
			    cp--;
			    break;
			}
		    default:
			*wsp = *cp;
		    }
		    break;

		default:
		    *wsp = *cp;
		}

	    *wsp++ = '\0';

	    {
		char *ret = *area;
		*area = wsp;
		return (ret);
	    }
	}
    }				/* end for(;;) */
}

/*
 *	tgoto - given the cursor motion string cm, make up the string
 *	for the cursor to go to (destcol, destline), and return the string.
 *	Returns "OOPS" if something's gone wrong, or the string otherwise.
 */

char *tgoto(const char *cm, int destcol, int destline)
{
 	static char ret[24];
	char *rp = ret;
	int incr = 0;
	int argno = 0;
	int numval;

	for (; *cm; cm++) {
		if (*cm == '%') {
			switch (*++cm) {
				case 'i':
					incr = 1;
		  			break;
				case 'r':
					argno = 1;
		  			break;
				case '+':
					numval = (argno == 0 ? destline : destcol);
					*rp++ = numval + incr + *++cm;
					argno = 1 - argno;
					break;
				case '2':
					numval = (argno == 0 ? destline : destcol);
					numval = (numval + incr) % 100;
					*rp++ = '0' + (numval / 10);
					*rp++ = '0' + (numval % 10);
					argno = 1 - argno;
					break;
				case 'd':
					numval = (argno == 0 ? destline : destcol);
					numval = (numval + incr) % 1000;
					if (numval > 99)
						*rp++ = '0' + (numval / 100);
					if (numval > 9)
						*rp++ = '0' + (numval / 10) % 10;
					*rp++ = '0' + (numval % 10);
					argno = 1 - argno;
					break;
				case '%':
					*rp++ = '%';
					break;
				default:
					return("OOPS");
 			}
		}
		else
			*rp++ = *cm;
	}
	*rp = '\0';
	return ret;
}

/*
 *	tputs - put the string cp out onto the terminal, using the function
 *	outc. This should do padding for the terminal, but I can't find a
 *	terminal that needs padding at the moment...
 */

int tputs(const char *cp, int affcnt, int (*outc)(int ch))
{
    if (cp == (const char *) NULL)
	return (1);
    /* Do any padding interpretation - left null for MINIX just now */
    while (*cp)
	(*outc) (*cp++);
    return (1);
}

