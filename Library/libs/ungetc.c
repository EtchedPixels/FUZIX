#include <stdio.h>
#include <unistd.h>

int ungetc(int c, FILE *fp)
{
   if (fp->mode & __MODE_WRITING)
      fflush(fp);

   /* Can't read or there's been an error then return EOF */
   if ((fp->mode & (__MODE_READ | __MODE_ERR)) != __MODE_READ)
      return EOF;

   /* Can't do fast fseeks */
   fp->mode |= __MODE_UNGOT;

   if( fp->bufpos > fp->bufstart )
      return *--fp->bufpos = (unsigned char) c;
   else if( fp->bufread == fp->bufstart )
      return *fp->bufread++ = (unsigned char) c;
   else
      return EOF;
}
