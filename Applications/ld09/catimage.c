/*
 * This program concatenates memory images the executables specified
 * on it's command line.
 *
 * The 'boot' image must have a symbol table any symbols that match
 * the below patterns have their values patched.
 *
 * int __seg0_text;	- Always zero
 * int __seg0_data;	- Segment offset of data of boot executable
 *
 * int __seg1_text;	- Segment offset of text of first executable
 * int __seg1_data;	- Segment offset of data of first executable
 * int __seg2_text;	- Segment offset of text of second executable
 * int __seg2_data;	- Segment offset of data of second executable
 *
 * int __seg9_text;	- Segment offset of text of executable nine
 * int __seg9_data;	- Segment offset of data of executable nine
 *
 * Any segment that's not an exact multiple of 16 bytes long is rounded up.
 *
 */

#include <stdio.h>
#ifdef __STDC__
#include <unistd.h>
#include <stdlib.h>
#endif
#include "x86_aout.h"

#ifndef __OUT_OK
#error "Compile error: struct exec invalid (long not 32 bit ?)"
#endif

unsigned long text_offt[10];	/* Locations to patch (0=don't) */
unsigned long data_offt[10];

char * input_file = "";
FILE * ofd;
FILE * ifd = 0;
struct exec header;

main(argc, argv)
int argc;
char ** argv;
{
   long image_offset, text_off;
   int  image_id;

   if( argc < 3 || argc > 11 )
      fatal("Usage: catimage mem.bin boot.out [a1.out] ... [a9.out]");

   open_obj(argv[2]);

   ofd = fopen(argv[1], "w");
   if( ofd == 0 ) fatal("Cannot open output file");

   read_symtable();

   image_offset = 0;

   for(image_id=0; image_id < argc-2; image_id++)
   {
      open_obj(argv[image_id+2]);

      printf("File %-14s seg=0x%04lx text=0x%04lx data=0x%04lx\n",
              input_file, (image_offset>>4),
	      (header.a_text>>4), (header.a_total>>4));

      text_off = image_offset;
      if( header.a_flags & A_SEP )
      {
         copy_segment(image_offset, A_TEXTPOS(header), header.a_text);
         image_offset += header.a_text;
         image_offset = ((image_offset+15L)&-16L);
   
         copy_segment(image_offset, A_DATAPOS(header), header.a_data);
      }
      else
      {
         copy_segment(image_offset, A_TEXTPOS(header),
	              header.a_text+header.a_data);
      }

      patch_bin(text_offt[image_id], (unsigned)(text_off>>4));
      patch_bin(data_offt[image_id], (unsigned)(image_offset>>4));

      image_offset += header.a_total;
      image_offset = ((image_offset+15L)&-16L);
   }

   if( fseek(ofd, image_offset-1, 0) < 0 )
      fatal("Cannot seek to end of output");

   fputc('\0', ofd);
   fclose(ofd);

   printf("Output file size %ldKb\n", ((image_offset+0x3FF)>>10));

   if( ifd ) fclose(ifd);
   exit(0);
}

open_obj(fname)
char * fname;
{
   input_file = fname;

   if( ifd ) fclose(ifd);

   ifd = fopen(fname, "r");
   if( ifd == 0 ) fatal("Cannot open input file");

   if( fread(&header, A_MINHDR, 1, ifd) != 1 )
      fatal("Incomplete executable header");

   if( BADMAG(header) )
      fatal("Input file has bad magic number");
}

copy_segment(out_offset, in_offset, length)
long out_offset, in_offset, length;
{
   char buffer[1024];
   int ssize;
   long bsize = length;

   if( fseek(ifd, in_offset, 0) < 0 )
      fatal("Cannot seek to start of input segment");

   if( fseek(ofd, out_offset, 0) < 0 )
      fatal("Cannot seek to start of output segment");

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
}

patch_bin(file_off, value)
long file_off;
int value;
{
   char wbuf[4];
   if( file_off > 0 )
   {
      printf("Patch at offset 0x%05lx = %04x\n", file_off, value);

      wbuf[0] = value;
      wbuf[0] = (value>>8);

      if( fseek(ofd, file_off, 0) < 0 )
	 fatal("Cannot seek to patch binary");

      if( fwrite(wbuf, 1, 2, ofd) != 2 )
	 fatal("Error patching output file");
   }
}

read_symtable()
{
   struct nlist item;
   int nitems;
   long base_off = 0;

   if( header.a_syms == 0 )
      fatal("Input file has been stripped!");

   if( fseek(ifd, A_SYMPOS(header), 0) < 0 )
      fatal("Cannot seek to start of symbols");

   nitems = header.a_syms;

   /* Foreach symbol */
   while( fread(&item, sizeof(struct nlist), 1, ifd) == 1 )
   {
      if( nitems-- <= 0 ) break;

      /* Match the name */
      if( memcmp(item.n_name, "__seg", 5) != 0 || item.n_name[6] != '_' )
         continue;

      /* Externals only */
      if( (item.n_sclass & N_CLASS) != C_EXT )
         continue;

      /* Data seg only */
      if( (item.n_sclass & N_SECT) != N_DATA &&
          (item.n_sclass & N_SECT) != N_BSS  &&
          (item.n_sclass & N_SECT) != N_TEXT )
	 continue;

      if( item.n_name[5] < '0' || item.n_name[5] > '9' )
         continue;

      if( (header.a_flags & A_SEP) && (item.n_sclass & N_SECT) != N_TEXT )
         base_off = header.a_text;
      else
         base_off = 0;

      switch( item.n_name[7] )
      {
      case 'd': data_offt[item.n_name[5]-'0'] = base_off+item.n_value; break;
      case 't': text_offt[item.n_name[5]-'0'] = base_off+item.n_value; break;
      }

#ifdef DEBUG
      printf("%-8.8s ", item.n_name);
      printf("%08lx ", item.n_value);
      switch(item.n_sclass & N_CLASS)
      {
      case C_NULL: printf("C_NULL "); break;
      case C_EXT:  printf("C_EXT  "); break;
      case C_STAT: printf("C_STAT "); break;
      default:     printf("%-6d ", (item.n_sclass & N_CLASS)); break;
      }
      switch(item.n_sclass & N_SECT)
      {
      case N_UNDF: printf("N_UNDF "); break;
      case N_ABS : printf("N_ABS  "); break;
      case N_TEXT: printf("N_TEXT "); break;
      case N_DATA: printf("N_DATA "); break;
      case N_BSS : printf("N_BSS  "); break;
      case N_COMM: printf("N_COMM "); break;
      }
      printf("\n");
#endif
   }
}

fatal(str)
char * str;
{
   fprintf(stderr, "catimage:%s: %s\n", input_file, str);
   exit(2);
}

