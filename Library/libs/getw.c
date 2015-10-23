#include <stdio.h>

int getw(FILE *f)
{
  int r;
  if (fread(&r, sizeof(r), 1, f) != 1)
    return EOF;
  return r;
}
