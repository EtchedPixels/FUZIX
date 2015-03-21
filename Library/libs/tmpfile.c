#include <stdio.h>
#include <stdlib.h>

FILE *tmpfile(void)
{
  int fd = mkstemp("/tmp/tmpfileXXXXXX");
  if (fd == -1)
    return;
  return fdopen(fd, "r+");
}

