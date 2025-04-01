/* Most of the functions that handle signals are kept in this file.
 * Exceptions are some signal() calls done in job.c
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

/* Graceful is set to catch most of the unused signals in Wish. All it
 * does is print out an error message and exit.
 */
#ifdef PROTO
static SIGTYPE graceful(int sig)
#else
static SIGTYPE 
graceful(sig)
  int sig;
#endif
{
  setcooked();
  fprints(2, "Received signal %d\n", sig);
  exit(1);
}


/* Catchsig is called once at Wish startup, and it sets graceful to
 * catch most of the signals unused by Wish.
 */
void 
catchsig()
{
#ifdef DEBUG
  int i;

  for (i = 1; i <= MAXSIG; i++)
  {
    if (i != SIGKILL && i != SIGCONT) /* SIGKILL cannot be caught or ignored */
      signal(i, graceful);
  }
#endif

   signal(SIGINT, SIG_IGN);	/* Sometimes taken out for debugging */
   signal(SIGQUIT, SIG_IGN); 

#if defined(UCBJOB) || defined(POSIXJOB)
  				/* Also catch these for job control */
  signal(SIGTSTP, stopjob);
  signal(SIGCHLD, SIG_IGN);
#endif
}

/* Dflsig sets all the signals to their default handlers, so that exec'd
 * programs will have a standard signal environment.
 */
void 
dflsig()
{
  int i;

#ifdef DEBUG
  fprints(2,"%d signals defined\n",MAXSIG);
#endif
  for (i = 1; i <= MAXSIG; i++) signal(i, SIG_DFL);
}
