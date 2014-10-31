#include <stdio.h>
#include <unistd.h>

int fsetpos(FILE *fp, fpos_t *pos)
{
   return fseek(fp, *pos, SEEK_SET);
}
