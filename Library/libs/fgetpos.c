#include <stdio.h>
#include <unistd.h>

int fgetpos(FILE *fp, fpos_t *pos)
{
   long l;

   if (fflush(fp) == EOF)
      return EOF;

   l = lseek(fp->fd, 0L, SEEK_CUR);
   if (l < 0)
      return l;
   *pos = l;
   return 0;
}
