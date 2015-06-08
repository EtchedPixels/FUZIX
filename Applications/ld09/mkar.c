
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __STDC__
#include <stdlib.h>
#include <unistd.h>
#else
#include <malloc.h>
#endif

#include "type.h"
#include "ar.h"

static struct ar_hdr arbuf;

#ifdef __STDC__
void
ld86r(int argc, char ** argv)
#else
ld86r(argc, argv)
   int argc; char ** argv;
#endif
{
char buf[128];
   FILE * fd, * ifd;
   struct stat st;
   int ar, libarg=0, need_o = 0, got_o = 0;

   for(ar=1; ar<argc; ar++) if( argv[ar][0] == '-' )
   {
      if( argv[ar][1] == 'r' ) need_o = 1;
      if( argv[ar][1] == 'o' ) { got_o++; libarg = 0; }
   }
   else
   {
      if( libarg == 0 ) libarg = ar;
   }

   if( libarg == 0 || got_o > 1 || need_o > got_o )
      fatalerror("-o option required for -r");

   if( (fd =fopen(argv[libarg], "wb")) == 0 ) fatalerror("Cannot open archive");
   if( fwrite(ARMAG, 1, SARMAG, fd) != SARMAG)  fatalerror("Cannot write magic");

   for(ar=1; ar<argc; ar++) if( ar != libarg && argv[ar][0] != '-' )
   {
      char * ptr;
      if( stat(argv[ar], &st) < 0 ) fatalerror("Cannot stat object");
      if((ptr=strchr(argv[ar], '/'))) ptr++; else ptr=argv[ar];
      memset(&arbuf, ' ', sizeof(arbuf));
      strcpy(buf, ptr); strcat(buf, "/                 ");
      strncpy(arbuf.ar_name, buf, sizeof(arbuf.ar_name));
     
      snprintf(arbuf.ar_date, 12, "%-12ld", (long)st.st_mtime);
      snprintf(arbuf.ar_uid, 6, "%-6d", (int)(st.st_uid%1000000L));
      snprintf(arbuf.ar_gid, 6, "%-6d", (int)(st.st_gid%1000000L));
      snprintf(arbuf.ar_mode, 8, "%-8lo", (long)st.st_mode);
      snprintf(arbuf.ar_size, 10, "%-10ld", (long)st.st_size);
      memcpy(arbuf.ar_fmag, ARFMAG, sizeof(arbuf.ar_fmag));

      if( fwrite(&arbuf, 1, sizeof(arbuf), fd) != sizeof(arbuf) )
         fatalerror("Cannot write header");

      ptr = malloc(st.st_size+2);
      if( ptr == 0 ) fatalerror("Out of memory");
      ptr[st.st_size] = ' ';
      if( (ifd = fopen(argv[ar], "rb")) == 0 ) fatalerror("Cannot open input");
      if( fread(ptr, 1, st.st_size, ifd) != st.st_size )
         fatalerror("Cannot read input file");
      fclose(ifd);

      if( st.st_size&1 ) st.st_size++;
      if( fwrite(ptr, 1, st.st_size, fd) != st.st_size )
         fatalerror("Cannot write output file");
   }
   fclose(fd);
   exit(0);
}
