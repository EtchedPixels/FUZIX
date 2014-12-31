/* lkaomf51.c - Create an absolute object memory format 51 file

   Copyright (C) 2002 Jesus Calvino-Fraga, jesusc at ieee dot org

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "aslink.h"

#define EQ(A,B) !strcmp((A),(B))
#define MEMSIZE 0x1000000
#ifndef DODUMP
#define DODUMP 0
#endif

typedef struct
{
    char PathName[PATH_MAX];
    char ModuleName[PATH_MAX];
} _infn;

int numin=0;
_infn * infn=NULL;

typedef struct
{
    char name[0x100];
    int FileNameNumber;
    int Procedure;//If the symbol belongs to a function
    int Static; //If the symbol is only public on its file
    int Address;
    int UsageType;
} _symbol;

int numsym=0;
_symbol * symbol=NULL;

typedef struct
{
    char name[0x100];
    int FileNameNumber;
    int BeginAdd;
    int EndAdd;
    int RegBank;
} _procedure;

int numproc=0;
_procedure * procedure=NULL;

typedef struct
{
    int Number;
    int Address;
    int Procedure;
    int FileNameNumber;
} _linenum;

int numlinenum=0;
_linenum * linenum=NULL;
#if 0
typedef struct
{
    char * name;
    int usage;
}
_UsageType;

_UsageType UsageType[]=
{
    {"CSEG",        0},
    {"GSINIT",      0},
    {"GSINIT0",     0},
    {"GSINIT1",     0},
    {"GSINIT2",     0},
    {"GSINIT3",     0},
    {"GSINIT4",     0},
    {"GSINIT5",     0},
    {"GSFINAL",     0},
    {"HOME",        0},
    {"XINIT",       0},
    {"XSEG",        1},
    {"XISEG",       1},
    {"REG_BANK_0",  2},
    {"REG_BANK_1",  2},
    {"REG_BANK_2",  2},
    {"REG_BANK_3",  2},
    {"DSEG",        2},
    {"OSEG",        2},
    {"SSEG",        2},
    {"ISEG",        3},
    {"BSEG",        4},
    {"",            5} /*A typeless number?*/
};
#endif
char * UsageTypeName[]={"CODE", "XDATA", "DATA", "IDATA", "BIT", "NUMBER"};
int AddNumber;
short * ihxBuff=NULL;
FILE * aomf51out;
int GlobalChkSum=0;
int HexSize, HexBegin=0x1000000;


int GetName(char * filepath, char * name)
{
  int j, k;
  for(j=strlen(filepath); j>0; j--)
      if( (filepath[j-1]=='/')||(filepath[j-1]=='\\') ) break;
  for(k=0; (filepath[j]!=0)&&(filepath[j]!='.'); j++, k++)
      name[k]=filepath[j];
  name[k]=0;
  return j;
}

void SaveLinkedFilePath(char * filepath)
{
  int j;

  if((yflag) && (!rflag))
    {
      int ext;

      infn = realloc(infn, sizeof(_infn)*(numin+1));

      /*Get the module name=filename, no drive, no dir, no ext*/
      ext = GetName(filepath, infn[numin].ModuleName);
      //printf("%s, %s\n", infn[numin].PathName, infn[numin].ModuleName);

      strcpy(infn[numin].PathName, filepath);

      /*If there is an extension remove it*/
      if (infn[numin].PathName[ext] == '.')
        infn[numin].PathName[ext] = '\0';

      /*Check if this filename is already in*/
      for (j=0; j<numin; j++)
        {
          if (EQ(infn[numin].PathName, infn[j].PathName))
            break;
        }
      if (j==numin)
        numin++;
    }
}

void FreeAll(void)
{
  if(infn!=NULL)
    {
      free(infn);
      numin=0;
      infn=NULL;
    }

  if(symbol!=NULL)
    {
      free(symbol);
      numsym=0;
      symbol=NULL;
    }

  if(procedure!=NULL)
    {
      free(procedure);
      numproc=0;
      procedure=NULL;
    }

  if(linenum!=NULL)
    {
      free(linenum);
      numlinenum=0;
      linenum=NULL;
    }

  if(ihxBuff!=NULL)
    {
      free(ihxBuff);
      ihxBuff=NULL;
    }
}

void OutputByte(unsigned char value)
{
  GlobalChkSum+=value;
  fwrite( &value, 1, 1, aomf51out );
}

void OutputWord(int value)
{
  OutputByte((unsigned char)(value%0x100));
  OutputByte((unsigned char)(value/0x100));
}

void OutputName(char * name)
{
  int k;
  OutputByte((unsigned char)strlen(name));
  for(k=0; name[k]!=0; k++)
      OutputByte((unsigned char)toupper(name[k]));
}

void OutputChkSum(void)
{
  OutputByte((unsigned char)(0x100-(GlobalChkSum%0x100)));
  GlobalChkSum=0;
}

void DumpForDebug (void)
{
  char DumpFileName[PATH_MAX];
  FILE * DumpFile;
  int j, k;

  strcpy(DumpFileName, infn[0].PathName);
  strcat(DumpFileName, ".d51");

  DumpFile=fopen(DumpFileName, "wb");
  if(DumpFile==NULL)
    {
      printf("Couldn't create file %s\n", DumpFileName);
      return;
    }

  fprintf(DumpFile,"SYMBOLS:\n");

  for(j=0; j<numsym; j++)
    {
      k=symbol[j].UsageType&0xf;
      fprintf(DumpFile, "%s, %s, %s, 0x%04x, %s\n",
              symbol[j].name,
              infn[symbol[j].FileNameNumber].PathName,
              (symbol[j].Procedure>=0)?procedure[symbol[j].Procedure].name:"GLOBAL",
              symbol[j].Address,
              k<6?UsageTypeName[k]:"???");
    }

  fprintf(DumpFile,"\nPROCEDURES:\n");
  for(j=0; j<numproc; j++)
    {
      fprintf(DumpFile, "%s, %s, 0x%04x-0x%04x, %c\n",
              procedure[j].name,
              infn[procedure[j].FileNameNumber].PathName,
              procedure[j].BeginAdd,
              procedure[j].EndAdd,
              procedure[j].RegBank + '0');
    }

  fprintf(DumpFile,"\nLINE NUMBERS:\n");
  for(j=0; j<numlinenum; j++)
    {
      fprintf(DumpFile, "%d:0x%04x, %s -> %s\n",
              linenum[j].Number,
              linenum[j].Address,
              infn[linenum[j].FileNameNumber].PathName,
              (linenum[j].Procedure>=0)?procedure[linenum[j].Procedure].name:"I don't know");
    }

  fclose(DumpFile);
}

void ParseRegisters(_symbol * symbol, const char * Registers)
{
  char c;
  int i;
  char regs[0x100][4];
  int address[4];
  int nRegs = sscanf(Registers, "[ %[^,] %c %[^,] %c %[^,] %c %[^,] ]",
                     regs[0], &c, regs[1], &c,  regs[2], &c, regs[3]);
  nRegs = (nRegs + 1) / 2;
  for (i=0; i<nRegs; i++)
    {
      if ((regs[i][0] == 'r') && isdigit(regs[i][1]))
        {
          address[i] = regs[i][1] - '0';
        }
    }
  for (i=1; i<nRegs; i++)
    {
      if (address[i] != address[i-1] + 1)
        {// we need strict ascending registers
          return;
        }
    }
  if (0 <= symbol->Procedure && symbol->Procedure < numproc)
    symbol->Address = address[0] + procedure[symbol->Procedure].RegBank * 8;
}

void OutputAOEMF51(void)
{
  int i, j, k, recsize;
  char MHRname[0x100], Mname[0x100];
  char aomf51FileName[PATH_MAX];

  strcpy(aomf51FileName, infn[0].PathName);
  strcat(aomf51FileName, ".omf");

  aomf51out=fopen(aomf51FileName, "wb");
  if(aomf51out==NULL)
    {
      printf("Couldn't create file %s\n", aomf51FileName);
      return;
    }

  GetName(infn[0].PathName, MHRname);
  GlobalChkSum=0;

  /*Module header record*/
  OutputByte(0x02);/*REC TYPE*/
  OutputWord((strlen(MHRname)+1)+3);/*Record Length*/
  OutputName(MHRname);/*Module Name*/
  OutputByte(0xff);/*TRN ID: RL51?*/
  OutputByte(0x00);
  OutputChkSum();

  for(j=0; j<numin; j++)
    {
      GetName(infn[j].PathName, Mname);

      /*Scope Definition record: begin module block*/
      OutputByte(0x10);/*REC TYPE*/
      OutputWord((strlen(Mname)+1)+2);/*Record Length*/
      OutputByte(0x00);/*BLK TYP: module block*/
      OutputName(Mname);/*Module Name*/
      OutputChkSum();

      /*Public symbols defined in this module*/
      recsize=2;
      for(k=0; k<numsym; k++)/*Compute the record length*/
        {
          if ( (symbol[k].FileNameNumber==j) &&
               (symbol[k].Address!=-1) &&
               (symbol[k].Procedure==-1) &&
               (symbol[k].Static==-1) )
            {
              recsize+=((strlen(symbol[k].name)+1)+5);
            }
        }

      if(recsize>2) /*If there are any symbols*/
        {
          OutputByte(0x12);       /*REC TYPE*/
          OutputWord(recsize);    /*Record Length*/
          OutputByte(0x01);       /*DEF TYPE: Public symbols*/
          for(k=0; k<numsym; k++)
            {
              if ( (symbol[k].FileNameNumber==j) &&
                   (symbol[k].Address!=-1) &&
                   (symbol[k].Procedure==-1) &&
                   (symbol[k].Static==-1) )
                {
                  OutputByte(0x00);/*SEG ID*/
                  OutputByte((unsigned char)symbol[k].UsageType);/*SYM INFO*/
                  OutputWord(symbol[k].Address);/*Offset*/
                  OutputByte(0x00);
                  OutputName(symbol[k].name);/*Symbol name*/
                }
            }
          OutputChkSum();
        }

      /*Local symbols defined in this module*/
      recsize=2;
      for(k=0; k<numsym; k++)/*Compute the record length*/
        {
          if ( (symbol[k].FileNameNumber==j) &&
               (symbol[k].Address!=-1) &&
               (symbol[k].Procedure==-1) &&
               (symbol[k].Static==j) )
            {
              recsize+=((strlen(symbol[k].name)+1)+5);
            }
        }

      if(recsize>2) /*If there are any symbols*/
        {
          OutputByte(0x12);       /*REC TYPE*/
          OutputWord(recsize);    /*Record Length*/
          OutputByte(0x00);       /*DEF TYPE: Local symbols*/
          for(k=0; k<numsym; k++)
            {
              if ( (symbol[k].FileNameNumber==j) &&
                   (symbol[k].Address!=-1) &&
                   (symbol[k].Procedure==-1) &&
                   (symbol[k].Static==j) )
                {
                  OutputByte(0x00);/*SEG ID*/
                  OutputByte((unsigned char)symbol[k].UsageType);/*SYM INFO*/
                  OutputWord(symbol[k].Address);/*Offset*/
                  OutputByte(0x00);
                  OutputName(symbol[k].name);/*Symbol name*/
                }
            }
          OutputChkSum();
        }

      /*Output the procedures of this module*/

      for(k=0; k<numproc; k++)
        {
          if(procedure[k].FileNameNumber==j)
            {
              /*Scope Definition record: begin PROCEDURE block*/
              OutputByte(0x10);/*REC TYPE*/
              OutputWord((strlen(procedure[k].name)+1)+2);/*Record Length*/
              OutputByte(0x02);/*BLK TYP: PROCEDURE block*/
              OutputName(procedure[k].name);/*Module Name*/
              OutputChkSum();

              /*Content Record*/
              OutputByte(0x06);/*REC TYPE*/
              if (procedure[k].EndAdd==-1)
                  procedure[k].EndAdd=HexSize;
              recsize=procedure[k].EndAdd-procedure[k].BeginAdd+1+4;
              OutputWord(recsize);/*Record Length*/
              OutputByte(0x00);/*SEG ID*/
              OutputWord(procedure[k].BeginAdd); /*Offset*/
              for (i=procedure[k].BeginAdd; i<=procedure[k].EndAdd; i++)
                {
                  OutputByte((unsigned char)ihxBuff[i]);
                  ihxBuff[i] -= 0x200;
                }
              OutputChkSum();

              /*Local Symbols*/

              recsize=2;
              for(i=0; i<numsym; i++)/*Get the record length*/
                {
                  if( (symbol[i].Procedure==k) &&
                      (symbol[i].Address!=-1) )
                    {
                      recsize+=((strlen(symbol[i].name)+1)+5);
                    }
                }

              if(recsize>2) /*If there are any symbols*/
                {
                  OutputByte(0x12);       /*REC TYPE*/
                  OutputWord(recsize);    /*Record Length*/
                  OutputByte(0x00);       /*DEF TYPE: Local symbols*/
                  for(i=0; i<numsym; i++)
                    {
                      if ( (symbol[i].Procedure==k) &&
                           (symbol[i].Address!=-1) )
                        {
                          OutputByte(0x00);/*SEG ID*/
                          OutputByte((unsigned char)symbol[i].UsageType);/*SYM INFO*/
                          OutputWord(symbol[i].Address);/*Offset*/
                          OutputByte(0x00);
                          OutputName(symbol[i].name);/*Symbol name*/
                        }
                    }
                  OutputChkSum();
                }

              /*Line Numbers*/
              recsize=2;
              for(i=0; i<numlinenum; i++)/*Get the record length*/
                  if(linenum[i].Procedure==k)
                      recsize+=5;

              if(recsize>2) /*If there are any line numbers*/
                {
                  OutputByte(0x12);       /*REC TYPE*/
                  OutputWord(recsize);    /*Record Length*/
                  OutputByte(0x03);       /*DEF TYPE: Line numbers*/
                  for(i=0; i<numlinenum; i++)
                    {
                      if ( (linenum[i].Procedure==k) )
                        {
                          OutputByte(0x00);/*SEG ID*/
                          OutputWord(linenum[i].Address);/*Offset*/
                          OutputWord(linenum[i].Number);/*Line Number*/
                        }
                    }
                  OutputChkSum();
                }

              /*Scope Definition record: end PROCEDURE block*/
              OutputByte(0x10);/*REC TYPE*/
              OutputWord((strlen(procedure[k].name)+1)+2);/*Record Length*/
              OutputByte(0x05);/*BLK TYP: PROCEDURE end block*/
              OutputName(procedure[k].name);/*Module Name*/
              OutputChkSum();
            }
        }

      /*Scope Definition record: end module block*/
      OutputByte(0x10);/*REC TYPE*/
      OutputWord((strlen(Mname)+1)+2);/*Record Length*/
      OutputByte(0x03);/*BLK TYP: module end*/
      OutputName(Mname);/*Module Name*/
      OutputChkSum();
    }

  /*Content records for everything that is not in the above procedures*/
  strcpy(Mname, "OTHER_SDCC_STUFF");

  /*Scope Definition record: begin module block*/
  OutputByte(0x10);/*REC TYPE*/
  OutputWord((strlen(Mname)+1)+2);/*Record Length*/
  OutputByte(0x00);/*BLK TYP: module block*/
  OutputName(Mname);/*Module Name*/
  OutputChkSum();

  for (i=HexBegin; i<HexSize; )
    {
      for (k=i; k<HexSize; k++)
        {
          if (ihxBuff[k] < 0)
            break;
        }
      if (k > i)
        {
          /*Content Record*/
          OutputByte(0x06);/*REC TYPE*/
          OutputWord(k-i+4);/*Record Length*/
          OutputByte(0x00);/*SEG ID*/
          OutputWord(i); /*Offset*/
          for ( ; i<k; i++)
            {
              OutputByte((unsigned char)ihxBuff[i]);
              ihxBuff[i] -= 0x200;
            }
          OutputChkSum();
        }
      for ( ; i<HexSize; i++)
        {
          if (ihxBuff[i] >= 0)
            break;
        }
    }

  /*Scope Definition record: end module block*/
  OutputByte(0x10);/*REC TYPE*/
  OutputWord((strlen(Mname)+1)+2);/*Record Length*/
  OutputByte(0x03);/*BLK TYP: module end*/
  OutputName(Mname);/*Module Name*/
  OutputChkSum();

  /*Module end record*/
  OutputByte(0x04);/*REC TYPE*/
  OutputWord((strlen(MHRname)+1)+5);/*Record Length*/
  OutputName(MHRname);/*Module Name*/
  OutputWord(0x00);
  OutputByte(0x0f);/*REG MSK: All the register banks?*/
  OutputByte(0x00);
  OutputChkSum();

  fclose(aomf51out);
}

void CollectInfoFromCDB(void)
{
  int i, j, k, CurrentModule;
  FILE * CDBin;
  char buff[0x1000];
  char SourceName[PATH_MAX];

  //"S:{G|F<filename>|L<filename>.<functionName>}$<name>$<level>$<block>(<type info>),<Address Space>,<on Stack?>,<stack offset>"
  char Sfmt[]="%[^$] %c %[^$] %c %[^$] %c %s";
  char c;
  char module[0x100];
  char scope[0x100];
  char name[0x100];
  char level[0x100];
  char block[0x100];
  char TypeInfo[0x100];
  char AddressSpace;
  int OnStack;
  int StackOffset;
  int IsISR;
  int IntNr;
  int RegBank;
  char Registers[0x100];
  int Address, CLine;

  if(numin==0) return;

  if (yfp != NULL)
    {
      fclose(yfp);
      yfp=NULL;
    }

  /*Build the source filename*/
  strcpy(SourceName, infn[0].PathName);
  strcat(SourceName, ".cdb");
  CDBin=fopen(SourceName, "r");
  if(CDBin==NULL)
    {
      printf("Couldn't open file '%s'\n", SourceName);
      lkexit(1);
    }

  CurrentModule=0; /*Set the active module as the first one*/
  while(!feof(CDBin))
    {
      if(NULL==fgets(buff, sizeof(buff)-1, CDBin))
        {
          if(ferror(CDBin))
            {
              perror("Can't read file");
              lkexit(1);
            }
          else if(!feof(CDBin))
            {
              fprintf(stderr, "Unknown error while reading file '%s'\n", SourceName);
              lkexit(1);
            }
        }

      if(!feof(CDBin)) switch(buff[0])
        {
          /*Example: "M:adq"*/
          case 'M':
            sscanf(&buff[2], "%s", name);
            for(j=0; j<numin; j++)
                if(EQ(infn[j].ModuleName, name)) break;
            if(j<numin) CurrentModule=j;
          break;

          /* Example:
          "S:G$actual$0$0({7}ST__00010000:S),E,0,0"
          "S:Lfile.main$j$1$1({2}SI:S),E,0,0"
          "S:Lfile.main$k$1$1({2}DG,SI:S),R,0,0,[r2,r3]"
          "S:G$DS1306_Reset_SPI$0$0({2}DF,SV:S),C,0,0"
          "S:G$main$0$0({2}DF,SV:S),C,0,0"
          */

          case 'S':
            sscanf(buff, Sfmt,
                   scope, &c,
                   name, &c,
                   level, &c,
                   block);

            /*<block>(<type info>),<Address Space>,<on Stack?>,<stack offset>*/
            sscanf(block, "%[^)] %c %c %c %c %d %c %d %c %s",
                   TypeInfo, &c, &c,
                   &AddressSpace, &c,
                   &OnStack, &c,
                   &StackOffset, &c,
                   Registers);

            i=-1; k=-1;
            switch(scope[2])
              {
                case 'G': /*Global symbol*/
                break;
                case 'F': /*Local symbol to a module*/
                  for(j=0; j<numin; j++)
                    {
                      if (EQ(&scope[3], infn[j].ModuleName))
                        {
                          i = j;
                          break;
                        }
                    }
                break;
                case 'L': /*Local symbol of a procedure*/
                  for(j=0; j<numproc; j++)
                    {
                      size_t mlen = strlen(infn[procedure[j].FileNameNumber].ModuleName);
                      if ((!strncmp (&scope[3], infn[procedure[j].FileNameNumber].ModuleName, mlen)) &&
                          (scope[mlen+3] == '.') &&
                          (EQ(&scope[mlen+4], procedure[j].name)))
                        {
                          k = j; /*Local symbol*/
                          break;
                        }
                    }
                break;
              }

            /*This symbol may have been already defined*/
            for (j=0; j<numsym; j++)
              {
                if (EQ(name, symbol[j].name) &&
                    (symbol[j].Procedure == k) &&
                    (symbol[j].Static == i) ) break;
              }
            if(j==numsym) /*New symbol*/
              {
                symbol=realloc(symbol, sizeof(_symbol)*(numsym+1));
                symbol[numsym].FileNameNumber=CurrentModule;
                strcpy(symbol[numsym].name, name);
                symbol[numsym].Procedure=k;
                symbol[numsym].Static=i;
                symbol[numsym].Address=-1;/*Collected later*/

                switch(AddressSpace)
                  {
                    case 'C': /*Code*/
                    case 'D': /*Code/static segment*/
                    case 'Z': /*Functions and undefined code space*/
                      symbol[numsym].UsageType=0x40;
                    break;

                    case 'F': /*External ram*/
                    case 'A': /*External stack*/
                    case 'P': /*External Pdata*/
                      symbol[numsym].UsageType=0x41;
                    break;

                    case 'E': /*Internal ram (lower 128) bytes*/
                    case 'I': /*SFR space*/
                      symbol[numsym].UsageType=0x42;
                    break;

                    case 'R': /*Register Space*/
                      ParseRegisters(&symbol[numsym], Registers);
                      symbol[numsym].UsageType=0x42;
                    break;

                    case 'B': /*Internal stack*/
                    case 'G': /*Internal ram*/
                      symbol[numsym].UsageType=0x43;
                    break;

                    case 'H': /*Bit addressable*/
                    case 'J': /*SBIT space*/
                      symbol[numsym].UsageType=0x44;
                    break;

                    default:
                      printf("Unknown scope information for: %s, AddressSpace:%c\n", symbol[numsym].name, AddressSpace);
                    break;
                  }
                numsym++;
              }
          break;

          /*Examples:
          F:G$AsciiToHex$0$0({2}DF,SC:U),C,0,0,0,0,0
          F:G$main$0$0({2}DF,SV:S),C,0,0,0,0,0
          F:Fbug1627975$f2$0$0({2}DF,DG,SI:U),C,0,0,0,0,0 */

          case 'F':
            sscanf(buff, "%[^$] %c %[^$] %c %[^$] %c %s",
                   scope, &c,
                   name, &c,
                   level, &c,
                   block);

            /*<block>(<type info>),<Address Space>,<on Stack?>,<stack offset>,<isr?>,<int nr>,<regbank> */
            sscanf(block, "%[^)] %c %c %c %c %d %c %d %c %d %c %d %c %d",
                   TypeInfo, &c, &c,
                   &AddressSpace, &c,
                   &OnStack, &c,
                   &StackOffset, &c,
                   &IsISR, &c,
                   &IntNr, &c,
                   &RegBank);
            /*The same may have been already defined */
            for(j=0; j<numproc; j++)
              {
                if (EQ(name, procedure[j].name) &&
                    (procedure[j].FileNameNumber == CurrentModule))
                  {
                    break;
                  }
              }
            if (j==numproc)
              {
                procedure=realloc(procedure, sizeof(_procedure)*(numproc+1));
                strcpy(procedure[numproc].name, name);
                procedure[numproc].FileNameNumber=CurrentModule;
                procedure[numproc].BeginAdd=-1;/*To be collected latter*/
                procedure[numproc].EndAdd=-1;/*To be collected latter*/
                procedure[numproc].RegBank=RegBank;
                numproc++;
              }

            /*This function name is also a global symbol*/
            for(j=0; j<numsym; j++)/*A global symbol may have been already defined*/
              {
                if (EQ(name, symbol[j].name) &&
                    (symbol[j].Procedure==-1) &&
                    (symbol[j].FileNameNumber == CurrentModule))
                  {
                    break;
                  }
              }
            if (j==numsym)
              {
                symbol=realloc(symbol, sizeof(_symbol)*(numsym+1));
                symbol[numsym].FileNameNumber=CurrentModule;
                strcpy(symbol[numsym].name, name);
                symbol[numsym].UsageType=0x00;/*A procedure name symbol*/
                symbol[numsym].Procedure=-1; /*Global symbol*/
                symbol[numsym].Address=-1;/*Collected later*/
                symbol[numsym].Static= buff[2]=='F' ? CurrentModule : -1; // o_gloom
                numsym++;
              }
          break;

          case 'L':
            switch(buff[2])
              {
                case 'G': /*Example L:G$P0$0$0:80*/
                  sscanf(buff, "%[^$] %c %[^$] %c %[^:] %c %x",
                         scope, &c, name, &c, level, &c, &Address);

                  for(j=0; j<numsym; j++)
                    {
                      if(EQ(symbol[j].name, name))
                        {
                          if( (symbol[j].Address==-1) && (symbol[j].Procedure==-1) )
                            {
                              symbol[j].Address=Address;
                            }

                          /*If the symbol is the name of a procedure, the address is also
                          the begining of such procedure*/
                          if ((symbol[j].UsageType & 0x0f) == 0x00)
                            {
                              for (k=0; k<numproc; k++)
                                {
                                  if (EQ(symbol[j].name, procedure[k].name))
                                    {
                                      if (procedure[k].BeginAdd == -1)
                                          procedure[k].BeginAdd = Address;
                                      break;
                                    }
                                }
                            }

                          break;
                        }
                    }
                break;

                case 'F': /*Example L:Fadq$_str_2$0$0:57A*/
                  sscanf(&buff[3], "%[^$] %c %[^$] %c %[^:] %c %x",
                         scope, &c, name, &c, level, &c, &Address);

                  for (j=0; j<numsym; j++)
                    {
                      if (EQ(symbol[j].name, name) &&
                          EQ(infn[symbol[j].FileNameNumber].ModuleName, scope))
                        {
                          if( (symbol[j].Address == -1) )
                            {
                              symbol[j].Address = Address;
                            }
                          break;
                        }
                    }

                  /*It could be also a static function*/
                  for (j=0; j<numproc; j++)
                    {
                      if (EQ(procedure[j].name, name) &&
                          EQ(infn[procedure[j].FileNameNumber].ModuleName, scope))
                        {
                          if( (procedure[j].BeginAdd == -1) )
                            {
                              procedure[j].BeginAdd = Address;
                            }
                          break;
                        }
                    }

                break;

                case 'L': /*Example L:Lmain$j$1$1:29*/

                  /*
                  L:Lds1306.DS1306_Write$Value$1$1:34
                  L:Lds1306.DS1306_Burst_Read$count$1$1:35
                  L:Lds1306.DS1306_Burst_Read$address$1$1:36
                  L:Lds1306.DS1306_Burst_Write$count$1$1:37
                  L:Lds1306.DS1306_Burst_Write$address$1$1:38
                  */
                  sscanf(&buff[3], "%[^.] %c %[^$] %c %[^$] %c %[^:] %c %x",
                         module, &c, scope, &c, name, &c, level, &c, &Address);

                  for (k=0; k<numproc; k++)
                    {
                      if (EQ(procedure[k].name, scope) &&
                          EQ(infn[procedure[k].FileNameNumber].ModuleName, module))
                        {
                          for (j=0; j<numsym; j++)
                            {
                              if ((symbol[j].FileNameNumber == procedure[k].FileNameNumber) &&
                                  (symbol[j].Procedure == k) &&
                                  (EQ(symbol[j].name, name)))
                                {
                                  if (symbol[j].Address == -1)
                                    symbol[j].Address = Address;
                                  break;
                                }
                            }
                          if (j<numsym)
                            break;
                        }
                    }
                break;

                /*Line Numbers*/
                case 'C': /*Example L:C$adq.c$38$1$1:3E*/  /*L:C$hwinit.c$29$1$1:7AD*/
                  sscanf(&buff[4], "%[^.] %[^$] %c %d %[^:] %c %x",
                         name, level, &c, &CLine, level, &c, &Address);

                  for(j=0; j<numin; j++)
                      if(EQ(infn[j].ModuleName, name)) break;
                  if(j<numin)
                    {
                      /*Check if this line is already defined*/
                      for(k=0; k<numlinenum; k++)
                        {
                          if( (linenum[k].Number==CLine) &&
                              (linenum[k].FileNameNumber==j) )break;
                        }
                      if(k==numlinenum) /*New line number*/
                        {
                          linenum=realloc(linenum, sizeof(_linenum)*(numlinenum+1));
                          linenum[numlinenum].Number=CLine;
                          linenum[numlinenum].FileNameNumber=j;
                          linenum[numlinenum].Procedure=-1;/*To be asigned later*/
                          linenum[numlinenum].Address=Address;
                          numlinenum++;
                        }
                    }
                break;

                case 'A': /*Example L:A$adq$424:40*/
                        /*No use for this one*/
                break;

                /*The end of a procedure*/
                case 'X': /*Example L:XG$AsciiToHex$0$0:88*/
                  sscanf(&buff[3], "%[^$] %c %[^$] %c %[^:] %c %x",
                         scope, &c, name, &c, level, &c, &Address);

                  for(k=0; k<numproc; k++)
                    {
                      if (EQ(procedure[k].name, name) &&
                          (scope[0] == 'G' ||
                           EQ(infn[procedure[k].FileNameNumber].ModuleName, &scope[1])))
                        {
                          if( (procedure[k].EndAdd == -1) )
                            {
                              procedure[k].EndAdd = Address;
                            }
                          break;
                        }
                    }
                break;
              }
          break;

          default:
          break;
        }
    }

  /*Make sure each procedure has an end*/
  for(k=0; k<(numproc-1); k++)
    {
      if (procedure[k].EndAdd==-1) procedure[k].EndAdd=procedure[k+1].BeginAdd-1;
    }
  /*Assign each line number to a procedure*/
  for(j=0; j<numlinenum; j++)
    {
      for(k=0; k<numproc; k++)
        {
          if ( (linenum[j].Address>=procedure[k].BeginAdd) &&
               (linenum[j].Address<=procedure[k].EndAdd) &&
               (linenum[j].FileNameNumber==procedure[k].FileNameNumber) )
            {
              linenum[j].Procedure=k;
            }
        }
    }

  fclose(CDBin);
}

int hex2dec (unsigned char hex_digit)
{
   if (isdigit (hex_digit))
      return hex_digit-'0';
   else
      return toupper (hex_digit)-'A'+10;
}

unsigned char GetByte(char * buffer)
{
    return hex2dec(buffer[0])*0x10+hex2dec(buffer[1]);
}

unsigned short GetWord(char * buffer)
{
  return  hex2dec(buffer[0])*0x1000+
          hex2dec(buffer[1])*0x100+
          hex2dec(buffer[2])*0x10+
          hex2dec(buffer[3]);
}

int ReadHexFile(int * Begin)
{
  char ihxFileName[PATH_MAX];
  char buffer[1024];
  FILE * filein;
  int j;
  unsigned char linesize, recordtype, rchksum, value;
  int address, hi_addr = 0;
  int MaxAddress = 0;
  int chksum;

  /*If the hexfile is already open, close it*/
  if(ofp!=NULL)
    {
      fclose(ofp);
      ofp=NULL;
    }

  strcpy(ihxFileName, infn[0].PathName);
  strcat(ihxFileName, ".ihx");

  if ( (filein=fopen(ihxFileName, "r")) == NULL )
    {
      printf("Error: Can't open file `%s`.\r\n", ihxFileName);
      return 0;
    }

  ihxBuff = calloc(MEMSIZE, sizeof(short));
  if (ihxBuff==NULL)
    {
      printf("Insufficient memory\n");
      fclose(filein);
      return -1;
    }

  for (j=0; j<MEMSIZE; j++) ihxBuff[j]=-1;

  while(1)
    {
      if(fgets(buffer, sizeof(buffer), filein)==NULL)
        {
          printf("Error reading file '%s'\n", ihxFileName);
          break;
        }
      if(buffer[0]==':')
        {
          linesize = GetByte(&buffer[1]);
          address = hi_addr | GetWord(&buffer[3]);
          recordtype = GetByte(&buffer[7]);
          rchksum = GetByte(&buffer[9]+(linesize*2));
          chksum=linesize+(address/0x100)+(address%0x100)+recordtype+rchksum;

          switch (recordtype)
            {
              case 0:
                for (j=0; j<linesize; j++)
                  {
                    value = GetByte(&buffer[9]+(j*2));
                    chksum += value;
                    ihxBuff[address+j] = value;
                  }
                if (MaxAddress < (address+linesize-1))
                  MaxAddress = (address+linesize-1);
                if (address < *Begin)
                  *Begin = address;
                break;

              case 4:
                hi_addr = (GetWord(&buffer[9]) << 16) & 0x00FFFFFF; //upto 24 bit address space
                break;

              default:
                break;
            }

          if ((chksum % 0x100) != 0)
            {
              printf("ERROR: Bad checksum in file %s\n", ihxFileName);
              fclose(filein);
              return -1;
            }

          if (recordtype==1)  /*End of record*/
            break;
        }
    }
  fclose(filein);

  return MaxAddress;
}

void CreateAOMF51(void)
{
  if((yflag) && (!rflag))
    {
      CollectInfoFromCDB();
#if DODUMP
      DumpForDebug();
#endif
      HexSize=ReadHexFile(&HexBegin)+1;
      OutputAOEMF51();
      FreeAll();
    }
}
