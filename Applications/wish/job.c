/* Job control. If UCBJOB is defined, we use BSD4.x job control. If POSIXJOB
 * is defined, we use POSIX job control. IF V7JOB is defined, we provide
 * job control using ptrace(). If none are defined, we simulate job control
 * as best we can.
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

/* We define the wait return status macros for SysV. We also define
 * WIFCORE, which evaluates as true if WIFSIGNALED is TRUE and
 * a core has been dumped. This only works under SysV and BSD.
 * Also defined are WRUNFG and WRUNBG, which are true is the
 * process is running in the fore or background.
 */
#define RUNFG	-1		/* Status if running in fg */
#define RUNBG	-2		/* Status if running in bg */

#define WAIT_T int
#define STATUS status
#ifdef UCBJOB
# undef  WAIT_T
# undef  STATUS
# define WAIT_T union wait
# define STATUS status.w_status
#endif


/* The job structure hold the information needed to manipulate the
 * jobs, using job numbers instead of pids. Note that the linked
 * list used by Wish is ordered by jobnumber.
 */
struct job
{
  int jobnumber;		/* The job number */
  int pid;			/* The pid of the job */
  char *name;			/* Job's argv[0]; */
  char *dir;			/* The job's working directory */
  WAIT_T status;		/* Job's status */
  bool changed;			/* Changed since last CLE? */
  struct job *next;		/* Pointer to next job */
};

static struct job *jtop = NULL,	/* The head of the linked list */
	*currentjob = NULL;	/* Pointer to current job structure */

int Exitstatus = 0;		/* The exit status of the last child */
static bool donepipe = TRUE;	/* Addjob makes a new currentjob if this is */
				 /* TRUE, then sets it FALSE. Joblist sets */
 				/* it TRUE again. */

#ifdef PROTO
# define P(s) s
#else
# define P(s) ()
#endif

#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
static struct job *findjob P((int pid ));
static void rmjob P((struct job *ptr ));
#endif
#undef P

#ifdef POSIXJOB
# include "posixjob.c"
#endif

#ifdef UCBJOB
# include "ucbjob.c"
#endif

#ifdef V7JOB
# include "v7job.c"
#endif

#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
/* Given a process id, return a pointer to it's job structure */
static struct job *
findjob(pid)
  int pid;
{
  struct job *ptr;

  for (ptr = jtop; ptr; ptr = ptr->next)
    if (ptr->pid == pid)
      return (ptr);
  return (NULL);
}
#endif


/* Given a job number, return it's process id */
#ifdef PROTO
static int pidfromjob(int jobno)
#else
static int
pidfromjob(jobno)
  int jobno;
#endif
{
  struct job *ptr;

  for (ptr = jtop; ptr; ptr = ptr->next)
    if (ptr->jobnumber == jobno)
      return (ptr->pid);
  return (-1);
}


/* Add the pid and it's argv[0] to the job list. Return the allocated
 * job number.
 */
int
addjob(pid, name, isbg)
  int pid, isbg;
  char *name;
{
  extern char currdir[];
  int jobno, last = 0;
  struct job *ptr, *old, *new;

#ifndef V7JOB
  if (currentjob && donepipe == FALSE)	/* We already have a current job */
  {
#ifdef DEBUG
    fprints(2, "I already got one, you sons of a donkey\n");
#endif
    return (0);
  }
#endif
  /* Build the structure */
  ptr = old = (struct job *) malloc((unsigned) sizeof(struct job));
  if (ptr == NULL)
  {
    perror("addjob");
    return (0);
  }
  ptr->pid = pid;
  ptr->name = (char *) malloc((unsigned) (strlen(name) + 1));
  if (ptr->name)
    (void) strcpy(ptr->name, name);
  else
  {
    perror("addjob");
    return (0);
  }
  /* We need some way of indicating the child */
  /* is still running. I am using RUNxx for now. */
  if (isbg)
    ptr->STATUS = RUNBG;
  else
    ptr->STATUS = RUNFG;

  ptr->dir = (char *) malloc((unsigned) (strlen(currdir) + 1));
  if (ptr->dir)
    (void) strcpy(ptr->dir, currdir);
  else
  {
    perror("addjob");
    return (0);
  }
  ptr->changed = FALSE;
  
  /* Find the end of list */
  if (jtop == NULL)
  {
    jobno = ptr->jobnumber = 1;
    jtop = ptr; ptr->next = NULL;
  }
  else
  {				/* Find a gap to put the node in */
    for (last = 0, old = new = jtop;
	 new != NULL && new->jobnumber - last == 1;
	 old = new, last = new->jobnumber, new = new->next);
    last++;
    if (new == NULL)
    {
      old->next = ptr;
      ptr->next = NULL;
    }
    else
    {
      ptr->next = new;
      old->next = ptr;
    }
    jobno = ptr->jobnumber = last;
  }

  currentjob = ptr;		/* a new current job */
  donepipe = FALSE;
#ifdef DEBUG
  fprints(2, "added jobno %d pid %d\n", jobno, pid);
#endif
  return (jobno);
}

#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
/* Remove the job from the job list */
static void
rmjob(ptr)
  struct job *ptr;
{
  struct job *prev;

  if (ptr==NULL) return;
#ifdef DEBUG
  fprints(2, "removing pid %d from list\n", ptr->pid);
#endif
  if (ptr==jtop) jtop=jtop->next;
  else
   {
     for (prev=jtop;prev && prev->next!=ptr; prev=prev->next)
       ;
     if (prev==NULL) return;
     prev->next= ptr->next;
   }
	free(ptr->name);
	if (ptr == currentjob)
	{
	  currentjob = NULL;
	  Exitstatus = (WIFEXITED(ptr->status)) ? WEXITSTATUS(ptr->status) : 1;
	}
	free(ptr);
}
#endif

/* Print out the list of current jobs and their status.
 * Note although this is a builtin, it is called from
 * main with an argc value of 0, to show new jobs only.
 */
int
joblist(argc, argv)
  int argc;
  char *argv[];
{
#ifndef FUZIX
  struct job *ptr, *next;
  bool exitzero;

  if (argc > 1)
  {
    prints("Usage: jobs\n");
    return (1);
  }

#if defined(UCBJOB) || defined(POSIXJOB)
  waitfor(0);			/* Collect any jobs doline() might have missed */
#endif

  for (ptr = jtop; ptr; ptr = ptr->next)
  {
/* Printing out conditions are tricky. We print if we're being called as
 * a builtin (when argc!=0); otherwise only if the status has changed,
 * and it wasn't the current job or exited with an exit status of 0.
 * Also don't print if it's RUNFG or RUNBG.
 */
    exitzero = (WIFEXITED(ptr->status) && (WEXITSTATUS(ptr->status) == 0));
    if (argc || (ptr->changed && !WRUNFG(ptr->status) && !WRUNBG(ptr->status)
		 && ptr != currentjob && !exitzero))
    {
      if (currentjob && currentjob->pid == ptr->pid)
	prints("    + [%d] %d", ptr->jobnumber, ptr->pid);
      else
	prints("      [%d] %d", ptr->jobnumber, ptr->pid);
      prints(" %s  ", ptr->name);
      /* Still running */
      if (WRUNFG(ptr->status) || WRUNBG(ptr->status))
      {
	prints("\n");
	continue;
      }
      if (WIFEXITED(ptr->status))
      {
	prints(" Exited %d\n", WEXITSTATUS(ptr->status));
	continue;
      }
      if (WIFSTOPPED(ptr->status))
      {
	prints(" Stopped %s\n", siglist[WSTOPSIG(ptr->status)]);
	continue;
      }
      if (WIFSIGNALED(ptr->status))
      {
	prints(" %s", siglist[WTERMSIG(ptr->status)]);
	if (WIFCORE(ptr->status))
	  prints(" (core dumped)\n");
	else
	  prints("\n");
	continue;
      }
    }
  }
  for (ptr = jtop; ptr; ptr = next)
  {
   next= ptr->next;
   if (WRUNFG(ptr->status) || WRUNBG(ptr->status)|| WIFSTOPPED(ptr->status))
      ptr->changed = FALSE;
    else
      rmjob(ptr);
  }
  donepipe = TRUE;		/* It must be, if doline() is calling us */
#endif
  return (0);
}

int
Kill(argc, argv)
  int argc;
  char *argv[];

{
  int pid, sig = 0;
  char *jobname, *sigwd;
#ifdef V7JOB
  struct job *job;
#endif

  switch (argc)
  {
    case 2:
      sig = 9;
      jobname = argv[1];
      goto killit;		/* Yuk, a goto */
    case 3:
      jobname = argv[2];
      sigwd = argv[1];
      if (*sigwd == '-')
	sigwd++;
      sig = atoi(sigwd);
#ifdef FUZIX
      goto killit;		/* Yuk, a goto */
#else
      if (sig == 0)
	for (i = 1; signame[i]; i++)
	  if (!strcmp(sigwd, signame[i]))
	  {
	    sig = i;
	    break;
	  }
#endif
  killit:
      if (sig == 0)
      {
	fprints(2,"Bad signal argument\n");
	return (1);
      }
      if (*jobname == '%')
	pid = pidfromjob(atoi(++jobname));
      else
	pid = atoi(jobname);
      if (pid < 1)
      {
	fprints(2, "No such job number: %s\n", jobname);
	return (1);
      }
#ifdef V7JOB
/* A stopped job can't be killed until the parent (i.e us) restarts it.
 * So if the job is stopped, we restart it so we can kill it :-)
 */
      job= findjob(pid);
      if (job && WIFSTOPPED(job->status))
	{
# ifdef DEBUG
	fprints(2,"About to restart ptrace the job\n");
# endif
     	  ptrace(7,pid,(PLONG)1,(PLONG)0);	/* Start it up again */
	}
#endif
      kill(pid, sig);
      return (0);
    default:
#ifdef FUZIX
      prints("Usage: kill [-]number job|pid\n");
#else
      prints("Usage: kill [-]signame|number job|pid\n");
      prints("  Signal names are:\n");
      for (i = 1; signame[i]; i++)
      {
	if (i % 6 == 1)
	  prints("    ");
	prints("%s  ", signame[i]);
	if (i % 6 == 0)
	  prints("\n");
      }
      prints("\n");
#endif
      return (1);
  }
}

#ifdef FUZIX
void
waitfor(pid)
  int pid;
{
  int status;
  int wpid;
  do {
    wpid = wait(&status);
  } while (wpid != pid);
}
#endif


#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
/* Builtins */
int
bg(argc, argv)
  int argc;
  char *argv[];

{
  int pid=0;
  char *c=NULL;

  if (argc > 2)
  {
    prints("usage: bg [pid]\n");
    return (1);
  }
  if (argc == 1)
  {
#ifdef DEBUG
    fprints(2,"Trying to use current job pointer\n");
#endif
    if (currentjob)
    {
      pid = currentjob->pid;
    }
  }
  else
  {
    c = argv[1];
    if (*c == '%')
      pid = pidfromjob(atoi(++c));
    else
      pid = atoi(c);
  }
  if (pid < 1)
  {
    fprints(2, "No such job number: %s\n", c);
    return (1);
  }
  bgstuff(pid);

  return (0);
}


/* Fg is a special builtin. Instead of returning an exitstatus, it returns
 * the pid of the fg'd process. Builtin() knows about this.
 */
int
fg(argc, argv)
  int argc;
  char *argv[];

{
  extern char currdir[];
  extern struct vallist vlist;
  int pid;
   struct job *ptr;
  char *c;

  if (argc > 2)
  {
    prints("usage: fg [pid]\n");
    return (0);
  }
  if (argc == 1)
  {
#ifdef DEBUG
    fprints(2,"Trying to use current job pointer\n");
#endif
    if (currentjob)
    {
      pid = currentjob->pid;
    }
  }
  else
  {
    c = argv[1];
    if (*c == '%')
      pid = pidfromjob(atoi(++c));
    else
      pid = atoi(c);
  }
  if (pid < 1)
  {
    fprints(2, "No such job number: %s\n", c);
    return (0);
  }

  if (fgstuff(pid)) return(0);

  ptr = findjob(pid);
  if (ptr)
  {
    if (strcmp(ptr->dir, currdir))	/* If directory has changed */
     {
      fprints(2, "[%d] %d %s (wd now: %s)\n", ptr->jobnumber, ptr->pid,
	      ptr->name, ptr->dir);

      chdir(ptr->dir);			/* Reset our current directory */
      strcpy(currdir,ptr->dir);
      setval("cwd", currdir, &vlist);
     }
    else
      fprints(2, "[%d] %d %s\n", ptr->jobnumber, ptr->pid, ptr->name);
  }
  else
  {
    fprints(2, "Couldn't find job ptr in fg\n");
    return (0);
  }
					/* and finally wake them up */

#ifndef V7JOB
# ifdef DEBUG
  fprints(2,"About to SIGCONT %d\n", pid);
# endif
  kill(pid, SIGCONT);
#endif

    ptr->STATUS = RUNFG;
  currentjob = ptr;
  return (pid);
}
#endif /* defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB) */
