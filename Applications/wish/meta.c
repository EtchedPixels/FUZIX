/* Metacharacter expansion. This file holds the routines that do the first
 * parsing of the command line, and the expansion of the metacharacters
 * ' " ` \ $ ~ * ? and [
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"
#define MAXMATCH 512
#define OK	0
#define NX_ERR -1
#define BR_ERR -2


extern struct candidate carray[];	/* The matched files */
extern struct candidate *next_word;
int ncand;
struct candidate *wordlist;	/* The list of words for the parser */
static int Mode;		/* Used by star */
static char *qline;		/* ``'d line from backquot */

#ifdef PROTO
static int matchdir(char *directory, char *pattern);
#else
static int matchdir();
#endif
#ifdef DEBUG
static void listcarray();
#endif

/* Match takes a string, and a pattern, which can contain *, ? , \ and [],
 * and returns 0 if the strings matched the pattern, otherwise a negative
 * number. If the pattern ends in a '/', matchdir is called with the
 * arg accumdir/string to find further matches. Match also adds each match
 * into the matches array given above.
 */
#ifdef PROTO
static int match(char *string, char *pattern, char *accumdir)
#else
static int
match(string, pattern, accumdir)
  char *string, *pattern, *accumdir;
#endif
{
  char c, *findex, *pindex, *fmatch, *pmatch, rempat[MAXWL], where[MAXPL];
  int i, mismatch, found, star;

  findex = string;		/* Initialise our vars */
  pindex = pattern;
  if ((*findex == '.') && (*pindex != '.'))	/* Don't match . unless asked */
    return (NX_ERR);
  mismatch = star = 0;
  while (*findex && !mismatch)	/* While no mismatch */
  {
    switch (*pindex)		/* Match the pattern's char */
    {
      case '*':
	pindex++;		/* Skip the '*' */
	pmatch = pindex;	/* We must match from here on */
	fmatch = findex;
	star = 1;		/* Found a star */
	break;
      case '?':
	pindex++;		/* just increment, letter automatically
				   matches. */
	findex++;
	break;
      case '[':
	found = 0;		/* For all chars in [] */
	for (; (*pindex) && (*pindex != ']'); pindex++)
	{
	  switch (*pindex)
	  {
	    case '\\':
	      if (*(pindex + 1) == *findex)	/* Try match on next char */
		found = 1;
	      break;
	    case '-':
	      if ((*(pindex + 1) == ']') || (*(pindex - 1) == '['))
		if (*pindex == *findex)
		{
		  found = 1;	/* Try a match on '-' */
		  break;
		}		/* or try all chars in the range */
	      for (c = *(pindex - 1) + 1; c <= *(pindex + 1) && !found && c != ']'; c++)
		if (c == *findex)
		  found = 1;
	      break;
	    default:		/* Default: just match that char */
	      if (*pindex == *findex)
		found = 1;
	      break;
	  }
	}
/*
 * We could exit the loop as soon as found is set to 1 but we have to
 * keep going to the end of the list so we can recommence matching
 * after the ']'. This also allows us to test for the unmatched
 * bracket.
 */
	if (*pindex == EOS)
	{
	  fprints(2, "Unmatched bracket '['\n");
	  return (BR_ERR);
	}
	if (found)
	{
	  pindex++;		/* move past the ']' */
	  findex++;		/* and on to the next letter */
	  break;
	}
	if (!star)
	  mismatch = 1;		/* failure for this letter of file */
	else
	{
	  pindex = pmatch;
	  findex = ++fmatch;
	}
	break;
      case '\\':
	pindex++;		/* go on to next symbol */

      default:
	if (*findex != *pindex)	/* No match and not a star */
	  if (!star)		/* gives an error */
	    mismatch = 1;
	  else
	  {
	    pindex = pmatch;	/* Move back to the char we must */
	    findex = ++fmatch;	/* match, & move along string */
	  }
	else
	{
	  pindex++;		/* Yes, a match, move up */
	  findex++;
	}
	break;
    }
  }
  if (mismatch)
    return (NX_ERR);		/* it didn't match, go on to next one */
  for (; *pindex == '*'; pindex++);	/* get rid of trailing stars in pattern */
  if (*findex == *pindex)
  {
    /* add it to the candidate list */
    where[0] = EOS;
    if (*accumdir)
    {
      strcpy(where, accumdir);
      strcat(where, "/");
    }
    strcat(where, string);
    if (ncand == MAXCAN)
      return (OK);
    carray[ncand].name = (char *) malloc((unsigned) (strlen(where) + 4));
    if (carray[ncand].name == NULL)
      return (OK);
    strcpy(carray[ncand].name, where);
    carray[ncand++].mode = Mode | TRUE | C_SPACE;	/* Was malloc'd */
    return (OK);
  }
  else if (*pindex == '/')	/* test if file is a directory */
  {
    for (i = 0; *pindex; i++, pindex++)
      rempat[i] = *(pindex + 1);/* even copies null across */
    strcpy(where, accumdir);
    if (where[0] != EOS)
    {
      strcat(where, "/");
      strcat(where, string);
    }
    else
      strcpy(where, string);
    return (matchdir(where, rempat));	/* recursively call this pair again */
  }
  else				/* it just doesn't match */
    return (NX_ERR);
}

/* Matchdir takes a directory name and a pattern, and returns 0 if any
 * files in the directory match the pattern. If none, it returns a negative
 * number. Note that this may be recursive, as it calls match(), which calls
 * matchdir().
 */
#ifdef PROTO
static int matchdir(char *directory, char *pattern)
#else
static int
matchdir(directory, pattern)
  char *directory, *pattern;
#endif
{
  DIR *dirp;

#ifdef USES_DIRECT
  struct direct *entry;
#else
  struct dirent *entry;
#endif
  int foundany = 0;

  if (*directory != EOS)
  {
    if ((dirp = opendir(directory)) == NULL)
      return (NX_ERR);
  }
  else
  {
    if ((dirp = opendir(".")) == NULL)
      return (NX_ERR);
  }
  while ((entry = readdir(dirp)) != NULL)
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") &&
	!match(entry->d_name, pattern, directory))
      foundany++;
  closedir(dirp);
  if (foundany)
    return (OK);
  else
    return (NX_ERR);
}

#ifdef PROTO
static void finddir(char *word, char *dir)
#else
static
void
finddir(word, dir)
  char *word, *dir;
#endif
{
  char c;
  int i = 0, j, l = 1;

/* This function finds the directory to start the matching from. */
  while (l)
    switch (c = word[i])
    {
      case '*':
      case '?':
      case '[':
	for (; dir[i] != '/' && i; i--);	/* go back to end of previous
						   component */
	if (i)
	  dir[i++] = EOS;
	else if (dir[0] == '/')
	  dir[++i] = EOS;
	else
	  dir[0] = EOS;
	l = 0;
	break;
      default:
	dir[i++] = c;
    }
  for (l = i, j = 0; word[l]; l++, j++)
    word[j] = word[l];
  word[j] = EOS;
}


#ifndef NO_TILDE
/* Tilde takes the word beginning with a ~, and returns the word with the
 * first part replaced by the directory name. If ~/, we use $HOME
 * e.g ~fred -> /u1/staff/fred
 *     ~jim/Dir/a*.c -> /usr/tmp/jim/Dir/a*.c
 *     ~/Makefile -> /u1/staff/warren/Makefile
 * If it cannot expand, it returns dir as a zero-terminated string
 *
 * It also uses the tilde list now as well.
 */
void
tilde(word, dir)
  char *word, *dir;
{
  extern struct vallist tlist;

  struct passwd *entry;
  struct val *t;
  char *a;

  word++;
  *dir = EOS;
  if (*word == '/')		/* use $HOME  if ~/ */
  {
    a = word;
    entry = getpwuid(getuid());
    strcpy(dir, entry->pw_dir);
  }
  else
  {
    a = strchr(word, '/');	/* Get the word to parse */
    if (a != NULL)
      *a = EOS;
    for (t = tlist.head; t; t = t->next) /* Try the tilde list next */
    {
      if (!strcmp(word, t->name))
      {
	strcpy(dir, t->val);
	break;
      }
    }
    if (*dir == EOS)		/* Finally, consult the passwd file */
    {
      entry = getpwnam(word);	/* Get the directory's name */
      if (entry)
	strcpy(dir, entry->pw_dir);
    }
  }
  endpwent();
  if (*dir == EOS)
    return;
  /* Form the real partial name */
  if (a != NULL)
  {
    *a = '/';
    strcat(dir, a);
  }
}
#endif

/* Dollar expands a variable. It takes a pointer to a candidiate,
 * and replaces it with the variable's value.
 */
#ifdef PROTO
static void dollar(struct candidate *cand)
#else
static void
dollar(cand)
  struct candidate *cand;
#endif
{
  extern int Exitstatus, Argc;
  extern char **Argv;
  struct candidate *next;
  char *end, *value;
  char c = EOS;
  char *a, *doll;
  unsigned int length, base, mall = 0;	/* Mall is 1 if we've malloc'd */


  doll = cand->name;
  /* If a special variable */
  if (strlen(doll) == 1 || doll[1] == '=')
  {
    switch (*doll)
    {
      case '$':		/* Replace with pid */
	value = (char *) malloc(10);
	length = getpid();
	if (value)
	{
	  sprints(value, "%d", length);
	  mall = 1;
	}
	break;
      case '#':		/* Replace with # of args */
	value = (char *) malloc(10);
	if (value)
	{
	  sprints(value, "%d", Argc - 1);
	  mall = 1;
	}
	break;
      case '?':		/* Replace with exist status */
	value = (char *) malloc(10);
	if (value)
	{
	  sprints(value, "%d", Exitstatus);
	  mall = 1;
	}
	break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':		/* Replace with Argv[n] */
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	length = *doll - '0';
	value = (length < Argc ? Argv[length] : "");
	break;
      case '*':		/* Replace with all args */
	if (Argc > 1)
	{
	  base = ncand;
	  ncand += (Argc - 1);
	  carray[ncand - 1].next = cand->next;
	  carray[ncand - 1].name = Argv[Argc - 1];
	  carray[ncand - 1].mode = C_SPACE;
	  cand->next = &carray[base];
	  for (length = 1; base < ncand - 1; base++, length++)
	  {
	    carray[base].name = Argv[length];
	    carray[base].mode = C_SPACE;
	    carray[base].next = &carray[base + 1];
	  }
	}
	if (cand->mode & TRUE)
	{
	  free(cand->name);
	  cand->mode = FALSE;
	}
	cand->name = NULL;
	return;
      default:
	fprints(2, "Unknown variable $%c\n", *doll);
	return;
    }
    if (doll[1] == '=')		/* An assignment */
    {
      end = doll + 1;
      c = '=';
    }
  }
  else
  {				/* A normal variable */
    /* Find the last char of the var */
    for (end = doll; end != EOS && isalnum(*end); end++);
    c = *end;
    *end = EOS;			/* Terminate the name */

    value = EVget(doll);	/* Get the value */
    if (!value)
    {
      fprints(2, "Unknown variable $%s\n", doll);
      return;
    }
    else if (c == '[')		/* or a field of it */
    {
      length = atoi(++end);	/* Get the field number */
      /* Skip the number */
      while (*end != ']' && *end != EOS)
	end++;
      if (*end == ']')
	end++;
      c = *end;
      *end = EOS;
      for (a = value; length; length--)
      {				/* Find a gap */
	if (!(a = strpbrk(a, " \t")))
	{
	  value = NULL;
	  break;
	}
	while (*a == ' ' || *a == '\t')
	  a++;			/* Skip the gap */
	if (*a == EOS)
	{
	  value = NULL;
	  break;
	}
      }
      if (value)		/* If a valid field */
      {
	length = strcspn(a, " \t");	/* Get the length of the field */
	value = (char *) malloc(length + 2);
	if (value)
	{
	  strncpy(value, a, length);	/* Make a copy - can't change var */
	  value[length] = EOS;
	  mall = TRUE;
	}
      }
    }
  }

  cand->mode &= ~C_DOLLAR;
  if (c)			/* If there's more to the value */
  {
    *end = c;			/* Insert the rest in the list */
    next = cand->next;
    addcarray(end, cand, cand->mode | TRUE, TRUE);
    carray[ncand - 1].next = next;
    cand->mode &= ~C_SPACE;
  }
  if (cand->mode & TRUE)
    free(cand->name);		/* we can replace totally */
  cand->name = value;
  if (mall)
    cand->mode |= TRUE;
  else
    cand->mode &= ~TRUE;
#ifdef DEBUG
  fprints(2, " In dollar ");
  listcarray();
#endif
  return;
}


/* Expline takes a linked list of words, and converts them into a normal
 * string. This is used to save history. No double quotes now.
 */
char *
expline(list)
  struct candidate *list;
{
  struct candidate *q;
  char *out, *a;
  int i, mode = 0;
  bool lastspace = FALSE;

  /* Find the amount to malloc */
  for (i = 0, q = list; q != NULL; q = q->next)
    if (q->name)
      i += strlen(q->name) + 6;	/* Just in case */

  out = (char *) malloc((unsigned) i + 4);
  if (out == NULL)
    return (NULL);

  for (a = out, q = list; q; q = q->next)
  {
    mode = q->mode;
    if (q->name)		/* It's a word */
    {
      if (lastspace)
	*(a++) = ' ';
      if (mode & C_QUOTE)
	*(a++) = '\'';
      if (mode & C_CURLY)
	*(a++) = '{';
      if (mode & C_BACKQUOTE)
	*(a++) = '`';
      if (mode & C_DOLLAR)
	*(a++) = '$';		/* Add $ to vars */
      strcpy(a, q->name);
      a += strlen(q->name);	/* Add the word!! */
      if (mode & C_QUOTE)
	*(a++) = '\'';
      if (mode & C_CURLY)
	*(a++) = '}';
      if (mode & C_BACKQUOTE)
	*(a++) = '`';
      lastspace = mode & C_SPACE;
    }
    else
    {
      if (lastspace)
	*(a++) = ' ';
      switch (mode & C_WORDMASK)
      {
	case C_SEMI:
	  strcpy(a, "; ");
	  a += 2;
	  break;
	case C_PIPE:
	  strcpy(a, "| ");
	  a += 2;
	  break;
	case C_DBLPIPE:
	  strcpy(a, "|| ");
	  a += 3;
	  break;
	case C_AMP:
	  strcpy(a, "& ");
	  a += 2;
	  break;
	case C_DBLAMP:
	  strcpy(a, "&& ");
	  a += 3;
	  break;
	case C_LT:
	  strcpy(a, "< ");
	  a += 2;
	  break;
	case C_LTLT:
	  strcpy(a, "<< ");
	  a += 3;
	  break;
	case C_GT:
	  if ((mode & C_FD) == 2)
	  {
	    strcpy(a, ">& ");
	    a += 3;
	  }
	  else
	  {
	    strcpy(a, "> ");
	    a += 2;
	  }
	  break;
	case C_GTGT:
	  strcpy(a, ">> ");
	  a += 3;
	  break;
      }
      lastspace = FALSE;
    }
  }
  return (out);
}

#ifdef DEBUG
/* Wordlist is used for debugging to print out the carray linked list */
static void
listcarray()
{
  struct candidate *curr;

  fprints(2, "Here's the wordlist:\n");
  for (curr = wordlist; curr != NULL; curr = curr->next)
  {
    fprints(2, "--> %x  ", curr);
    if (curr->name)
      fprints(2, "%s", curr->name);
    if (curr->mode & C_WORDMASK)
      switch (curr->mode & C_WORDMASK)
      {
	case C_SEMI:
	  fprints(2, ";    C_SEMI");
	  break;
	case C_PIPE:
	  fprints(2, "|    C_PIPE");
	  break;
	case C_DBLPIPE:
	  fprints(2, "||   C_DBLPIPE");
	  break;
	case C_AMP:
	  fprints(2, "&    C_AMP");
	  break;
	case C_DBLAMP:
	  fprints(2, "&&   C_DBLAMP");
	  break;
	case C_LT:
	  fprints(2, "<    C_LT");
	  break;
	case C_LTLT:
	  fprints(2, "<<   C_LTLT");
	  break;
	case C_GT:
	  fprints(2, "%d>   C_GT", curr->mode & C_FD);
	  break;
	case C_GTGT:
	  fprints(2, "%d>>  C_GTGT", curr->mode & C_FD);
	  break;
	default:
	  fprints(2, "%o  C_UNKNOWN", curr->mode);
      }
    else
    {
      if (curr->mode & C_SPACE)
	fprints(2, "   C_SPACE");
      if (curr->mode & C_DOLLAR)
	fprints(2, "   C_DOLLAR");
      if (curr->mode & C_QUOTE)
	fprints(2, "   C_QUOTE");
      if (curr->mode & C_CURLY)
	fprints(2, "   C_DBLQUOTE");
      if (curr->mode & C_BACKQUOTE)
	fprints(2, "   C_BACKQUOTE");
      if (curr->mode & TRUE)
	fprints(2, "   (malloc'd)");
    }
    fprints(2, " %x\n", curr->next);
  }
}

#endif

/* Addword is used by Meta_1 to add a word into the carray list.
 * This is where history is expanded.
 */
#ifdef PROTO
static void addword(char *string, int mode)
#else
static void
addword(string, mode)
  char *string;
  int mode;
#endif
{
  char *b;

  if (string != NULL)		/* It's a word, check it out */
  {
    if (*string == EOS)
      return;

#ifndef NO_HISTORY
    /* Find any history and retrieve it */
    if ((mode & (C_QUOTE | C_BACKQUOTE | C_DOLLAR | C_CURLY)) == 0)
      if ( *string == '!' && (b = gethist(++string)) != NULL)
    {
      meta_1(b, FALSE);		/* Expand it too */
      return;
    }
#endif
    /* mode &= ~FALSE;		I have NO idea what this does! */
  }
  addcarray(string, &carray[ncand - 1], mode, mode & TRUE);
}


/* Star expands * ? and [] in the carray */
#ifdef PROTO
static void star(struct candidate *curr)
#else
static void
star(curr)
  struct candidate *curr;
#endif
{
  char dir[MAXWL];
  struct candidate *a;
  int base;

  Mode = curr->mode & ~(TRUE | C_SPACE);
  base = ncand;			/* Save start of expansion */
  dir[0] = EOS;
  finddir(curr->name, dir);	/* expand them */
  a = curr->next;
  if (!matchdir(dir, curr->name))
  {
    qsort((void *) &carray[base], ncand - base, sizeof(struct candidate), compare);
    /* Now insert into list */
    carray[ncand - 1].next = a;
    curr->next = &carray[base];
    for (; base < ncand - 1; base++)
      carray[base].next = &carray[base + 1];
  }
  if (curr->mode & TRUE)
  {
    free(curr->name);
    curr->mode = FALSE;
  }
  curr->name = NULL;
#ifdef DEBUG
  fprints(2, " In star ");
  listcarray();
#endif
}

/* Getbackqline is called when backquot forks to run a `command`.
 */
#ifdef PROTO
static bool getbackqline( uchar *line, int *nosave)
#else
static bool
getbackqline(line, nosave)
  uchar *line;
  int *nosave;
#endif
{
  *nosave = 0;
  if (qline)
  {
    strcpy(line, qline);
    qline = NULL;
    return (TRUE);
  }
  else
    return (FALSE);
}

/* Backquot expands backquotes */
#ifdef PROTO
static void backquot(struct candidate *curr)
#else
static void
backquot(curr)
  struct candidate *curr;
#endif
{
#ifdef PROTO
  extern bool(*getaline) (uchar *line , int *nosave );
#else
  extern bool(*getaline) ();
#endif
  extern int saveh;
  struct candidate *first;
  int term, base;
  int pfd[2];
  char *line;

  term = pipe(pfd);
  if (term == -1)
  {
    fprints(2, "Cannot open pipe in backquot\n");
    return;
  }
  switch (fork())
  {
    case -1:
      fatal("Bad fork in meta_4\n");

    case 0:
      /* Redirect our output */
      close(pfd[0]);
      close(1);
      dup(pfd[1]);
      close(pfd[1]);
      qline = curr->name;
      getaline = getbackqline;	/* Pass line to doline */
      saveh = FALSE;
      doline(FALSE);
      exit(0);			/* and die */
    default:
      close(pfd[1]);
      base = ncand;		/* Get start of expansion */
      first = curr;
      curr = curr->next;	/* Move curr up now */
      line = (char *) malloc(MAXLL);
      if (line == NULL)
      {
	fprints(2, "Can't malloc line in backquot\n");
	return;
      }
      fileopen(NULL, pfd[0]);
      while (getfileline(line, &term))
	meta_1(line, C_SPACE | TRUE);	/* Add words to the carray */
      fileclose();
      free(line);
      /* Delete first node */
      if (first->mode & TRUE)
	free(first->name);
      first->name = NULL;
      first->mode = 0;
      /* Insert words in list */
      if (ncand > base)
      {
	carray[base - 1].next = NULL;	/* Undo addcarray */
	first->next = &carray[base];
	carray[ncand - 1].next = curr;
      }
  }
#ifdef DEBUG
  fprints(2, " In backquot ");
  listcarray();
#endif
}

/* Meta_1 takes the user's input line (old), and builds a linked list of words
 * in the carray. It also expands history. Meta_1 returns TRUE if
 * successful; if any errors occur, it returns FALSE.
 *
 * If mustmalc is TRUE, meta_1 assumes the word must be strcpy'd.
 * As of version 38, support for double quotes is gone, and backquotes are
 * treated in the same way as single quotes.
 */
bool
meta_1(old, mustmalc)
  char *old;
  bool mustmalc;
{
  char bq[3];			/* Used to find backslashes in quotes */
  char *a, *old2;
  char c;
  int mode;
  int lastdollar = 0;		/* Was last word a variable? */

  bq[0]= '\\'; bq[2]= EOS;
  while (1)			/* Parse each word */
  {
    a = strpbrk(old, " \t\n\\'`{;|<>&$");
    /* why `&& *old' below - Not all strpbrk()s give NULL when *old=EOS */
    if (a != NULL && *old)	/* We found a word */
    {
      c = *a;
      *a = EOS;
      mode = (c == ' ' || c == '\t') ? C_SPACE : 0;
      /* Add the word to the list */
      if (a > old)
	addword(old, mode | lastdollar | mustmalc);

      mode = 0;
      switch (c)		/* Now deal with the `token' */
      {
	case '{':
	  mode = C_CURLY;
	  c = '}';
	  goto quotes;		/* Yuk, a goto */
	case '`':
	  mode = C_BACKQUOTE;
	  goto quotes;		/* Yuk, a goto */
	case '\'':
	  mode = C_QUOTE;
      quotes:
	  old2= old = ++a; bq[1]= c;
      quotes2:
	  a = strpbrk(old2,bq);	/* Find either slosh or matching quote */
	  if (a == NULL)
	  {
	    fprints(2, "No matching %c\n", c);
	    return (FALSE);
	  }
	  if (*a == '\\')
	   {
	    if (a>old)		/* there is something before the slosh */
	     {
		*a= EOS;
	        addword(old, mode | lastdollar | mustmalc);
	     }
	    old= ++a; old2= old+1;
	    goto quotes2;	/* Yuk, another goto */
	   }
	  *(a++) = EOS;		/* Terminate the word */
	  if (*a == ' ' || *a == '\t')	/* Check for trailing spaces */
	  {
	    a++;
	    mode |= C_SPACE;
	  }
	  addword(old, mode | lastdollar | mustmalc);
	  break;
	case '\\':
	  old = (char *) malloc(2);
	  if (old)
	  {
	    old[0] = *(a + 1);
	    old[1] = EOS;
	    addword(old, mode | lastdollar | TRUE);
	  }
	  a += 2;
	  break;
	case '|':
	  mode = C_PIPE;
	  goto doubles;		/* Yuk, a goto */
	case '&':
	  mode = C_AMP;
	  goto doubles;		/* Yuk, a goto */
	case '<':
	  mode = C_LT;
      doubles:
	  a++;
	  if (*a == c)
	  {
	    a++;
	    mode += C_DOUBLE;
	  }
	  addword(NULL, mode);
	  break;
	case '>':
	  mode = 1 | C_GT;	/* Default to stdout */
	  switch (*(a + 1))
	  {
	    case '>':
	      mode = 1 | C_GTGT;
	      a++;
	      break;
	    case '&':
	      mode = 2 | C_GT;
	      a++;
	  }
	  addword(NULL, mode);
	  a++;
	  break;
	case ';':
	  addword(NULL, C_SEMI);
	case '$':
		if (lastdollar && (a==old))	/* We found a $$ */
		 {
		  a++; mode= C_DOLLAR | TRUE ;
		  mode|= ((*a==' ')||(*a=='\t')) ? C_SPACE : FALSE;
		  addword("$",mode); a--;
		  c=' ';		/* Don't recognise 2nd $ below */
		 }
	case ' ':
	case '\t':
	  a++;
	case '\n':
	  break;
	default:
	  fprints(2, "Unknown token %c in meta_1", c);
	  return (FALSE);
      }
      lastdollar = (c == '$') ? C_DOLLAR : 0;	/* Mark variables */

      /* Ok, skip intermediate spaces */
      for (old = a; *old && (*old == ' ' || *old == '\t'); old++);
    }
    else
    {
      addword(old, lastdollar | mustmalc);
      break;
    }
  }
#ifdef DEBUG
  fprints(2, " In meta_1 ");
  wordlist = next_word;
  listcarray();
#endif
  return (TRUE);
}


/* Joinup concatenates two carray elements together. The following pointers
 * must be non-null: curr, curr->next, curr->name, curr->next->name.
 */
#ifdef PROTO
static void joinup(struct candidate *curr)
#else
static void
joinup(curr)
  struct candidate *curr;
#endif
{
  char *a;

  a = (char *) malloc((unsigned) (strlen(curr->name) + strlen(curr->next->name) + 1));
  if (!a)
    return;
  strcpy(a, curr->name);
  strcat(a, curr->next->name);
  if (curr->mode & TRUE)
    free(curr->name);
  curr->mode = curr->next->mode | TRUE;
  curr->next = curr->next->next;
  curr->name = a;
}

/* Meta_2 expands $ and ~ in the carray */
void
meta_2()
{
  struct candidate *curr;
  char *a, tildir[MAXWL];

  for (curr = wordlist; curr != NULL; curr = curr->next)
  {
    a = curr->name;
    if (a == NULL || curr->mode & C_QUOTE)
     continue;

#ifndef NO_TILDE
    if (*a == '~')		/* If a ~ */
    {
      tilde(a, tildir);		/* expand it too */
#ifdef DEBUG
      fprints(2, "tildir is %s\n", tildir);
#endif
      if (curr->mode & TRUE)
	free(curr->name);
      curr->name = (char *) malloc((unsigned) strlen(tildir) + 4);
      if (curr->name != NULL)
      {
	strcpy(curr->name, tildir);
	curr->mode = TRUE | C_SPACE;
      }
    }
#endif

    if (curr->mode & C_DOLLAR)	/* If it has a $ */
      dollar(curr);		/* expand it */

  }
  for (curr = wordlist; curr != NULL; curr = curr->next)
  {
    while (curr->name && curr->next && curr->next->name && !(curr->mode & C_SPACE))
      joinup(curr);		/* No space, we'd better join em */

#ifdef NOTYET
    if (curr->mode & C_CURLY)	/* If it is in {} */
      curly(curr);		/* expand it */
#endif

    if (curr->mode & C_BACKQUOTE)	/* Ha! Found a backquote word */
      backquot(curr);

    if (!(curr->mode & C_QUOTE) && curr->name && strpbrk(curr->name, "*?["))
      star(curr);

    while (curr->name && curr->next && curr->next->name
	&& !(curr->mode & ~TRUE) && !(curr->next->mode & ~(TRUE|C_SPACE)))
      joinup(curr);		/* No space, we'd better join em */
  }
#ifdef DEBUG
  fprints(2, " In meta_2 ");
  listcarray();
#endif
}
