#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <grp.h>

static void writesn(const char *p)
{
  write(1, p, strlen(p));
  write(1, "\n", 1);
}

static void writeg(gid_t g)
{
  struct group *gp;
  gp = getgrgid(g);
  if (gp)
    writesn(gp->gr_name);
  else
    writesn(_itoa(g));
}

int main(int argc, char *argv[])
{
  gid_t *g;
  int n = getgroups(0, NULL);
  gid_t eg = getegid();
  int egp = 1;

  /* ENOSYS means a Level 1 system so just do the right thing */
  if (n < 0 && errno != ENOSYS) {
    perror(argv[0]);
    exit(1);
  }
  if (n > 0) {
    g = (gid_t *)sbrk(sizeof(gid_t) * n);
    if (g == (gid_t *)-1) {
      perror(argv[0]);
      exit(1);
    }
    if (getgroups(n, g) < 0) {
      perror(argv[1]);
      exit(1);
    }
    while(n--) {
      /* Current gid is also in groups table */
      if (*g == eg)
        egp = 0;
      writeg(*g++);
    }
  }
  if (egp)
    writeg(eg);
  exit(0);
}  
