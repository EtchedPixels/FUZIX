/*      7th Edition Job Control functions
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 *
 */
#ifndef WIFSTOPPED
#define WIFSTOPPED(x)   (((x) & 0xFF) == 0x7F)	/* As per my Version7 manual */
#define WIFSIGNALED(x)  ((x) & 0xFF)
#define WIFEXITED(x)    (((x) & 0xFF) == 0)
#define WEXITSTATUS(x)	 ((x) >> 8)
#define WTERMSIG(x)	 ((x) & 0xFF)
#define WSTOPSIG(x)	 ((x) >> 8)
#endif
#define WIFCORE(x)	 ((x) & 0x80)
#define WRUNFG(x)	 ((x) == RUNFG)
#define WRUNBG(x)	 ((x) == RUNBG)

/* The following tables are all VERY OS-dependent, especially on POSIX
 * systems. If you find the shell giving you weird signal messages, you
 * should change the names of the signals below.
 */
static char *siglist[] = {
	"", "Hangup", "Interrupt", "Quit", "Illegal Instruction",
	"Trace/BPT Trap", "IOT Trap", "EMT Trap", "Floating Point Exception",
	"Killed", "Bus Error", "Segmentation Violation", "Bad System Call",
	"Broken Pipe", "Alarm", "Terminated", "User Signal 1", "User Signal 2",
	"Child Death", "Power Failure"
};

char *signame[] = {
	"", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL",
	"BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "USR1", "USR2", "CLD",
	"PWR", NULL
};


/* Waitfor is called to wait for a requested job to stop/die. It receives the
 * new status of any children. If it's the requested job, we return. Pid holds
 * the process-id of the job we are after; if it is 0, we return when no
 * processes are left waiting to report a change in state.
 *
 * If pid is negative, we are a subshell and _must_ return only when the pid
 * dies. We set the subshell flag and make pid positive again. Then, if we
 * find out that the job was stopped, we wake it up. This works because we
 * are put to sleep at the same time as the child, and the original shell
 * fg()s _us_.
 */
void
waitfor(pid)
  int pid;
{
  struct job *thisjob;
  int wpid;
  bool subshell=FALSE;
  int stopsig;

  int status;

#ifdef DEBUG
  fprints(2,"In waitfor\n");
#endif

  if (pid<0) { pid= -pid; subshell= TRUE; }
  while (1)
  {
    if (pid == 0)
      return;
    wpid = wait(&status);

#ifdef DEBUG
    fprints(2,"waitxx() returned with %x\n",status);
#endif
    if (wpid == -1 || wpid == 0)
      break;

    thisjob = findjob(wpid);
    if (thisjob == NULL)
      continue;

/* Version 7 job control is quite interesting. Since we don't have a ^Z
 * key, we use the ^\ key instead. Luckily this is caught by the parent
 * and stops the process. When any signal is caught by the child, it
 * stops. We must arrange to stop it on SIGQUIT (^\), and restart it on
 * any other signal. 
 */
    if (WIFSTOPPED(status))
     {
	stopsig= WSTOPSIG(status);
#ifdef DEBUG
	fprints(2,"Stopped status %x, stopsig %d (%s)\n",
		status,stopsig,signame[stopsig]);
#endif
	if (stopsig!=SIGQUIT)
	 {
	  if (stopsig== SIGTRAP)
	  {
#ifdef DEBUG
	fprints(2,"About to ptrace(7,%d,1,0)\n",wpid);
#endif
            ptrace(7,wpid,(PLONG)1,(PLONG)0);	/* Keep it going after exec */
	  }
	  else
	  {
#ifdef DEBUG
	fprints(2,"About to ptrace(7,%d,1,stopsig)\n",wpid);
#endif
            ptrace(7,wpid,(PLONG)1,(PLONG)stopsig); /* Deliver the signal */
	  }
          continue;
	 }
     }		/* SIGQUIT falls out to below where job is marked stopped */

    thisjob->status = status;
    thisjob->changed = TRUE;
    if (pid == wpid)
      return;
  }
}

#ifdef PROTO
static void bgstuff(int pid)
#else
static void
bgstuff(pid)
  int pid;
#endif
{
  struct job *ptr;

#ifdef DEBUG
  fprints(2,"About to ptrace(7,%d,1,0)\n", pid);
#endif
   ptrace(7,pid,(PLONG)1,(PLONG)0);	/* Start it up again */
  if (ptr)
    { ptr->STATUS = RUNBG; currentjob= ptr; }
}

#ifdef PROTO
static int fgstuff(int pid)
#else
static int
fgstuff(pid)
  int pid;
#endif
{
  struct job *ptr;

  ptr=findjob(pid);

/* We need to wake up stopped jobs before we can wait on them */
  if ((ptr) && WIFSTOPPED(ptr->status))
   {
#ifdef DEBUG
    fprints(2,"About to ptrace(7,%d,1,0)\n", pid);
#endif
    ptrace(7,pid,(PLONG)1,(PLONG)0);	/* Start it up again */
   }

  ptr->STATUS = RUNFG;
  currentjob = ptr;
  return (0);
}
