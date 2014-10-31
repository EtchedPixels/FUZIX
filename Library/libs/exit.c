/*
 *	Wrapper for clean up and then exit to the kernel
 *	via _exit
 */

#include <stdlib.h>
#include <unistd.h>

extern void __do_exit(int rv);

void exit(int status)
{
  __do_exit(status);
  _exit(status);
}