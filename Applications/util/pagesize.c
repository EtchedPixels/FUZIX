#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  printf("%d\n", sysconf(_SC_PAGESIZE));
  return 0;
}