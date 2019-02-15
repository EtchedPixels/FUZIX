#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  const char *p = _ultoa(sysconf(_SC_PAGESIZE));
  write(1, p, strlen(p));
  write(1, "\n", 1);
  return 0;
}