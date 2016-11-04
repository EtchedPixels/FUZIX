/*
 *	A simple reboot and halt
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  char *p = strchr(argv[0], '/');
  if (p)
    p++;
  else
    p = argv[0];
  if (strcmp(p, "halt") == 0)
    uadmin(A_SHUTDOWN,0,0);
  else
    uadmin(A_REBOOT,0,0);
  /* If we get here there was an error! */
  perror(argv[0]);
  return 1;
}
