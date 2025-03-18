/*	POSIX Job Control functions
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 *
 */

#define WIFCORE(x)	(FALSE)	/* We can't tell */
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
	"Broken Pipe", "Alarm", "Terminated" ,"Urgent Socket Condition",
	"Stopped (signal)", "Stopped", "Continue", "Child Status Change",
	"(tty input)", "(tty output)", "I/O", "Cpu Time Limit",
	"File Size Limit", "Virtual Time Alarm", "Profile Alarm",
	"Window Change", "Resource Lost", "User Signal 1", "User Signal 2"
};

char *signame[] = {
	"", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "EMT", "FPE", "KILL",
	"BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM" ,"URG", "STOP", "TSTP",
	"CONT", "CHLD", "TTIN", "TTOU", "IO", "XCPU", "XFSZ", "VTALRM", "PROF",
	"WINCH", "LIST", "USR1", "USR2", NULL
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
  int status;

  int waitflags;
  waitflags = (pid == 0) ? WNOHANG | WUNTRACED : WUNTRACED;

#ifdef DEBUG
  fprints(2,"In waitfor\n");
#endif

  if (pid<0) { pid= -pid; subshell= TRUE; }
  while (1)
  {
    wpid = waitpid(-1, &status, waitflags);

#ifdef DEBUG
    fprints(2,"waitxx() returned\n");
#endif
    if (wpid == -1 || wpid == 0)
      break;

    if (subshell && WIFSTOPPED(status))	/* We can't return yet */
	 {
	   kill(wpid, SIGCONT);		/* Wake the child up first - waah! */
	   continue;
	 }
#ifndef NO_JOB
    thisjob = findjob(wpid);
    if (thisjob == NULL)
      continue;
    thisjob->status = status;
    thisjob->changed = TRUE;
#endif
    if (pid == wpid)
      return;
  }
}

/* Stopjob is only called when we received a SIGTSTP. If we are wait3()ing
 * a pid, we send a SIGSTOP to that pid. This should then cause wait3()
 * to exit, and thus waitfor() will return.
 */
SIGTYPE
stopjob(a)
 int a;
{
  signal(SIGTSTP, stopjob);
#ifdef DEBUG
  fprints(2,"In stopjobs\n");
#endif
  if (currentjob)
  {
#ifdef DEBUG
    fprints(2,"About to stop pid %d\n", currentjob->pid);
#endif
    kill(currentjob->pid, SIGSTOP);
  }
}


static void
bgstuff(pid)
  int pid;
{
  struct job *ptr;

  setpgid(pid, pid);
#ifdef DEBUG
  fprints(2,"About to SIGCONT %d\n", pid);
#endif
  kill(pid, SIGCONT);
#ifndef NO_JOB
  ptr = findjob(pid);
  if (ptr) { ptr->STATUS = RUNBG; currentjob= ptr; }
#endif
}


static int
fgstuff(pid)
  int pid;
{

/* Under UCBJOB and POSIXJOB, we don't try & bring the pid into our
 * pgrp, that doesn't work. Instead, we move the terminal
 * over to that pgrp, and move ourselves to that pgrp as well.
 */
    if (tcsetpgrp(0, pid) == -1)	/* Move the terminal to that pgrp */
    {
      perror("fg tcsetpgrp");
      return (1);
    }
    if (setpgid(0, pid) == -1)	/* Set shell's process group to the pid's */
    {
      perror("fg setpgid");
      return (1);
    }
 return(0);
}
