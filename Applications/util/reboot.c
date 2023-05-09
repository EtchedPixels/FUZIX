/*
 *	A simple reboot and halt
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static int telinit(unsigned char c)
{
  int fd = open("/var/run/initctl", O_WRONLY|O_TRUNC|O_CREAT, 0600);
  if (fd < 0 || write(fd, &c, 1) != 1 || close(fd) == -1 ||
      kill(1, SIGUSR1) < 0)
        return 0;
  return 1;
}

int main(int argc, char *argv[])
{
  int pv = 0;
  char *p = strchr(argv[0], '/');
  if (p)
    p++;
  else
    p = argv[0];
  if (argc == 2 && strcmp(argv[1], "-f") == 0) {
    argc--;
    pv = AD_NOSYNC;
  }
  if (argc != 1) {
    write(2, argv[0], strlen(argv[0]));
    write(2, ": unexpected argument.\n", 23);
    exit(1);
  }
  /* shutdown does a polite shutdown */
  if (strcmp(p, "shutdown") == 0) {
    if (telinit(6))
      exit(0);
    write(2, argv[0], strlen(argv[0]));
    write(2, ": unable to talk to init.\n", 24);
  }
  if (strcmp(p, "halt") == 0)
    uadmin(A_SHUTDOWN, pv, 0);
  else if (strcmp(p, "suspend") == 0)
    uadmin(A_SUSPEND, pv, 0);
  else {
    if (pv == 0 && telinit(5))
      exit(0);
    uadmin(A_REBOOT, pv, 0);
  }
  /* If we get here there was an error! */
  perror(argv[0]);
  return 1;
}
