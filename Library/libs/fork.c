#include <stdio.h>
#include <unistd.h>

/*
 *	Wrap the kernel _fork() call
 */
pid_t fork(void)
{
  return _fork(0, NULL);
}
