/* List of commands defined for the command line editor.
 * We only use the ctrl chars, so that people can use
 * the editor with 8-bit ascii characters.
 *
 * $Revision: 41.3 $ $Date: 2003/04/21 13:08:43 $
 */

#include "header.h"

#define MARK		  0	/* Save position, make a mark */
#define START		  1	/* Go to start of line */
#define BAKCH		  2	/* Go back one character */
#define INT		  3	/* Interrupt this task */
#define DELCH		  4	/* Delete the current character */
#define END		  5	/* Goto the end of the line */
#define FORCH		  6	/* Go forward one char */
#define BEEP		  7	/* Simply ring the bell */
#define BKSP		  8	/* Backspace over previous character */
#define COMPLETE	  9	/* Complete the current word */
#define FINISH		 10	/* Finish and execute the line */
#define KILLEOL		 11	/* Kill from cursor to end of line */
#define CLREDISP	 12	/* Clear screen & redisplay line */
#define NL		 13	/* Same as FINISH */
#define NEXTHIST	 14	/* Step forward in history */
#define OVERWRITE	 15	/* Toggle insert/overwrite mode */
#define BACKHIST	 16	/* Step backward in history */
#define XON		 17	/* Resume tty output */
#define REDISP		 18	/* Redisplay the line */
#define XOFF		 19	/* Stop tty output */
#define TRANSPCH	 20	/* Transpose current & previous characters */
#define KILLALL		 21	/* Kill the whole line */
#define QUOTE		 22	/* Quote next character literally */
#define DELWD		 23	/* Delete word backwards */
#define GOMARK		 24	/* Goto a mark */
#define YANKLAST	 25	/* Yank the previous word into a buffer */
#define SUSP		 26	/* Suspend process */

/* A Gap here for	 27	This can be filled as needed */
/* A Gap here for	 28	This can be filled as needed */

#define MODEON		 29	/* Include bindings with mode bit on */
#define MODEOFF		 30	/* Exclude bindings with mode bit on */
#define QUOTEUP		 31	/* Turn the following char's msb on */

#define DEL		127	/* Same as BKSP */

#define MATCHPART	129	/* Match a previous partial command */
#define BAKWD		130	/* Go backwards one word */
#define DELWDF		131	/* Delete word forwards */
#define FORWD		132	/* Go forwards one word */
#define GETHELP		133	/* Get help on a word */
#define PUT		134	/* Insert buffer on the line */
#define YANKNEXT	135	/* Yank the next word into the buffer */
#define SEARCHF		136	/* Search forward for next typed character */
#define SEARCHB		137	/* Search backwards for next typed character */

#define isctrl(x) (((x+1)&0x7f)<33)

/* Strip takes the line, and removes leading spaces. If the first non-space
 * character is a hash, it returns 1. This should also remove trailing
 * comments; I might just move the whole thing into meta_1.
 */
#ifdef PROTO
static int strip(uchar *line)
#else
static int
strip(line)
  uchar *line;
#endif
{
  int i, nosave = 0;

  for (i = 0; line[i] == ' '; i++);
  if (line[i] == '#')
  { nosave = 1; i++; }
  if (i) strcpy((char *) line, (char *) &line[i]);
  return (nosave);
}

bool Msb;			/* Is var Msb defined? */
#ifndef NO_COMLINED
uchar yankbuf[512];		/* Buffer used when yanking words */
uchar *wordterm;		/* Characters that terminate words */
extern int wid;			/* The width of the screen (minus 1) */

/* A quick blurb on the curs[] structure.
 * Curs[0] holds the column pos'n of the cursor, curs[1] holds the # of
 * lines the cursor is below the prompt line. e.g (0,0) is the first char
 * of the prompt, (25,3) is in column 25, 3 lines below the prompt line.
 */
int curs[2];


/* Go moves the cursor to the position (vert,hor), and updates the cursor */
#ifdef PROTO
static void go(int hor, int vert)
#else
static void
go(hor, vert)
  int hor, vert;
#endif
{
  extern char *bs, *nd, *up;
  int hdiff, vdiff;

  vdiff = vert - curs[1];	/* vertical difference between */
				/* current and future positions */

  if (vdiff <= 0)		/* if negative go up */
    for (; vdiff; vdiff++) addbuf(up);
  else
  {				/* else go down */
    for (; vdiff; vdiff--) addbuf("\n");
    curs[0] = 0;
  }

  hdiff = hor - curs[0];	/* horizontal difference between */
				/* current and future positions */
  curs[0] = hor;
  curs[1] = vert;		/* a new current pos, hopefully */
				/* assigned here because hor changed */
				/* below and curs needed above */

  if (hdiff < 0)		/* if negative go back */
  {
    if (-hdiff <= hor)		/* if shorter distance just use ^H */
      for (; hdiff; hdiff++) addbuf(bs);
    else
    {				/* else cr and go forward */
      addbuf("\r");
      for (; hor; hor--) addbuf(nd);
    }
  }
  else
    for (; hdiff; hdiff--)	/* have to go forward */
      addbuf(nd);
}

/* Backward: Move the cursor backwards one character */
#ifdef PROTO
static void backward(void)
#else
static void
backward()
#endif
{
  extern char *bs;

  if (curs[0] == 0) go(wid - 1, curs[1] - 1);
  else { addbuf(bs); curs[0]--; }
}

/* Forward: Move the cursor forwards one character */
#ifdef PROTO
static void forward(void)
#else
static void
forward()
#endif
{
  extern char *nd;

  curs[0]++;
  if (curs[0] >= wid)
  {
    addbuf("\n");		/* goto start of next line */
    curs[0] = curs[0] % wid;	/* hopefully gives zero */
    curs[1]++;
  }
  else addbuf(nd);
}

/* Show is a routine that replaces four routines in the old version of Clam.
 * The flag variable holds the id of which `routine' to emulate:
 *
 * case 0: Insert	Insert the letter at position pos in the line,
 *			updating the cursor, and redrawing the line.
 * case 1: Overwrite	Overwrite the letter at pos in the line,
 *			updating the cursor, and redrawing the line.
 *			Note that to toggle ins/ovw, we can have a
 *			variable ovwflag, and do ovwflag= 1-ovwflag.
 * case 2: Show		Just redisplay the line.
 * case 3: Goend	Goto the end of the line.
 */
int
Show(line, pos, let, flag)
  uchar *line;
  int let, pos, flag;
{
  extern int lenprompt;
  int i, horig=0, vorig=0;
  uchar letter = (uchar) let, c;

  switch (flag)
  {				/* Case 0: insert character */
    case 0:
      for (i = pos; line[i]; i++);	/* goto end of line */
      for (; i != pos; i--)	/* copy characters forward */
	line[i] = line[i - 1];
    case 1:
      line[pos] = letter;	/* Case 1: overwrite char */
      letter &= 0x7f;
      if (isctrl(letter)) horig = curs[0] + 2;
      else horig = curs[0] + 1; /* Calculate the new cursor */
      vorig = curs[1];		/* position */
      if (horig > wid - 1)
      { horig = horig % wid;
	vorig++;
      }
      break;
    case 2:
      pos = 0;			/* Case 2: show the line */
      horig = curs[0];
      vorig = curs[1];		/* save original values */
      curs[0] = lenprompt;
      curs[1] = 0;
  }				/* Case 3: goto end of line */
  for (c = line[pos]; c; c = line[++pos])	/* write out rest of line */
  {
    if (isctrl(c))		/* if it's a control char */
    { mputc('^');		/* print out the ^ and */
      mputc(c | 64);		/* the equivalent char for it */
    }
    else mputc(c);		/* else just show it */
  }
  if (flag != 3)
    go(horig, vorig);		/* h/vorig unused for goend */
  return (pos);			/* goend only uses this value */
}


/* I'm not exactly sure what copyback() does yet - Warren */
#ifdef PROTO
static void copyback(uchar *line, int pos, int count)
#else
static void
copyback(line, pos, count)
  uchar *line;
  int pos, count;
#endif
{
  uchar c;
  int i, horig, vorig, wipe;

#ifdef DEBUG
  fprints(2, "pos %d count %d\n", pos, count);
#endif
  if ((i = pos + count) < MAXLL)
  {
    wipe = 0;
    for (horig = pos; horig < i; horig++)
      if (isctrl(line[horig]))
	wipe += 2;		/* calculate amount to blank */
      else
	wipe++;
    for (; line[i] != EOS; ++i) /* copy line back count chars */
      line[i - count] = line[i];
    for (horig = i - count; i > horig && i < MAXLL + 1; i--)
      line[i - 1] = EOS;	/* end with nulls */
  }
  else
  {
    write(2, "count passed to copyback is too big\n", 35);
    return;
  }
  horig = curs[0];
  vorig = curs[1];
  for (c = line[pos]; c; c = line[++pos])
    if (isctrl(c))
    {
      mputc('^');
      mputc(c | 64);
    }
    else
      mputc(c);
#ifdef DEBUG
  fprints(2, "wipe %d\n", wipe);
#endif
  for (i = 0; i < wipe; i++)
    mputc(' ');			/* blank old chars */
  go(horig, vorig);
}

#define delnextword(line,pos)		nextword(line,&pos,0)
#define forword(line,pos)		nextword(line,pos,1)
#define yanknext(line,pos)		nextword(line,&pos,2)

#define delprevword(line,pos)		prevword(line,pos,0)
#define backword(line,pos)		prevword(line,pos,1)

/* The following two routines each replace three separate ones from old Clam */

/* Nextword works on the word after/at the cursor poition, depending on flag:
 *
 * case 0: Delnextword		The word after the cursor is removed
 *				from the line, and the display is updated.
 * case 1: Forword		The cursor is moved to the start of the
 *				next word.
 * case 2: Yanknext		The word after the cursor is put into yankbuf.
 *
 * Although pos is passed as a pointer, only forword() updates the value.
 */
#ifdef PROTO
static void nextword(uchar *line, int *p, int flag)
#else
static void
nextword(line, p, flag)
  uchar *line;
  int *p, flag;
#endif
{
  int inword = 0, l = 1, pos = *p, charcount = 0;
  uchar c;

  while (l)
  {
    if ((c = line[pos]) == EOS)
    { l = 0; break; }
    if (strchr((char *) wordterm, c) != NULL)	/* Found end of a word */
    { charcount++;
      pos++;
      if (inword) l = 0;
    }
    else
    { inword = 1;
      charcount++;
      pos++;
    }
  }

#ifdef DEBUG
  fprints(2, "Deleting %d chars\n", charcount);
#endif
  switch (flag)
  {
    case 0:
      copyback(line, *p, charcount);
      break;
    case 1:
      for (; l < charcount; l++) forward();	/* move forward */
      *p = pos;
    case 2:
      for (pos = *p; l < charcount; l++, pos++)
	yankbuf[l] = line[pos];
      yankbuf[l] = EOS;
  }
}

/* Prevword works on the word before the cursor poition, depending on flag:
 *
 * case 0: Delprevword		The word before the cursor is removed
 *				from the line, and the display is updated.
 * case 1: Backword		The cursor is moved to the start of the
 *				previous word.
 * case 2: Yankprev		The word before the cursor is put into yankbuf.
 *
 * Although pos is passed as a pointer, only delprevword and
 * backword update the value.
 */
int
prevword(line, p, flag)
  uchar *line;
  int *p, flag;
{
  int inword = 0, l = 1, q, pos = *p, charcount = 0;

  while (l)
  {
    if (pos == 0)
    { l = 0; break; }
    if (strchr((char *) wordterm, line[pos - 1]) != NULL)
    {						/* Found end of a word */
      if (inword) l = 0;
      else
      { charcount++; pos--; }
    }
    else
    {
      inword = 1;
      charcount++;
      pos--;
    }
  }
#ifdef DEBUG
  fprints(2, "Deleting %d chars\n", charcount);
#endif

  switch (flag)
  {
    case 0:
      for (; l < charcount; l++) backward();	/* move back */
      copyback(line, pos, charcount);	/* and copy the line on top */
      *p = pos;
      break;
    case 1:
      for (; l < charcount; l++) backward();	/* move back */
      *p = pos;
      break;
    case 2:
      q = pos;
      for (; l < charcount; l++, q++) yankbuf[l] = line[q];
      yankbuf[l] = EOS;
  }
  return (pos);
}


/* Clrline: The line from the position pos is cleared */
#ifdef PROTO
static void clrline(uchar *line, int pos)
#else
static void
clrline(line, pos)
  uchar *line;
  int pos;
#endif
{
  extern char *cd;
  int i, horig=0, vorig=0;

  if (*cd != EOS)		/* If there's a special char for clr */
  {				/* then use it */
    addbuf(cd);
    for (i = pos; line[i]; i++) line[i] = EOS;
#ifdef DEBUG
    fprints(2, "cleared ok\n");
#endif
  }
  else horig = curs[0];		/* else we gotta use spaces. */
  vorig = curs[1];		/* Orginal values for curs set. */
  for (i = pos; line[i]; i++)	/* Wipe out all those chars */
  {				/* with spaces (so slow), */
    mputc(' ');
    if (isctrl(line[i])) mputc(' ');
    line[i] = EOS;
  }
  go(horig, vorig);		/* restore original curs position */
}

/* Transpose transposes the characters at pos and pos-1 */
#ifdef PROTO
static void transpose(uchar *line, int pos)
#else
static void
transpose(line, pos)
  uchar *line;
  int pos;
#endif
{
  uchar temp;

  if (isctrl(line[pos - 1])) backward();
  backward();
  temp = line[pos];
  line[pos] = line[pos - 1];
  line[pos - 1] = temp;
  if (isctrl(line[pos - 1]))
  { mputc('^');
    mputc(line[pos - 1] | 64);
  }
  else mputc(line[pos - 1]);
  if (isctrl(line[pos]))
  { mputc('^');
    mputc(line[pos] | 64);
  }
  else mputc(line[pos]);
  if (isctrl(line[pos])) backward();
  backward();
}


/* Getuline gets a line from the user, returning a flag if the line
 * should be saved.
 */
bool
getuline(line, nosave)
  uchar *line;
  int *nosave;
{
  extern char *wbeep, *cl, *cd;
  extern uchar bindbuf[], *bindptr, CLEmode;
  extern int errno, lenprompt, curr_hist;
  uchar a;
  int c, times = 1, i, pos = 0, hist = curr_hist,
	hsave = lenprompt, vsave = 0, possave = 0;
  int beeplength = strlen(wbeep);

  memset(line, 0, MAXLL);
  wordterm = (uchar *) EVget("Wordterm");	/* Determine the word
						   terminators */
  if (wordterm == (uchar *) NULL || *wordterm == EOS)
    wordterm = (uchar *) " \t><|/;=&`";
  if (EVget("Msb")) Msb = TRUE;
  else Msb = FALSE;
  bindptr = bindbuf;		/* Set up the pointer to the bind buffer */
  *bindptr = EOS;

  prprompt();			/* Print out our prompt */
  setcbreak();			/* and set the terminal into cbreak mode */
  curs[0] = lenprompt;		/* lenprompt global set by prprompt or when
				   prompt set */
  curs[1] = 0;			/* start on line 0 */
  while (1)
  {
    flushbuf();			/* Ensure user can see the current line */
    c = getcomcmd();		/* Get a character or a command */
    for (; times > 0; times--)
      switch (c)
      {
	case EOF:
	  fprints(2, "%d\n", errno);
	  perror("comlined");
	  exit(1);
	case MARK:
	  hsave = curs[0];	/* save position (make mark) */
	  vsave = curs[1];
	  possave = pos;
	  break;
	case START:
	  go(lenprompt, 0);	/* goto start of the line */
	  pos = 0;
	  break;
	case BAKCH:
	  if (pos > 0)		/* if not at home, go back */
	  { if (isctrl(line[pos - 1])) backward();
	    backward();
	    pos--;
	  }
	  else Beep;		/* else ring bell */
	  break;
	case INT:
	  pos = goend(line, pos);
	  addbuf("\n");
	  flushbuf();
	  setcooked();
	  return (FALSE);
	case DELCH:
	  if (line[0] == EOS)	/* Leave shell if possible */
	  { flushbuf();
	    if (EVget("ignoreeof"))
	      fprints(2, "Use `exit' to exit wish\n");
	    else leave_shell(0); /* eof */
	    setcooked();
	    return (FALSE);	/* return if no exit */
	  }
	  else if (line[pos] != EOS)
	    copyback(line, pos, 1);	/* delete char */
	  else
	    complete((char *) line, &pos, FALSE);
	  break;
	case END:
	  pos = goend(line, pos);	/* goto end of the line */
	  break;
	case FORCH:
	  if (line[pos] != EOS) /* if not at end, go forward */
	  { if (isctrl(line[pos])) forward();
	    forward();
	    pos++;
	  }
	  else Beep;		/* else ring bell */
	  break;
	case KILLALL:
	  go(lenprompt, 0);	/* goto start */
	  pos = 0;
	  clrline(line, 0);	/* and kill from pos=0 */
	  hist = curr_hist;	/* reset hist */
	  for (pos = 0; pos < MAXLL; pos++)
	    line[pos] = EOS;
	  pos = possave = 0;
	  break;
	case DEL:
	case BKSP:
	  if (pos > 0)		/* if not at home, delete */
	  { if (isctrl(line[pos - 1])) backward();
	    backward();
	    copyback(line, --pos, 1);	/* move line back on to */
	  }			/* prev char */
	  else Beep;		/* else ring bell */
	  break;
	case COMPLETE:
	  if (line[0] != EOS)	/* try to complete word */
	    complete((char *) line, &pos, TRUE);
	  else Beep;
	  break;
	case NL:
	case FINISH:
	  pos = goend(line, pos);
	  addbuf("\n");
	  flushbuf();
	  setcooked();
	  line[pos++] = EOS;
	  *nosave = strip(line);/* process it now */
	  if (line[0] != EOS) return (TRUE);
	  else return (FALSE);
#ifndef NO_HISTORY
	case NEXTHIST:
	  if (hist < curr_hist) /* put next hist in line buf */
	  { loadhist((char *) line, ++hist);
	    goto redisp;	/* Yuk, a goto */
	  }
	  else Beep;
	  break;
	case BACKHIST:
	  if (hist == 1) Beep;
	  else
	  { if (hist == curr_hist)	/* in line buf */
	      (void) savehist((char *) line, 0);
	    loadhist((char *) line, --hist);
	  }
#endif
	case REDISP:
      redisp:
	  go(lenprompt, 0);
	  addbuf(cd);		/* Clear to end of screen */
	  show(line, TRUE);	/* Show the line typed so far. */
	  pos = goend(line, 0); /* goto end of the line */
	  break;
	case CLREDISP:
	  addbuf(cl);		/* Clear the screen */
	  flushbuf();
	  prprompt();		/* Reprint the prompt and */
	  show(line, TRUE);	/* the line typed so far. */
	  break;
	case KILLEOL:
	  clrline(line, pos);	/* kill line from cursor on */
	  break;
	case XON:
	  continue;		/* can't use this */
	case XOFF:
	  continue;		/* can't use this */
	case TRANSPCH:
	  if (pos > 0 && line[pos] != EOS)	/* if not home or at end */
	    transpose(line, pos);	/* swap current and prev char */
	  else Beep;		/* else ring bell */
	  break;
	case QUOTE:
	  if (pos >= MAXLL)
	  { Beep;
	    continue;
	  }
	  mputc('"');
	  backward();		/* literal char */
	  flushbuf();
	  read(0, (char *) &a, 1);
	  c = a;
	  if (c)		/* don't allow EOS (null) */
	    insert(line, pos++, c);
	  break;
	case DELWD:
	  if (pos) delprevword(line, &pos);
	  else Beep;
	  break;
	case GOMARK:
	  pos = possave;
	  go(hsave, vsave);
	  break;
	case YANKLAST:
	  if (pos != 0)		/* if not at home */
	    yankprev(line, pos);/* yank previous word */
	  else Beep;		/* else ring bell */
	  break;
	case SUSP:
	  continue;
	case BAKWD:
	  if (pos > 0)
	    backword(line, &pos);
	  else Beep;
	  break;
	case DELWDF:
	  if (line[pos] != EOS)
	    delnextword(line, pos);
	  else Beep;
	  break;
	case FORWD:
	  if (line[pos] != EOS)
	    forword(line, &pos);
	  else Beep;
	  break;
	case PUT:
	  if (pos > MAXLL - strlen((char *) yankbuf))
	  { Beep;
	    break;
	  }
	  for (i = 0; yankbuf[i]; i++)	/* insert yank buffer */
	    insert(line, pos++, yankbuf[i]);
	  break;
	case YANKNEXT:
	  if (line[pos] != EOS) /* if not at end */
	    yanknext(line, pos);/* yank next word */
	  else Beep;		/* else ring bell */
	  break;
	case SEARCHF:
	  if (line[pos])	/* search forward */
	  { read(0, (char *) &a, 1);
	    c = a;		/* char to search for */
	    for (i = pos + 1; line[i] && c != line[i]; i++);
	    if (line[i])
	      while (pos < i)
	      { pos++;
		if (isctrl(line[pos - 1])) forward();
		forward();
	      }
	    else Beep;		/* not found */
	  }
	  else Beep;		/* at end of line */
	  break;
	case SEARCHB:
	  if (pos > 0)		/* search backwards */
	  { read(0, (char *) &a, 1);
	    c = a;		/* char to search for */
	    for (i = pos - 1; i >= 0 && c != line[i]; i--);
	    if (i >= 0)
	      while (pos > i)
	      { pos--;
		if (isctrl(line[pos])) backward();
		backward();
	      }
	    else Beep;		/* not found */
	  }
	  else Beep;		/* at end of line */
	  break;
	case MODEON:
	  CLEmode = 1;		/* Turn the CLE mode on */
	  break;
	case MODEOFF:
	  CLEmode = 0;		/* Turn the CLE mode off */
	  break;
	case BEEP:
	  Beep;			/* Simply beep */
	  break;
	case QUOTEUP:
	  if (pos >= MAXLL)	/* Turn on next char's msb */
	  { Beep;
	    continue;
	  }
	  mputc('"');
	  backward();		/* literal char */
	  flushbuf();
	  read(0, (char *) &a, 1);
	  c = a;
	  if (c)		/* don't allow EOS (null) */
	  { c |= 0x80;
	    insert(line, pos++, c);
	  }
	  break;
	default:
	  if (pos >= MAXLL - 1)
	  { Beep;
	    break;
	  }
	  insert(line, pos++, c);
	  break;
      }
    times = 1;
  }
}
#else	/* NO_COMLINED */
bool
getuline(line, nosave)
  uchar *line;
  int *nosave;
{
  int i;

  prprompt();			/* Print out our prompt */
  for (i=0; i<MAXLL; i++) {
    read(0, &line[i], 1);	/* Get a char */
    if (line[i]=='\n') break;
  }
  line[i] = EOS;
  *nosave = strip(line);/* process it now */
  if (line[0] != EOS) return (TRUE);
  else return (FALSE);
}
#endif
