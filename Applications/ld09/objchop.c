
#include <stdio.h>
#ifdef __STDC__
#include <unistd.h>
#include <stdlib.h>
#endif
#include "x86_aout.h"

#ifndef __OUT_OK

main()
{
   fprintf(stderr, "Compile error: struct exec invalid\n");
   exit(1);
}

#else

FILE * ifd;
struct exec header;

main(argc, argv)
int argc;
char ** argv;
{
   FILE * ofd;
   if( argc != 5 ) fatal("Usage: objchop a.out text.bin data.bin sizes.asm");

   ifd = fopen(argv[1], "r");
   if( ifd == 0 ) fatal("Cannot open input file");

   if( fread(&header, A_MINHDR, 1, ifd) != 1 )
      fatal("Incomplete executable header");

   if( BADMAG(header) )
      fatal("Input file has bad magic number");

   if( fseek(ifd, A_TEXTPOS(header), 0) < 0 )
      fatal("Cannot seek to start of text");

   write_file(argv[2], header.a_text);

   if( fseek(ifd, A_DATAPOS(header), 0) < 0 )
      fatal("Cannot seek to start of data");

   write_file(argv[3], header.a_data);

   ofd = fopen(argv[4], "w");
   if( ofd == 0 ) fatal("Cannot open output file");

   fprintf(ofd, "TEXT_SIZE=%ld\nDATA_SIZE=%ld\nBSS_SIZE=%ld\nALLOC_SIZE=%ld\n",
           header.a_text, header.a_data, header.a_bss, header.a_total);

   fclose(ofd);

   exit(0);
}

write_file(fname, bsize)
char * fname;
long bsize;
{
   char buffer[1024];
   int ssize;
   FILE * ofd;

   ofd = fopen(fname, "w");
   if( ofd == 0 ) fatal("Cannot open output file");

   while(bsize>0)
   {
      if( bsize > sizeof(buffer) ) ssize = sizeof(buffer);
      else ssize = bsize;

      if( (ssize=fread(buffer, 1, ssize, ifd)) <= 0 )
         fatal("Error reading segment from executable");
      if( fwrite(buffer, 1, ssize, ofd) != ssize )
         fatal("Error writing output file");
      bsize -= ssize;
   }
   fclose(ofd);
}

fatal(str)
char * str;
{
   fprintf(stderr, "objchop: %s\n", str);
   exit(2);
}

#endif
