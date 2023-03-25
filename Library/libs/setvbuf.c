#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
   int rv = 0;
   fflush(fp);
   if( fp->mode & __MODE_FREEBUF ) free(fp->bufstart);
   fp->mode &= ~(__MODE_FREEBUF|__MODE_BUF);
   fp->bufstart = fp->unbuf;
   fp->bufend = fp->unbuf + sizeof(fp->unbuf);
   fp->mode |= _IONBF;

   if( mode == _IOFBF || mode == _IOLBF )
   {
      if( size <= 0  ) size = BUFSIZ;
      if( buf == 0 )
      {
         if( (buf = malloc(size)) != 0 )
	    fp->mode |= __MODE_FREEBUF;
         else rv = EOF;
      }
      if( buf )
      {
         fp->bufstart = (unsigned char *)buf;
         fp->bufend = (unsigned char *)buf+size;
         fp->mode &= ~__MODE_BUF;
         fp->mode |= mode;
      }
   }
   fp->bufpos = fp->bufread = fp->bufwrite = fp->bufstart;
   return rv;
}
