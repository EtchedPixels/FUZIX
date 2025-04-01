/* Parsing the input file
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

/* The parser uses the symbols parsed by meta.c, but it needs a few more
 * definitions for proper parsing. These are below.
 */

#define C_WORD	C_SPACE		/* This indicates the returned word is a */
 /* usual word */
#define C_EOF	FALSE		/* Returned when no more tokens left */

extern struct candidate *wordlist;	/* The list of words to parse */
static struct candidate *pcurr;	/* A ptr to the word we are parsing */

/* Gettoken returns the symbol which stands for the current word, or 0.
 * If 0, word points to a string that holds a normal word. This is guaranteed
 * not to be clobbered until we get back out to main().
 */
#ifdef PROTO
static int gettoken(char **word, int *fd)
#else
static int 
gettoken(word, fd)
  char **word;
  int *fd;
#endif
{
  int mode;

  /* Skip any null words */
  while (1)
  {
    /* Return EOF on end of list */
    if (pcurr == NULL)
      return (C_EOF);
    mode = pcurr->mode;

    if (mode & C_WORDMASK)	/* We've found a token, return */
    {
      *word = NULL;
      *fd = mode & C_FD;
      mode &= C_WORDMASK;	/* stripping any fd */
      pcurr = pcurr->next;
      return (mode);
    }

    if (pcurr->name && *(pcurr->name) != EOS)
    {
	*word = pcurr->name;	/* just return  it */
	pcurr = pcurr->next;
	return (C_WORD);
    }
    pcurr = pcurr->next;	/* Not a valid word or symbol, skip */
  }
}

/* Command parses the input and calls invoke() to redirect and execute
 * each simple command in the parsebuf. It returns the pid to wait on
 * in waitpid, or 0 if no pid to wait on, as well as the token that ended
 * the pasebuf. This routine is recursive, and when passed makepipe=TRUE,
 * makes a pipe, and returns the fd of the writing end in pipefdp.
 */
int 
command(waitpid, makepipe, pipefdp, anydups)	/* Do simple command */
  int *waitpid, *pipefdp;
  int makepipe, anydups;
{
  int token, term;
  int argc, pid, pfd[2];
  char *argv[MAXARG + 1];
  char *word;
  int i, fd, how = 0;
  struct rdrct newfd[3];

  argc = 0;
  argv[0] = NULL;
  for (i = 0; i < 3; i++)
  {
    newfd[i].fd = 0;
    newfd[i].how= 0;
    newfd[i].file = NULL;
  }
  if (makepipe == FALSE)
    pcurr = wordlist;		/* Start parsing the wordlist */

  while (1)
  {
    token = gettoken(&word, &fd);
#ifdef DEBUG
    prints("Token is %o", token);
    if (word != NULL)
      prints(" word is %s", word);
    write(1, "\n", 1);
#endif
    switch (token)
    {
      case C_WORD:
	if (argc == MAXARG)
	{
	  fprints(2, "Too many args\n");
	  break;
	}
	argv[argc++] = word;
	continue;
      case C_LT:
	if (makepipe)
	{
	  fprints(2, "Extra <\n");
	  break;
	}
	if (gettoken(&(newfd[fd].file), &i) != C_WORD)
	{
	  fprints(2, "Illegal <\n");
	  break;
	}
	newfd[fd].how = H_FROMFILE;
	anydups= TRUE;
	continue;
      case C_GT:
      case C_GTGT:
	if (newfd[fd].fd != 0 || newfd[fd].file != NULL)
	{
	  fprints(2, "Extra > or >>\n");
	  break;
	}
	if (gettoken(&(newfd[fd].file), &i) != C_WORD)
	{
	  fprints(2, "Illegal > or >>\n");
	  break;
	}
	if (token == C_GTGT)
	  newfd[fd].how = H_APPEND;
	anydups= TRUE;
	continue;
      case C_AMP:		/* If a pipe, call ourselves to get */
      case C_EOF:		/* the write file descriptor */
      case C_SEMI:
      case C_DBLAMP:
      case C_DBLPIPE:
      case C_PIPE:
	argv[argc] = NULL;
	if (token == C_PIPE)
	{
	  anydups= TRUE;
	  if (newfd[1].fd != 0 || newfd[1].file != NULL)
	  {
	    fprints(2, "> or >> conflicts with |\n");
	    break;
	  }
	  term = command(waitpid, TRUE, &(newfd[1].fd),anydups);
	}			/* and set up the terminal token */
	else
	  term = token;
	/* If called by us, make the needed pipe */
	if (makepipe)
	{
	  if (pipe(pfd) == -1)
	  {
	    perror("pipe");
	    break;
	  }
	  /* and return the write file descriptor */
#ifdef DEBUG
	  fprints(2, "Opened pipe fds %d and %d\n", pfd[0], pfd[1]);
#endif
	  *pipefdp = pfd[1];
	  newfd[0].fd = pfd[0];
	}
	if (term == C_AMP)
	  how = H_BCKGND;
	pid = invoke(argc, argv, newfd, how, anydups);
	/* End of command line, return pid to wait */
	if (token != C_PIPE)
	  *waitpid = pid;
	if (argc == 0 && token != C_EOF)
	  fprints(2, "Missing command\n");
	return (term);
      default:
	prints("Unknown token %o in command()\n", token);
    }
  }
}
