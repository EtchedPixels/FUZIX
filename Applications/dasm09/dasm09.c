/***************************************************************************
 * dasm09 -- Portable M6809/H6309/OS9 Disassembler                         *
 * Copyright (C) 2000  Arto Salmi                                          *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the Free Software             *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *
 ***************************************************************************/

/* NOTE! os9 call table is not tested. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TYPES
#define TYPES
typedef unsigned char  byte;
typedef unsigned short word;
#endif

#ifndef NULL
#define NULL 0
#endif

FILE *infile = NULL;
uint16_t offset = 0;

#define OPCODE(address)  getb(address)
#define ARGBYTE(address) getb(address)
#define ARGWORD(address) getword(address)


static void readerr(void)
{
    perror("infile");
    exit(1);
}

static uint8_t getb(uint16_t addr)
{
    if (fseek(infile, addr-offset, SEEK_SET)<0)
	readerr();
    return getc(infile);
}

static uint16_t getword(uint16_t addr)
{
    if (fseek(infile, addr-offset, SEEK_SET)<0)
	readerr();
    return (getc(infile)<<8) | getc(infile);
}

#include "dasm09.h"

static char *Options[]=
{
 "begin","end","offset","out","noaddr","nohex","x","os9", NULL
};

void usage(void)
{
   printf("Usage: dasm09 [options] <filename>\n"
          "Available options are:\n"
          " -begin  - start disassembly address [offset]\n"
          " -end    - end disassembly address  [auto]\n"
          " -offset - address to load program [0]\n"
          " -out    - output file [stdout]\n"
          " -noaddr - no address dump\n"
          " -nohex  - no hex dump\n"
          " -x      - use 6309 opcodes\n"
          " -os9    - patch swi2 (os9 call)\n"
          "All values should be entered in hexadecimal\n");

   exit(1);
}


int main(int argc, char *argv[])
{
   unsigned begin=0,end=0,pc,add;
   char *fname=NULL,*outname=NULL;
   int showhex=TRUE,showaddr=TRUE;
   int i,j,n;
   char buf[30];
   int off;
   FILE *out=stdout;

   printf("dasm09: M6809/H6309/OS9 disassembler V0.1 © 2000 Arto Salmi\n");

   for (i=1,n=0;i<argc;++i)
   {
      if (argv[i][0]!='-')
      {
         switch (++n)
         {
            case 1:  fname=argv[i];
                     break;
           default:  usage();
         }
      }
      else
      {
         for (j=0;Options[j];++j)
         if (!strcmp(argv[i]+1,Options[j])) break;
         switch (j)
         {
         case 0:  ++i; if (i>argc) usage();
                  begin=strtoul(argv[i],NULL,16);
                  break;
         case 1:  ++i; if (i>argc) usage();
                  end=strtoul(argv[i],NULL,16);
                  break;
         case 2:  ++i; if (i>argc) usage();
                  offset=strtoul(argv[i],NULL,16);
                  break;
         case 3:  ++i; if (i>argc) usage();
                  outname=argv[i];
                  break;
         case 4:  showaddr=FALSE;break;
         case 5:  showhex=FALSE;break;

         case 6:  codes             = h6309_codes;
                  codes10           = h6309_codes10;
                  codes11           = h6309_codes11;
                  exg_tfr           = h6309_exg_tfr;
                  allow_6309_codes  = TRUE;
                  break;

           case 7: os9_patch = TRUE; break;

         default: usage();
         }
      }
   }

   infile=fopen(fname,"rb");
   if(!infile) usage();
   if(!end)
   {
      fseek(infile,0,SEEK_END);
      off=ftell(infile);
      end=(offset+off)-1;
      rewind(infile);
   }

   if(!begin) if(offset) begin=offset;

   if(outname)
   {
      out=fopen(outname,"w");
      if(!out) printf("can't open %s \n",outname);
   }

   begin&=0xFFFF;
   end&=0xFFFF;
   pc=begin;

   fprintf(out,"; org $%04X \n",pc);

   do
   {
     if(showaddr) fprintf(out,"%04X: ",pc);
     add=Dasm(buf,pc);

     if(showhex)
     {
       for(i=0;i<5;i++)
       {
	 if(add) {add--;fprintf(out,"%02X ",getb(pc++));}
         else fprintf(out,"   ");
       }
     } else pc+=add;

     if((!showaddr)&&(!showhex)) fprintf(out,"\t");

     fprintf(out,"%s \n",buf);
   } while( pc <= end);

   printf("Done\n");

   exit:
   if(infile)       fclose(infile);
   if(outname) if(out)    fclose(out);

   return(0);
}
