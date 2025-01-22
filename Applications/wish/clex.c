/* Here is the declaration of the candidate array. This is used in meta.c
 * and probably in parse.c, as well as several million other places.
 */

#include "header.h"

struct candidate carray[MAXCAN];
extern int ncand;
static int maxlen;

/* Compare is the routine used by qsort to reorder the elements
 * in the carray.
 */
int
compare(a, b)
#ifdef BSD43
  CONST struct candidate *a, *b;
#else
  CONST void *a, *b;
#endif
{
  struct candidate *c, *d;

  c = (struct candidate *) a;
  d = (struct candidate *) b;
  return (strcmp(c->name, d->name));
}

/* Addcarray adds a new node to the carray. If prev is non-null, then
 * prev->next points to the new node. If malc is TRUE, space is malloc'd
 * and the word copied.
 */
void
addcarray(word, prev, mode, malc)
  char *word;
  struct candidate *prev;
  int mode;
  bool malc;			/* Ha ha - a play on clam :-) */
{
  int j;
  struct candidate *here;

  if (ncand == MAXCAN) return;
  here = &carray[ncand];
  if (malc && word)
  { j = strlen(word);
    here->name = (char *) Malloc((unsigned) (j + 2), "addcarray");
    strcpy(here->name, word);
  }
  else here->name = word;
  if (prev >= carray) prev->next = here;
  here->next = NULL;
  here->mode = mode;
  ncand++;
}


#ifndef NO_CLEX
/* Print out the maxlen partial match on the word, placing the result at
 * pos in the given line.
 */
#ifdef PROTO
static void extend(char *line, int *pos, char *word)
#else
static void
extend(line, pos, word)
  char *line;
  int *pos;
  char *word;
#endif
{
  extern char *wbeep;
  extern int beeplength;
  int i, j, nostop = 1;
  char *newword, *t;

  if (*word == '~' || *word == '$') word++;
  t = strrchr(word, '/');			/* Go to the last '/' */
  if (t != NULL) word = ++t;

  switch (ncand)
  {
    case 0:
      Beep;
      return;
    case 1:
      newword = carray[0].name;
    default:
      if ((newword = (char *) malloc((unsigned) maxlen + 2)) == NULL)
	return;
      for (i = 0; i < maxlen + 1; i++)		/* Clear the new word */
	newword[i] = EOS;
      strcpy(newword, word);			/* Set up as much as we have */

      for (i = strlen(word); nostop && i < maxlen; i++)
	for (j = 0; j < ncand; j++)
	{ if (strlen(carray[j].name) <= i)	/* Candidate too short, stop */
	  { newword[i] = EOS; nostop = 0; break; }
	  if (newword[i] == 0)			/* Copy 1 letter over */
	    newword[i] = carray[j].name[i];
	  if (newword[i] != carray[j].name[i])	/* Doesn't match the copy */
	  { newword[i] = EOS;			/* the scrub letter and stop */
	    nostop = 0;
	    break;
	  }
	}
  }
  for (i = strlen(word); i < strlen(newword); i++)
    insert((uchar *) line, (*pos)++, (uchar) newword[i]);

  if (ncand == 1)
  { if ((carray[0].mode & S_IFMT) == S_IFDIR)
      insert((uchar *) line, (*pos)++, '/');
    else
      insert((uchar *) line, (*pos)++, ' ');
  }
  else
  { Beep; free(newword); }
}


/* Print out the candidates found in columns.
 */
#ifdef PROTO
static void colprint(void)
#else
static void
colprint()
#endif
{
  extern int wid;
  int i, j, collength, numperline, index;
  char format[6];

  maxlen += 2;
  numperline = wid / maxlen;
  collength = ncand / numperline;
  if (ncand % numperline) collength++;
  sprints(format, "%%%ds", maxlen);
  for (i = 0; i < collength; i++)
  { write(1, "\n", 1);
    for (j = 0; j < numperline; j++)
    { index = i + j * collength;
      if (index >= ncand) break;
      if ((carray[index].mode & S_IFMT) == S_IFDIR)
	strcat(carray[index].name, "/");
#ifdef S_IFLNK
      else if ((carray[index].mode & S_IFMT) == S_IFLNK)
	strcat(carray[index].name, "@");
#endif
      else if (carray[index].mode & 0111)
	strcat(carray[index].name, "*");
      prints(format, carray[index].name);
    }
  }
  write(1, "\n", 1);
}




/* Find the name of a file, given a partial word to match against. The
 * word may be an absolute path name, or a relative one. Any matches
 * against the word are added to the carray.
 */
#ifdef PROTO
static void findfile(char *word)
#else
static void
findfile(word)
  char *word;
#endif
{
  extern char currdir[];
  int i, j;
  char partdir[MAXWL];
  char *cddir;
  char *match;
  DIR *dirp;
  struct stat statbuf;
#ifdef USES_DIRECT
  struct direct *entry;
#else
  struct dirent *entry;
#endif

  for (i = 0; i < MAXWL; i++)
    partdir[i] = EOS;

  if (word != NULL && *word != EOS)
  {
						/* Make full pathname */
    if (*word != '/')
    { strcpy(partdir, currdir);
      strcat(partdir, "/");
    }
    strcat(partdir, word);

						/* Find the directory name */
    if ((match = strrchr(partdir, '/')) == NULL)
    { prints("Looney! No / in word %s\n", partdir); return; }

    *match = EOS;
    match++;				/* Match holds the partial file name */

    i = strlen(match);			/* Get the length of the name */
  }
  else
  { strcpy(partdir, currdir); i = 0; }

  cddir= partdir;
  if (*partdir == EOS)			/* Can occur when only / is 1st char */
    cddir="/";
  
  if ((dirp = opendir(cddir)) == NULL)
  { prints("Could not open the directory %s\n", cddir); return; }

  if (chdir(cddir) == 0)
    while ((entry = readdir(dirp)) != NULL && ncand < MAXCAN)
    {
						/* Ignore dot and dot-dot */
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
	continue;

				/* If we find a partial match add to list */
      if (i == 0 || !strncmp(entry->d_name, match, i))
      { j = strlen(entry->d_name);
	if (j > maxlen) maxlen = j;
						/* and get the mode as well */
	statbuf.st_mode = 0;
	stat(entry->d_name, &statbuf);
	addcarray(entry->d_name, NULL, statbuf.st_mode, TRUE);
      }
    }
  closedir(dirp);
  chdir(currdir);
}


/* Find the name of a variable, or if word has a / in it, get the
 * variable's value and treat it as a path.
 * Any matches against the word are added to the carray.
 */
#ifdef PROTO
static void finddollar(char *word)
#else
static void
finddollar(word)
  char *word;
#endif
{
  char dir[MAXWL];
  extern struct vallist vlist;
  struct val *v;
  int i, j, mode;
  char *a;

  word++;					/* Skip over dollar sign */
  i = strlen(word);
  if ((a = strchr(word, '/')) != NULL)		/* We have to find a file */
  { *(a++) = EOS;				/* Get the var name */
    for (v = vlist.head; v; v = v->next)
      if (!strncmp(v->name, word, i))
      { sprints(dir, "%s/%s", v->val, a);	/* Form the full path name */
	findfile(dir);				/* and match against it */
      }
    *(--a) = '/';				/* Extend needs to see the / */
    return;
  }

						/* Look through the var names */
  for (v = vlist.head; v; v = v->next)
    if (!strncmp(v->name, word, i))
    { j = strlen(v->name);
      if (*(v->val) == '/') mode = S_IFDIR;	/* Guess if it's a dir */
      else mode = 0;
      addcarray(v->name, NULL, mode, TRUE);
      if (j > maxlen) maxlen = j;
    }
}

/* Find the name of an alias or builtin, given the partial word.
 * Any matches against the word are added to the carray.
 */

#ifdef PROTO
static void findbuilt(char *word)
#else
static void
findbuilt(word)
  char *word;
#endif
{
  extern struct builptr buillist[];
#ifndef NO_ALIAS
  extern struct vallist alist;
#endif
  int h, i, j;
  struct val *v;

  i = strlen(word);

#ifndef NO_ALIAS
  for (v = alist.head; v; v = v->next)
    if (!strncmp(v->name, word, i))
    { j = strlen(v->name);
      addcarray(v->name, NULL, 0700, TRUE);
      if (j > maxlen) maxlen = j;
    }
#endif

  for (h = 0; buillist[h].name; h++)			/* Then the builtins */
    if (i == 0 || !strncmp(buillist[h].name, word, i))
    { j = strlen(buillist[h].name);
      addcarray(buillist[h].name, NULL, 0700, TRUE);
      if (j > maxlen) maxlen = j;
    }
}

/* Find the name of a file, or a user, given the partial word. If there
 * are no slashes, just go for a user, else get the home dir & call
 * findfile. Any matches against the word are added to the carray.
 *
 * This now uses the tilde list.
 */
#ifdef PROTO
static void findpasswd(char *word)
#else
static void
findpasswd(word)
  char *word;
#endif
{
  extern struct vallist tlist;
  struct val *t;
  int i, j;
  char dir[MAXWL];

  struct passwd *entry;

  if (!strncmp(word, "~#/", 3))			/* If looking for builtins, */
  { findbuilt(&word[3]);
    return;
  }						/* use findbuilt() */

  if (strchr(word, '/') != NULL)/* We have to find a file */
  { tilde(word, dir);
    if (*dir != 0) findfile(dir);
    return;
  }

  word++;					/* Skip over tilde */
  i = strlen(word);

						/* Try our tilde list first */
  for (t = tlist.head; t; t = t->next)
    if (!strncmp(t->name, word, i))
    { j = strlen(t->name);
      addcarray(t->name, NULL, S_IFDIR, TRUE);
      if (j > maxlen) maxlen = j;
    }

  while ((entry = getpwent()) != NULL && ncand < MAXCAN)
  {
    if (i == 0 || !strncmp(entry->pw_name, word, i))
    { j = strlen(entry->pw_name);
      addcarray(entry->pw_name, NULL, S_IFDIR, TRUE);
      if (j > maxlen) maxlen = j;
    }
  }
  endpwent();
}

/* Find the name of a file by looking through $PATH.
 * Any matches against the word are added to the carray.
 */
#ifdef PROTO
static void findbin(char *word)
#else
static void
findbin(word)
  char *word;
#endif
{
  int i;
  char word2[MAXWL];
  char *Path, *thispath, *temp;

  temp = EVget("PATH");				/* Get the PATH value */
  if (temp == NULL) return;
  Path = thispath = (char *) malloc((unsigned) strlen(temp) + 4);
  if (thispath == NULL) return;
  strcpy(thispath, temp);
  while (thispath != NULL)
  { for (i = 0; i < MAXWL; i++) word2[i] = EOS;
    temp = thispath;
    while (*temp != EOS && *temp != ' ' && *temp != ':') temp++;
    if (*temp == EOS) temp = NULL;
    else *(temp++) = EOS;

    strcpy(word2, thispath);
    strcat(word2, "/");
    if (word != NULL || *word != EOS) strcat(word2, word);

    findfile(word2);

    while (temp != NULL && *temp != EOS && (*temp == ' ' || *temp == ':'))
      temp++;
    if (temp == NULL || *temp == EOS) thispath = NULL;
    else thispath = temp;
  }
  free(Path);
}


/* Complete subsumes the work of two routines in old Clam, depending on how:
 *
 * case 0:	Print out a columnated list of files that match the word
 *		at position pos. The line is unchanged.
 * case 1:	Try to complete as much as possible the word at pos, by
 *		using the find routines above. Add the completion to the line.
 */
void
complete(line, pos, how)
  char *line;
  int *pos;
  bool how;
{
  extern char *wbeep, yankbuf[];
  extern int beeplength;
  extern char *wordterm;
  char *b, *yankword = yankbuf;
  int startpos;

  ncand = maxlen = 0;

  if (line[(*pos) - 1] == ' ')
  {
    strcpy(yankbuf, "");
    startpos = *pos - 1;
  }						/* nothing there to get */
  else
  { b = wordterm;		/* first get the thing we've got so far */
    wordterm = " ";
    startpos = yankprev((uchar *) line, *pos);
    wordterm = b;
  }

  if (*yankword == '\'' || *yankword == '"' || *yankword == '`')
    yankword++;
  if (startpos == 0 && *yankword != '.' && *yankword != '/' && *yankword != '~'
      && *yankword != '$')
    findbin(yankword);
  else
    switch (*yankword)
    { case '~':
	findpasswd(yankword); break;
      case '$':
	finddollar(yankword); break;
      default:
	findfile(yankword);
    }

  if (how == TRUE) extend(line, pos, yankword);
  else
  { if (ncand == 0) Beep;
    else
    { qsort((void *) carray, ncand, sizeof(struct candidate), compare);
      colprint();
      prprompt();
      show((uchar *) line, TRUE);
    }
  }
  while ((--ncand) >= 0) free(carray[ncand].name);
}
#endif	/* NO_CLEX */
