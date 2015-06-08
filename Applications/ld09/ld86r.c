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

#define ARMAG "!<arch>\n"
#define SARMAG 8
#define ARFMAG "`\n"

struct ar_hdr {
	char	ar_name[16],
		ar_date[12],
		ar_uid[6],
		ar_gid[6],
		ar_mode[8],
		ar_size[10],
		ar_fmag[2];
} arbuf;

void
fatal(char * str) { fprintf(stderr, "%s\n", str); exit(2); }

void
main(int argc, char ** argv)
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
      fatal("Err, what's the output gonna be called?");

   if( (fd =fopen(argv[libarg], "wb")) == 0 ) fatal("Cannot open archive");
   if( fwrite(ARMAG, 1, SARMAG, fd) != SARMAG)  fatal("Cannot write magic");

   for(ar=1; ar<argc; ar++) if( ar != libarg && argv[ar][0] != '-' )
   {
      char * ptr;
      if( stat(argv[ar], &st) < 0 ) fatal("Cannot stat object");
      if((ptr=strchr(argv[ar], '/'))) ptr++; else ptr=argv[ar];
      memset(&arbuf, ' ', sizeof(arbuf));
      strcpy(buf, ptr); strcat(buf, "/                 ");
      strncpy(arbuf.ar_name, buf, sizeof(arbuf.ar_name));
      
      sprintf(arbuf.ar_date, "%-12ld", (long)st.st_mtime);
      sprintf(arbuf.ar_uid, "%-6d",    (int)(st.st_uid%1000000L));
      sprintf(arbuf.ar_gid, "%-6d",    (int)(st.st_gid%1000000L));
      sprintf(arbuf.ar_mode, "%-8lo",  (long)st.st_mode);
      sprintf(arbuf.ar_size, "%-10ld", (long)st.st_size);
      memcpy(arbuf.ar_fmag, ARFMAG, sizeof(arbuf.ar_fmag));

      if( fwrite(&arbuf, 1, sizeof(arbuf), fd) != sizeof(arbuf) )
         fatal("Cannot write header");

      ptr = malloc(st.st_size+2);
      if( ptr == 0 ) fatal("Out of memory");
      ptr[st.st_size] = ' ';
      if( (ifd = fopen(argv[ar], "rb")) == 0 ) fatal("Cannot open input");
      if( fread(ptr, 1, st.st_size, ifd) != st.st_size )
         fatal("Cannot read input file");
      fclose(ifd);

      if( st.st_size&1 ) st.st_size++;
      if( fwrite(ptr, 1, st.st_size, fd) != st.st_size )
         fatal("Cannot write output file");
   }
   fclose(fd);
   exit(0);
}
