/* This file contains routines for setting the terminal characteristics,
 * and for getting the termcap strings.
 *
 * $Revision: 41.3 $ $Date: 2003/04/21 13:08:43 $
 */

#include "header.h"

int beeplength;			/* Strlen of beep */
int wid;			/* The width of the screen (minus 1) */

 /* The termcap strings Wish uses */
char *bs, *nd, *cl, *cd, *up, *so, *se, *wbeep;

static char *termcapbuf;	/* Buffer to get termcap strings */

/* Wish keeps the ``normal'' terminal characteristics (i.e the ones Wish's
 * children see) in a structure, and restores the characteristics each
 * time it forks. The structure depends upon the OS we are being compiled
 * under.
 */

#ifdef USES_TCGETA
static struct termio tbuf, tbuf2;

#endif

#ifdef USES_SGTTY
static struct sgttyb tbuf, tbuf2;
static struct tchars sbuf, sbuf2;

# ifdef USES_MORESIG
static struct ltchars moresigc, moresigc2;
# endif
#endif

#ifdef USES_TERMIOS
static struct termios tbuf, tbuf2;

#endif


#ifdef DEBUG
/* Printctrl prints out a terminal sequence that Wish got
 * from termcap. Only used when debugging the ensure Wish
 * is actually getting the strings, and the right ones.
 */
static void
printctrl(name, str)
  char *name, *str;
{
  int i;

  prints("%s: ", name);
  for (i = 0; str[i]; i++)
    if (str[i] > 31)
      prints("%c",str[i]);
    else
    {
      prints("%c",'^');
      if (str[i] > 26)
	prints("%c",str[i] + 64);
      else
	prints("%c",str[i] + 96);
    }
  prints('\n');
}

#endif


/* There are 3 versions of getstty, setcooked and setcbreak,
 * defined by USES_TCGETA, USES_TERMIOS and USES_SGTTY
 */

#ifdef USES_TCGETA
/* Getstty: Get the current terminal structures for Wish */
void
getstty()
{
  if (ioctl(0, TCGETA, &tbuf))
    perror("ioctl in getstty");
  memcpy(&tbuf2, &tbuf, sizeof(tbuf));
}

/* Setcbreak: Set terminal to cbreak mode */
void
setcbreak()
{
  bool keepstty;

  if (EVget("Keepstty"))
    keepstty = TRUE;
  else
    keepstty = FALSE;

  if (!keepstty)
    getstty();
  tbuf2.c_lflag &= ~(ICANON | ECHO | ISIG); /* Turn off canonical input and echo */
  tbuf2.c_cc[4] = 1;		/* read 1 char before returning like CBREAK */
  if (ioctl(0, TCSETA, &tbuf2))
    perror("ioctl in setcbreak");
}

/* Setcooked: Set terminal to cooked mode */
void
setcooked()
{
  if (ioctl(0, TCSETA, &tbuf))
    perror("ioctl in setcooked");
}

#endif				/* USES_TCGETA */


#ifdef USES_TERMIOS
/* Getstty: Get the current terminal structures for Wish */
void
getstty()
{
  if (tcgetattr(0, &tbuf))
    perror("tcgetattr in getstty");
  memcpy(&tbuf2, &tbuf, sizeof(tbuf));
}

/* Setcbreak: Set terminal to cbreak mode */
void
setcbreak()
{
  int i;
  bool keepstty;

  if (EVget("Keepstty"))
    keepstty = TRUE;
  else
    keepstty = FALSE;

  if (!keepstty)
    getstty();
  /* Turn off canonical input and echo */
  tbuf2.c_lflag = tbuf2.c_lflag & (~ICANON) & (~ECHO);
  for (i=0; i< NCCS; i++) tbuf2.c_cc[i]=0;
  tbuf2.c_cc[VMIN] = 1;		/* read 1 char before returning like CBREAK */
  if (tcsetattr(0, TCSANOW, &tbuf2))
    perror("cbreak tcsetattr");
}

/* Setcooked: Set terminal to cooked mode */
void
setcooked()
{
  if (tcsetattr(0, TCSANOW, &tbuf))
    perror("cooked tcsetattr");
}

#endif				/* USES_TERMIOS */


#ifdef USES_SGTTY
/* Getstty: Get the current terminal structures for Wish */
void
getstty()
{
  if (ioctl(0, TIOCGETP, &tbuf))/* get the sgttyb struct */
    perror("ioctl2 in getstty");
  if (ioctl(0, TIOCGETC, &sbuf))/* get the tchars struct */
    perror("ioctl3 in getstty");
#ifdef USES_MORESIG
  bcopy(&sbuf, &sbuf2, sizeof(sbuf));	/* and copy them so we can change */
  bcopy(&tbuf, &tbuf2, sizeof(tbuf));
  if (ioctl(0, TIOCGLTC, &moresigc))	/* get the ltchars struct */
    perror("ioctl4 in getstty");
  bcopy(&moresigc, &moresigc2, sizeof(moresigc));
#else
  memcpy(&sbuf2, &sbuf, sizeof(sbuf));	/* and copy them so we can change */
  memcpy(&tbuf2, &tbuf, sizeof(tbuf));
#endif
}

/* Setcbreak: Set terminal to cbreak mode */
void
setcbreak()
{
  bool keepstty;

  if (EVget("Keepstty"))
    keepstty = TRUE;
  else
    keepstty = FALSE;

  if (!keepstty)
    getstty();
  /* setup terminal with ioctl calls */

  tbuf2.sg_flags |= CBREAK;	/* cbreak mode to get each char */
  tbuf2.sg_flags &= (~ECHO);	/* do not echo chars to screen */
  if (ioctl(0, TIOCSETP, &tbuf2))	/* put it back, modified */
    perror("ioctl1 su");
  sbuf2.t_intrc = (UNDEF);	/* no interrupt or quitting */
  /* sbuf2.t_quitc=(UNDEF); 		   Allow quit while debugging */
  sbuf2.t_eofc = (UNDEF);	/* or eof signalling */
  if (ioctl(0, TIOCSETC, &sbuf2))	/* put it back, modified */
    perror("ioctl2 scb");
#ifdef USES_MORESIG
  moresigc2.t_suspc = (UNDEF);	/* no stopping */
  moresigc2.t_dsuspc = (UNDEF);	/* or delayed stopping */
  moresigc2.t_rprntc = (UNDEF);	/* or reprinting */
  moresigc2.t_flushc = (UNDEF);	/* or flushing */
  moresigc2.t_werasc = (UNDEF);	/* or word erasing */
  moresigc2.t_lnextc = (UNDEF);	/* or literal quoting */
  if (ioctl(0, TIOCSLTC, &moresigc2))	/* put it back, modified */
    perror("ioctl3");
#endif
}


/* Setcooked: Set terminal to cooked mode */
void
setcooked()
{
  if (ioctl(0, TIOCSETP, &tbuf))
    perror("ioctl1 sd");
  if (ioctl(0, TIOCSETC, &sbuf))
    perror("ioctl2 sd");
#ifdef USES_MORESIG
  if (ioctl(0, TIOCSLTC, &moresigc))	/* set ltchars struct to be default */
    perror("ioctl3 in setcooked");
#endif
}

#endif				/* USES_SGTTY */


/* gettstring: Given the name of a termcap string, gets the string
 * and places it into loc. Returns 1 if ok, 0 if no string.
 * If no string, loc[0] is set to EOS.
 */
#ifdef PROTO
static bool gettstring(char *name, char **loc)
#else
static bool
gettstring(name, loc)
  char *name, **loc;
#endif
{
  char bp[50], *area = bp;

  if (tgetstr(name, &area) != NULL)
  {
    area = bp;
    while (isdigit(*area))
      area++;			/* Skip time delay chars */
    *loc = (char *) malloc((unsigned) strlen(area) + 2);
    if (!(*loc))
      return (FALSE);
    strcpy(*loc, area);
    return (TRUE);
  }
  else
    *loc = "";
  return (FALSE);
}


/* Terminal is called at the beginning of Wish to get the termcap
 * strings needed by the Command Line Editor. If they are not got,
 * or the wrong ones are found, the CLE will act strangely. Wish
 * should at this stage default to a dumber CLE.
 */
void
terminal()
{
  extern int beeplength;
  char *t, term[20];

/* set up cursor control sequences from termcap */

  t=EVget("TERM");
  if (!t) fatal("No termcap entry available");
  strncpy(term, t, 10);
  termcapbuf = (char *) malloc(2048);
  if (!termcapbuf)
    return;
  tgetent(termcapbuf, term);
  if (tgetflag("bs") == 1)
    bs = "\b";
  else
    gettstring("bc", &bs);
  if ((wid = tgetnum("co")) == -1)
    wid = 80;
  wid--;		/* this is to eliminate unwanted auto newlines */
  gettstring("cl", &cl);
  gettstring("cd", &cd);
  gettstring("nd", &nd);
  gettstring("up", &up);
  gettstring("so", &so);
  gettstring("se", &se);
  gettstring("bl", &wbeep);
  if (*wbeep == EOS)
    
    wbeep = "\007";
  beeplength = strlen(wbeep);
#ifdef DEBUG
  printctrl("bs", bs);
  printctrl("cl", cl);
  printctrl("cd", cd);
  printctrl("nd", nd);
  printctrl("up", up);
  printctrl("so", so);
  printctrl("se", se);
  printctrl("beep", wbeep);
#endif
  free(termcapbuf);
}
