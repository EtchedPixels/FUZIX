
#include <stddef.h>
#include <signal.h>
#include <unistd.h>

int system(const char *command)
{
   int wait_val, wait_ret, pid;
   sighandler_t save_quit;
   sighandler_t save_int;

   if( command == 0 ) return 1;

   save_quit = signal(SIGQUIT, SIG_IGN);
   save_int  = signal(SIGINT,  SIG_IGN);

   if( (pid=fork()) < 0 )
   {
      signal(SIGQUIT, save_quit);
      signal(SIGINT,  save_int);
      return -1;
   }
   if( pid == 0 )
   {
      signal(SIGQUIT, SIG_DFL);
      signal(SIGINT,  SIG_DFL);

      execl("/bin/sh", "sh", "-c", command, (char*)0);
      _exit(127);
   }
   /* Signals are not absolutly guarenteed with vfork */
   signal(SIGQUIT, SIG_IGN);
   signal(SIGINT,  SIG_IGN);

   do
   {
      if( (wait_ret = wait(&wait_val)) == -1 )
      {
         wait_val = -1;
	 break;
      }
   }
   while( wait_ret != pid );

   signal(SIGQUIT, save_quit);
   signal(SIGINT,  save_int);
   return wait_val;
}
