/* The command history is stored as a singly linked list, each node holding
 * a command, its history number, and a pointer to the next node.
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

#ifndef NO_HISTORY
static struct vallist hlist =	/* The history list */
	{ NULL, NULL};
int curr_hist = 1;		/* The current history number */
static int maxhist = 25;	/* Maximum # of saved lines */
static int curr_saved = 0;	/* How many we have saved currently */
static bool nohistdup = TRUE;	/* Do we omit duplicate lines? */


/* Savehist saves the given line into the history with the given history
 * number, and ensuring that there are no more than max histories.
 * It returns 1 if saved, or 0 if a duplicate or other errors.
 */
int
savehist(line, andadd)
  char *line;
  bool andadd;
{
  struct val *v;
  char *mh;

  mh = EVget("history");	/* Set the current history */
  if (mh) maxhist = atoi(mh);

				/* Adjust the number saved if necessary */
  while (curr_saved > maxhist) { pullval(&hlist); curr_saved--; }

				/* If we don't care about duplicates, or there */
				/* isn't a duplicate, save the history */
  if (!nohistdup || ((v = searchval(&hlist, line, TRUE, FALSE)) == NULL))
  {
    v = (struct val *) Malloc(sizeof(struct val), "savehist malloc");
    v->name = Malloc(strlen(line) + 1, "savehist malloc");
    strcpy(v->name, line); v->hnum = curr_hist++; appendval(&hlist, v);
    if (++curr_saved > maxhist) { pullval(&hlist); curr_saved--; }
  }
  return (1);
}

/* Loadhist finds the command line with the given histnum, and loads it
 * at the given pos into the command line.
 */
void
loadhist(line, histnum)
  char *line;
  int histnum;
{
  struct val *ptr;

			/* Find either the last hist or the histnum one */
  for (ptr = hlist.head; ptr && ptr->hnum != histnum; ptr = ptr->next);
  if (ptr) strcpy(line, ptr->name);	/* copy into the command line */
				/* else we can't find that number so load */
				/* nothing. Maybe it should beep here */
}

/* The history builtin prints out the current history list.
 * None of the arguments are used.
 */
int
history(argc, argv)
  int argc;
  char *argv[];
{
  struct val *ptr;

  for (ptr = hlist.head; ptr && ptr->hnum < curr_hist; ptr = ptr->next)
    if (ptr->hnum >= curr_hist - maxhist)
    { prints("%d  ", ptr->hnum);
      mprint((uchar *) ptr->name, 0);	/* this routine intercepts the ^ chars */
    }
  return (0);
}

/* Getnumhist returns the command line with the given history number.
 * If none exists, NULL is returned.
 */
#ifdef PROTO
static char * getnumhist(int histnum)
#else
static char *
getnumhist(histnum)
  int histnum;
#endif
{
  struct val *ptr;

  for (ptr = hlist.head; ptr; ptr = ptr->next)
    if (ptr->hnum == histnum) return (ptr->name);
  return (NULL);
}

/* Gethist takes a event string, and returns a match from
 * the history. The event can be one of the following:
 *	   Value			Return value
 *	an integer		The command line with the given histnum
 *	"!"			The last command line
 *	-integer		The line integer histories ago
 *	word			The last line that strncmps the word
 *
 * If no event is found, NULL is returned.
 */
char *
gethist(event)
  char *event;
{
  struct val *ptr, *this;
  int histnum, len;

  if ((*event >= '0' && *event <= '9') || (*event == '-') || (*event == '!'))
  {
    switch (*event)
    {
      case '-':
	histnum = curr_hist - atoi(event + 1);
	break;
      case '!':
	histnum = curr_hist - 1;
	break;
      default:
	histnum = atoi(event);
    }
#ifdef DEBUG
    prints("Histnum is %d\n", histnum);
#endif
    return (getnumhist(histnum));
  }
  else
  {
    /*
       If the history matches the requested event then that history line is
       copied into the event pointer. Note that the first histnum characters of
       event will not change and thus further searches for more recently
       matching histories will still be valid.
    */
    len= strlen(event);
    for (ptr=NULL, this = hlist.head; this; this = this->next)
        if (!strncmp(this->name, event, len)) ptr=this;

    if (ptr) return (ptr->name); else return(NULL);
  }
}
#endif	/* NO_HISTORY */
