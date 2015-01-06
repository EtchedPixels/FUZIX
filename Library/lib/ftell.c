#include <stdio.h>
#include <unistd.h>

long ftell(FILE *fp)
{
   if (fflush(fp) == EOF)
      return EOF;
   return lseek(fp->fd, 0L, SEEK_CUR);
}
