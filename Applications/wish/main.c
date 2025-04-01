/* The main loop of the shell
 */

#include "header.h"
char x,y,z;

int Argc;			/* The number of arguments for Wish */
char **Argv;			/* The argument list for Wish */
static char *prompt;		/* A pointer to the prompt string */
int lenprompt;			/* and the length of the prompt */
int saveh;			/* Should we save history? */
static bool Loginshell=FALSE;	/* Indicates if we're a login shell */
static struct candidate *next_word;

#ifdef PROTO
bool(*getaline) (uchar *line,int *nosave) = getuline;
#else
bool(*getaline) () = getuline;	/* Our input routine defaults to getuline() */
#endif


/* Lengthint is used by prprompt to determine the number
 * of chars in the string version of an integer; this is
 * needed so the prompt length is calculated properly.
 */
#ifdef PROTO
static int lengthint(int num)
#else
static int lengthint(num)
  int num;
#endif
{
  int i;

  for (i = 0; num; num = num / 10, i++);
  return (i);
}

/* Print out the current time in a set length string */
#ifdef PROTO
static void printime(void)
#else
static void
printime()
#endif
{
  long clock;
  struct tm *t;

  time(&clock);
  t = localtime(&clock);
  prints("%2d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
}

/* Prprompt prints out the prompt. It parses all the special options
 * that the prompt can take, and determines the effective length of
 * the prompt as it appears on the screen. If there is no $prompt,
 * it defaults to '% '.
 */
void
prprompt()
{
  extern int curr_hist;
  extern char *so, *se;
  char c;
  int i, len;

  lenprompt = 0;
  prompt = EVget("prompt");
  if (prompt == NULL)
    prompt = "% ";
  for (i = 0; prompt[i]; i++)
    if (prompt[i] == '%')
      switch (prompt[++i])
      {
	case EOS:
	case '%':
	  write(1, "%", 1);
	  lenprompt++;
	  break;
	case '!':
#ifndef NO_HISTORY
	case 'h':
	  lenprompt += lengthint(curr_hist);
	  prints("%d", curr_hist);
	  break;
#endif
	case 'd':
	  len = strlen(EVget("cwd"));
	  lenprompt += len;
	  write(1, EVget("cwd"), len);
	  break;
	case 'S':
	  prints("%s", so);
	  break;
	case 's':
	  prints("%s", se);	/* temp stop standout */
	  break;
	case '@':
	case 't':
	  lenprompt += 8;
	  printime();
	  break;
	default:
	  write(1, "%", 1);
	  lenprompt += 2;
	  if (prompt[i] < 32 || prompt[i] == 127)
	  {
	    write(1, "^", 1);
	    c = prompt[i] + 64;
	    write(1, &c, 1);
	    lenprompt++;
	  }
	  else
	    write(1, &prompt[i], 1);
      }
    else
    {
      if (prompt[i] < 32 || prompt[i] == 127)
      {
	write(1, "^", 1);
	c = prompt[i] + 64;
	write(1, &c, 1);
	lenprompt++;
      }
      else
	write(1, &prompt[i], 1);
      lenprompt++;
    }
}

/* Set the terminal back to normal
 * before leaving the shell
 */
void
leave_shell(how)
  int how;
{
  char *argv[2];

  setcooked();
  if (Loginshell)		/* If we're a login shell */
  {
    argv[0] = "source";
    argv[1] = ".klogout";	/* Source .klogout */
    source(2, argv);
  }
  write(1, "\n", 1);
  exit(how);
}

/* Setup does most of the initialisation of the shell. Firstly the default
 * variables & the environ variables are set up. Then the termcap strings
 * are found, and then any other misc. things are done.
 */
#ifdef PROTO
static void setup(void)
#else
static void
setup()
#endif
{
  extern char currdir[];
  extern struct vallist vlist;
  char *argv[2];
  char *home;

  /* Initialise the environment */
  if (!EVinit())
    fatal("Can't initialise environment");
#ifdef USES_GETWD
  if (getwd(currdir))		/* Get the current directory */
#else
  if (getcwd(currdir, MAXPL))
#endif
    setval("cwd", currdir, &vlist);
  else
    write(2, "Can't get cwd properly\n", 23);
  setval("prompt", "% ", &vlist);	/* Set the prompt to % for now */
  setval("Version", "2.0.41", &vlist);
  catchsig();			/* Catch signals */
  getstty();			/* Set up the stty for Wish */
  terminal();			/* Get the termcap strings */

#ifndef NO_BIND
  initbind();			/* Set the default key bindings */
#endif

  argv[0] = "source";
  home= EVget("HOME");
  if (home)
   { argv[1]= Malloc(strlen(home) + strlen(".wishrc")+3, "setup malloc");
     sprints(argv[1],"%s/.wishrc", home);
   }
  else argv[1] = ".wishrc";		/* Source .wishrc */
  source(2, argv);
  free(argv[1]);
  if (*Argv[0]=='-')		/* If we're a login shell */
  {
    Loginshell= TRUE;		/* Make sure we know */
    if (home)
     { argv[1]= Malloc(strlen(home) + strlen(".wishlogin")+3, "setup malloc");
       sprints(argv[1],"%s/.wishlogin", home);
     }
    else argv[1] = ".wishlogin";	/* Source .wishlogin */
    source(2, argv);
    free(argv[1]);
  }
  saveh = TRUE;
}


/* Doline is the loop which gets a line from the user, runs it through
 * the metachar expansion, and finally calls command() to execute it.
 * It returns when there are no more lines available - so it really
 * should be called dolines()!
 *
 * If isalias is FALSE, we are not expanding aliases. Otherwise we are,
 * so don't clear the carray, our args are in there. Also, how will
 * then hold H_BCKGND if we are a subshell.
 */

void
doline(isalias)
  int isalias;
{
  extern struct candidate carray[], *wordlist;	/* Wordlist is list or words */
  extern int Exitstatus, ncand;
  int i;

/* Nextword holds the word for the next pipeline,
 * termword holds the token that terminated the pipeline.
 */
  struct candidate *termword;
  char *linebuf;
  int pid, q, oldncand;
  int term;

#ifdef DEBUG
  i = (int) sbrk(0);
  prints("In doline, Brk value is %x\n", i);
  prints("Getaline f'n ptr is %x\n", getaline);
#endif
  linebuf = (char *) malloc(MAXLL);
  if (linebuf == NULL)
    fatal("Couldn't malloc linebuf\n");

   while ((*getaline) (linebuf, &q) == TRUE)	/* Get a line from user */
  {
    if (q || *linebuf == EOS)
      continue;

#ifdef DEBUG
    fprints(2, "In doline, line is %s\n", linebuf);
#endif
    if (isalias == FALSE)
      ncand = 0;		/* Initialise the carray */
    next_word = &carray[ncand];	/* list for meta.c */
    oldncand = ncand;

    if (meta_1(linebuf, FALSE) == FALSE)
      continue;			/* Expand ! */
#ifndef NO_HISTORY
    if (saveh)
      savehist(expline(carray), 1);	/* Save the line */
#endif

/* At this point, we need to extract pipelines
 * from the parsed line, so that we expand
 * metachars properly. This means we need yet
 * another loop. Oh well.
 */
    for (wordlist = next_word; wordlist != NULL; wordlist = next_word)
    {
      /* Find pipeline end */
      while (next_word && (i = next_word->mode & C_WORDMASK) != C_SEMI && i != C_DBLAMP
	     && i != C_DBLPIPE)
	next_word = next_word->next;
      /* If we found one */
      if (next_word)
      {
	termword = next_word;	/* Save it */
	next_word = next_word->next;	/* Nextword is next pipeline */
	termword->next = NULL;	/* Terminate our pipeline */
      }
      meta_2();			/* Expand metacharacters */
      term = command(&pid, FALSE, NULL, FALSE);	/* Actually run it here */

      if (isalias & H_BCKGND) pid= -pid;	/* Wait ALWAYS if subshell */
      if (term != C_AMP && pid)
	waitfor(pid);		/* Wait for it to finish */
      if (term == C_DBLAMP && Exitstatus != 0)
	break;
      if (term == C_DBLPIPE && Exitstatus == 0)
	break;
#ifndef NO_JOB
      if (saveh)
	joblist(0, NULL);	/* print the list of jobs */
#endif

    }

    for (i = oldncand; i < ncand; i++)	/* Free the command words */
      if (carray[i].mode == TRUE)
      {
	carray[i].mode = FALSE;
	free(carray[i].name);
      }
    ncand = oldncand;

  }
  free(linebuf);
}

/* Main. This is the bit that starts the whole ball rolling.
 */
int
main(argc, argv)
  int argc;
  char *argv[];

{

  Argc = argc;			/* Set up arg variables */
  Argv = argv;
#ifdef MALLOCDEBUG
  initmall();			/* Initialise the malloc arrays */
#endif
  setup();			/* Set up the vars & the termcap stuff */

  while (1)
    doline(FALSE);		/* We only exit when we shutdown() */
}
