/* The key binding routines used by the command line editor
 *
 * $Revision: 41.3 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

/* Wish allows keystrokes to be bound to other keystrokes. The following
 * structure is used to hold these bindings. Note well the mode bit; a
 * binding with mode!=0 is only used when the CLE has it's mode bit on.
 */

#ifndef NO_BIND
struct keybind
{
  uchar *key;			/* The key sequence we have bound */
  int len;			/* The length of the key sequence */
  uchar *cmd;			/* The string it is mapped to */
  uchar mode;			/* Mode bit. Binding only used when mode on */
  struct keybind *next;
};


static int Keylength = 0;	/* The maximum key length */
static struct keybind *Bindhead = NULL;	/* List of bindings */

uchar bindbuf[512];		/* Buffer used to expand bindings */
uchar *bindptr;			/* Pointer into bindbuf */
uchar CLEmode;			/* This holds the CLE mode */

/* The default key bindings
 */

static char *defbind[15][3] = {
			        { NULL, "\033\020", "\201" },
			        { NULL, "\033B", "\202" },
			        { NULL, "\033b", "\202" },
			        { NULL, "\033D", "\203" },
			        { NULL, "\033d", "\203" },
			        { NULL, "\033F", "\204" },
			        { NULL, "\033f", "\204" },
			        { NULL, "\033H", "\205" },
			        { NULL, "\033h", "\205" },
			        { NULL, "\033P", "\206" },
			        { NULL, "\033p", "\206" },
			        { NULL, "\033Y", "\207" },
			        { NULL, "\033y", "\207" },
			        { NULL, "\033/", "\210" },
			        { NULL, "\033?", "\211" }
};

/* Bind is a builtin. With no arguments, it lists the current key bindings.
 * With 1 arg, it shows the binding (if any) for argv[1]. With 2 args
 * the string argv[1] will be replaced by argv[2].
 */

int 
Bind(argc, argv)
  int argc;
  uchar *argv[];
{
  int s, showall;
  uchar *key, *cmd, *ctemp;
  struct keybind *temp, *Bindtail;

  if (argc > 4 || argc < 3)
  {
    fprints(2, "usage: bind [[-m] key [value]]\n");
    return (1);
  }
  showall = 0;
  switch (argc)
  {
    case 4:
      key = argv[2];
      cmd = argv[3];		/* Bind a string to another */
      goto bindit;		/* Yuk, a goto */
    case 3:
      key = argv[1];
      cmd = argv[2];		/* Bind a string to another */
  bindit:
      s = strlen((char *)key);	/* Get the key's length */
      if (s == 0)
	break;

      temp = (struct keybind *) malloc(sizeof(struct keybind));
      if (!temp)
      {
	perror("malloc");
	return (1);
      }
      temp->key = (uchar *) malloc(s + 1);
      if (!(temp->key))
      {
	perror("malloc");
	return (1);
      }

      strcpy((char *)temp->key, (char *)key);	/* Copy the key */
      temp->len = s;
      temp->cmd = (uchar *) malloc(strlen((char *)cmd) + 1);
      if (!(temp->cmd))
      {
	perror("malloc");
	return (1);
      }
      /* Copy the value */
      ctemp = temp->cmd;
      while (*cmd != EOS)
      {
	if (*cmd != '\\')
	  *(ctemp++) = *(cmd++);
	else
	{
	  showall = 0;
	  cmd++;		/* or an octal value */
	  while (isdigit(*cmd))
	    showall = (showall << 3) + (*(cmd++) - 48);
	  *(ctemp++) = showall & 0xff;
	}
      }
      *ctemp = EOS;
      temp->next = NULL;
      if (argc == 4)
	temp->mode = 1;
      else
	temp->mode = 0;
      if (s > Keylength)
	Keylength = s;

      if (!Bindhead)		/* Add to linked list */
	Bindhead = temp;	/* Currently this allows duplicates :-( */
      else
      {
	for (Bindtail = Bindhead; Bindtail->next; Bindtail = Bindtail->next);
	Bindtail->next = temp;
      }
      break;
    case 1:
      showall = 1;		/* Print one or more bindings */
    case 2:
      for (temp = Bindhead; temp; temp = temp->next)
	if (showall || !strcmp((char *)temp->key, (char *)key))
	{
	  if (temp->mode)
	    prints(" * ");
	  else
	    prints("   ");
	  mprint(temp->key, 1);
	  for (s = Keylength - temp->len; s; s--)
	    write(1, " ", 1);
	  prints(" bound to ");
	  mprint(temp->cmd, 0);
	}
  }
  return (0);
}

/* Unbind is a builtin which removes argv[1] from the list of key bindings */

int 
unbind(argc, argv)
  int argc;
  uchar *argv[];
{
  int s;
  uchar *key;
  struct keybind *temp, *t2;

  if (argc != 2)
  {
    fprints(2, "usage: unbind string, or unbind all\n");
    return (1);
  }
  key = argv[1];
  if (!strcmp((char *)key,"all"))
   {
    for (temp = Bindhead, t2 = Bindhead; temp; t2 = temp, temp = temp->next)
     {
      free(temp->key);
      free(temp->cmd);
      free(temp);
     }
    Bindhead = NULL;
    return(0);
   }
  s = strlen((char *)key);		/* Get the key's length */
  if (s == 0)
    return (1);

  Keylength = 0;
  for (temp = Bindhead, t2 = Bindhead; temp; t2 = temp, temp = temp->next)
    if (s == temp->len && !strcmp((char *)temp->key, (char *)key))
    {
      if (temp == Bindhead)
	Bindhead = temp->next;
      else
	t2->next = temp->next;
      free(temp->key);
      free(temp->cmd);
      free(temp);
    }
    else if (temp->len > Keylength)
      Keylength = temp->len;
  return (0);
}

/* Initialise the default key bindings */
void initbind()
 {
  int i;

  for (i = 0; i < 15; i++)
    Bind(3, (uchar **) defbind[i]);	/* Set default bindings */
  CLEmode = 0;				/* and start in mode 1 */
 }


/* Exbind: get one or more characters from the user, expanding bindings
 * along the way. This routine is recursive & hairy! When called from
 * getcomcmd(), inbuf is NULL, indicating we want user keystrokes.
 * These are read in, and if there are no partial bind matches, are
 * returned to getcomcmd(). If any partial matches, they are buffered
 * in bindbuf until either no partials or 1 exact match. Once a match is
 * found, we call ourselves with inbuf pointing to the replacement string.
 * Thus bindings can recurse, up to the size of bindbuf. Note also that
 * after we recurse once, we check to see if there are any leftover chars
 * in inbuf, and recurse on them as well.
 */

#ifdef PROTO
static void expbind(uchar *inbuf)
#else
static void 
expbind(inbuf)			/* Expand bindings from user's input */
  uchar *inbuf;
#endif
{
  uchar a, *startptr, *exactptr;
  int c, currlen, partial, exact;
  struct keybind *temp;

  if (inbuf == NULL)
    bindptr = bindbuf;
  startptr = bindptr;
  currlen = 0;

  while (1)			/* Look for a keystroke binding */
  {
    partial = exact = 0;
    if ((inbuf == NULL) || ((a = *(inbuf++)) == EOS))
      c = read(0, (char *)&a, 1);
    if (c != -1)		/* Decide which uchar to put in the buffer */
    {
      *(bindptr++) = a;
      *bindptr = EOS;
      currlen++;
      if ((int) (bindptr - bindbuf) > 510)
	return;
    }

    for (temp = Bindhead; temp != NULL; temp = temp->next)
    {				/* Count the # of partial & exact matches */
      /* We exclude mode bindings when CLEmode==0 */
      if (CLEmode == 0 && temp->mode == 1)
	continue;
      if (currlen > temp->len)
	continue;
      if (!strcmp((char *)startptr, (char *)temp->key))
      {
	exact++;
	exactptr = temp->cmd;
      }
      if (!strncmp((char *)startptr, (char *)temp->key, currlen))
	partial++;
    }
    if (partial == 0)
      break;			/* No binding at all */
    if (partial == 1 && exact == 1)	/* An exact match, call ourselves */
    {
      bindptr = startptr;	/* with the matched word */
      expbind(exactptr);
      break;
    }
  }

  if (inbuf != NULL && *inbuf != EOS)	/* If any part of our word left over */
    expbind(inbuf);		/* check it as well */
}

/* Getcomcmd converts a user's keystokes into the commands used by the CLE.
 * If there are no chars handy in bindbuf, we call expbind() to get some.
 * Then we scan thru the chars and deliver chars or commands to the CLE.
 * Hopefully because we use commands>255, the CLE will work with 8-bit
 * extended ASCII.
 */

int 
getcomcmd()			/* Get either a character or a command from the */
{				/* user's input */
  int c;

  /* If no chars, get chars from stdin and */
  /* expand bindings */
  while ((c = *(bindptr++)) == 0)
  {
    expbind(NULL);
    bindptr = bindbuf;
  }

  /* Default to usual keys */
#ifdef DEBUG
  fprints(2, "Returning %x\n", c);
#endif
  return (c);
}
#endif	/* NO_BIND */
