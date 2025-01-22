/* Execute commands
 *
 * $Revision: 41.3 $ $Date: 2003/04/21 13:09:31 $
 */

#include "header.h"

struct dupstack
{
  struct dupstack *prev;
  int newfd[3];
};

static struct dupstack *duptop = NULL;

/* Unredirect takes the last set of fds pushed on the dup stack, and
 * dups them back to where they came from. Nd is used by redirect()
 * to undo any dups if an error occurs.
 */
#ifdef PROTO
static void undirect(struct dupstack *nd)
#else
static void 
undirect(nd)
  struct dupstack *nd;
#endif
{
  int i;
  struct dupstack *d;

  if (nd)
    d = nd;
  else
  {
    if (duptop == NULL)
    {
      fprints(2, "Nothing on dupstack to pull!\n");
      return;
    }
    d = duptop;
  }

  for (i = 0; i < 3; i++)
  {
    if (d->newfd[i])
    {
      close(i);			/* Close the one we are using */
      /* and dup back the old one */
      if (dup2(d->newfd[i], i) == -1)
      {
	fprints(2, "Can't undup fd %d\n", i);
      }
      close(d->newfd[i]);
    }
  }
  /* Now pull the dup object off the stack */
  if (!nd)
    duptop = d->prev;
  free(d);
}

/*
 * Redirect redirects the file descriptors for fds 0 to 2 for the
 * current process; for each fd it also takes a bitmap indicating whether
 * to append to the output file, or to open from an input file.
 * If an error occurs, we return -1, else 0. We only push a dupstack
 * if no error occurs, so we must not call undirect afterwards.
 */
#ifdef PROTO
static int redirect(struct rdrct newfd[3])
#else
static int
redirect(newfd)
  struct rdrct newfd[3];
#endif
{
  int i, j, appnd;
  int mode, flags;
  struct dupstack *d;

#ifdef DEBUG
  fprints(2, "Redirect...\n");
#endif


  d = (struct dupstack *) Malloc(sizeof(struct dupstack), "redirect");

  /* Clear all the fields */
  d->newfd[0] = d->newfd[1] = d->newfd[2] = 0;

  for (i = 0; i < 3; i++)	/* Do each fd in turn */
  {
    /* Not this one */
    if (newfd[i].fd < 3 && newfd[i].file == NULL)
      continue;
    appnd = newfd[i].how & H_APPEND;


    j = dup(i);			/* Copy that file desc */
    if (j == -1)
    {
      fprints(2, "Couldn't dup fd %d\n", i);
      undirect(d);
      return (-1);
    }
    else
      d->newfd[i] = j;

    if (newfd[i].fd > 2)	/* Fd is already open */
    {
      close(i);
      if (dup2(newfd[i].fd, i) == -1)
      {
	fprints(2, "Could not dup2(%d,%d)\n", newfd[i].fd, i);
	undirect(d);
	return (-1);
      }
#ifdef DEBUG
      fprints(2, "Dup'd %d to %d\n", newfd[i].fd, i);
#endif
      close(newfd[i].fd);
    }
    /* Open this file */
    else
    {
      close(i);
      if (newfd[i].how & H_FROMFILE)	/* for reading */
      {
	flags = O_RDONLY;
	if (open(newfd[i].file, flags) == -1)
	{
	  fprints(2, "Can't open %s\n", newfd[i].file);
	  undirect(d);
	  return (-1);
	}
      }
      else
	/* for writing */
      {
	flags = O_WRONLY | O_CREAT;
	if (!appnd)
	  flags |= O_TRUNC;
	else
	  flags |= O_APPEND;
	mode = 0666;
	if (open(newfd[i].file, flags, mode) == -1)
	{
	  fprints(2, "Can't open %s\n", newfd[i].file);
	  undirect(d);
	  return (-1);
	}
#ifdef DEBUG
	fprints(2, "Opened %s as %d\n", newfd[i].file, i);
#endif
      }
    }
  }
  /* Now put the old fd's on the stack */
  d->prev = duptop;
  duptop = d;
  return (0);
}

#ifndef NO_ALIAS
#define MAXAL	16
static char *curr_alias[MAXAL];	/* The list of aliases we have done */
static int adepth = 0;		/* Our current depth */

/* Runalias checks to see if there is an alias, and then runs it.
 * It either returns the exit status of the alias, or -1 if there
 * was no alias. The only bit in how we are interested in is
 * H_BCKGND. This gets passed onto doline to indicate how to waitfor()
 * the child.
 */
#ifdef PROTO
static int runalias(int argc, char *argv[], int how)
#else
static int
runalias(argc, argv, how)
  int argc;
  char *argv[];
  int how;
#endif
{
  extern int Argc, saveh, Exitstatus;
  extern char **Argv;
#ifdef PROTO
  extern bool(*getaline) (uchar *line , int *nosave );
  bool(*oldgetline) (uchar *line , int *nosave );
#else
  extern bool(*getaline) ();
  bool(*oldgetline) ();
#endif

  int i, oldsaveh, oldargc;
  char **oldargv;

  /* Check for already used alias */
  for (i = 0; i < adepth; i++)
    if (!strcmp(curr_alias[i], argv[0]))
      return (-1);

  if (checkalias(argv[0]) != NULL)	/* Find an alias */
  {
    oldargc = Argc;
    oldargv = Argv;
    Argc = argc;
    Argv = argv;
    oldsaveh = saveh;
    oldgetline = getaline;
    if (adepth == MAXAL)
    {
      fprints(2, "Aliases nested too deep\n");
      /* exit(1); */
      return (-1);
    }
    curr_alias[adepth++] = argv[0];
#ifdef DEBUG
    fprints(11, "About to send the alias through doline()\n");
#endif
    saveh = FALSE;
    how= (how & H_BCKGND) | TRUE;
    getaline = getaliasline;
    doline(how);
    adepth--;
    Argc = oldargc;
    Argv = oldargv;
    getaline = oldgetline;
    saveh = oldsaveh;
    return (Exitstatus);
  }
  else
    return (-1);
}
#endif	/* NO_ALIAS */


static int bgpgrp;	/* Background process group id */

/* Invoke simple command. This takes the args to pass to the executed
 * process/builtin, the list of new file descriptors, and a bitmap
 * indicating how to run the process. We only use the background bits
 * at the moment.
 */

int
invoke(argc, argv, newfd, how, anydups)
  int argc, how;
  char *argv[];
struct rdrct newfd[];
int anydups;

{
  extern int saveh, Exitstatus;
  int pid, i;

#ifdef DEBUG
  if (argv[0])
    fprints(2, "Invoking %s\n", argv[0]);
#endif
  /* Firstly redirect the input/output */
  if (anydups)
  {
    i = redirect(newfd);
    if (i == -1)
      return (0);
  }

  if (!(how & H_BCKGND))	/* If a foreground process */
  {
    bgpgrp=0;			/* Reset for next bg process */
    /* Try builtins first */
    if (argc == 0 || ((i = builtin(argc, argv, &pid)) != -1))
    {
      Exitstatus = i;
      if (anydups)
	undirect(NULL);
      return (pid);
    }

#ifndef NO_ALIAS
    if ((i = runalias(argc, argv, how)) >= 0)
    { Exitstatus= i;
      if (anydups)
	undirect(NULL);
      return (0);
    }
#endif
  }

  /* Else fork/exec the process */
  switch (pid = fork())
  {
    case -1:
      fprints(2, "Can't create new process\n");
      if (anydups)
	undirect(NULL);
      return (0);
      /* Restore signals to normal if fg */
    case 0:
      saveh = FALSE;
      dflsig();
#ifdef V7JOB
      ptrace(0,0,(PLONG)0,(PLONG)0);	/* Enable tracing on the child */
#endif
      if (how & H_BCKGND)
      {				/* Move process to new proc-grp if bg */
#if defined(UCBJOB) || defined(POSIXJOB)
#ifdef POSIXJOB
				/* First get a pgrp-id if none */
	if (bgpgrp==0) bgpgrp=getpid();
	i = setpgid(0, bgpgrp);
	if (i == -1)
	  fprints(2, "I was -1\n");
#else
	i = setpgrp(0, getpid());
	if (i == -1)
	  fprints(2, "I was -1\n");
#endif				/* POSIXJOB */
#endif				/* UCBJOB || POSIXJOB */
      }
#ifndef NO_VAR
      if (!EVupdate())
	fatal("Can't update environment");
#endif
      /* The fork only wants fds 0,1,2 */
#ifdef DEBUG
      fprints(2, "The child is closing 3 to 20\n");
#endif
      for (i = 3; i < 20; i++)
	close(i);

      /* Try builtins first */
      if (argc == 0 || ((i = builtin(argc, argv, &pid)) != -1))
      {
	exit(i);
      }

#ifndef NO_ALIAS
      if ((i = runalias(argc, argv, how)) >= 0)
      { 
	if (anydups)
	  undirect(NULL);
	exit(i);
      }
#endif
      /* Finally exec() the beast */

      execvp(argv[0], argv);
      /* Failed, exit the child */
      fprints(2, "Can't execute %s\n", argv[0]);
      exit(1);

    default:
#ifdef DEBUG
      fprints(2, "Just forked %s, pid %d\n", argv[0], pid);
#endif
      /* and return the new pid */
#ifndef NO_JOB
      addjob(pid, argv[0], how & H_BCKGND);
#endif
#ifdef V7JOB
/* We can't just leave a backgrounded process as is, because it will
 * stop on the exec(), and because we won't waitfor() it, it will stay
 * like that. So we wake it up here.
 */
      if (how & H_BCKGND)
	ptrace(7,pid,(PLONG)1,(PLONG)0);
#endif
      if (anydups)
	undirect(NULL);
      return (pid);
  }
}
