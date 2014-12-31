/* lkmem.c - Create a memory summary file with extension .mem

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sdld.h"
#include "aslink.h"

int summary(struct area * areap)
{
  if (TARGET_IS_8051 || TARGET_IS_6808) {
    /* only for 8051 and 6808 targets */

    #define EQ(A,B) !strcasecmp((A),(B))
    #define MIN_STACK 16
    #define REPORT_ERROR(A, H) \
    {\
        fprintf(of, "%s%s", (H)?"*** ERROR: ":"", (A)); \
        fprintf(stderr, "%s%s", (H)?"\n?ASlink-Error-":"",(A)); \
        toreturn=1; \
    }

    #define REPORT_WARNING(A, H) \
    { \
        fprintf(of, "%s%s", (H)?"*** WARNING: ":"", (A)); \
        fprintf(stderr, "%s%s",(H)?"\n?ASlink-Warning-":"", (A)); \
    }

    char buff[128];
    int j, toreturn=0;
    unsigned int Total_Last=0, k;

    struct area * xp;
    FILE * of;

    /*Artifacts used for printing*/
    char start[15], end[15], size[15], max[15];
    char format[]="   %-16.16s %-8.8s %-8.8s %-8.8s %-8.8s\n";
    char line[]="---------------------";

    typedef struct
    {
        unsigned long Start;
        unsigned long Size;
        unsigned long Max;
        char Name[NCPS];
        unsigned long flag;
    } _Mem;

    unsigned int dram[0x100];
    _Mem Ram8051[] = {
        {0,     8,  8,   "REG_BANK_0", 0x0001},
        {0x8,   8,  8,   "REG_BANK_1", 0x0002},
        {0x10,  8,  8,   "REG_BANK_2", 0x0004},
        {0x18,  8,  8,   "REG_BANK_3", 0x0008},
        {0x20,  0,  16,  "BSEG_BYTES", 0x0010},
        {0,     0,  128, "UNUSED",     0x0000},
        {0x7f,  0,  128, "DATA",       0x0020},
        {0,     0,  128, "TOTAL:",     0x0000}
    };

    _Mem IRam8051 =  {0xff,   0,   128, "INDIRECT RAM",       0x0080};
    _Mem Stack8051 = {0xff,   0,     1, "STACK",              0x0000};
    _Mem XRam8051 =  {0xffff, 0, 65536, "EXTERNAL RAM",       0x0100};
    _Mem Rom8051 =   {0xffff, 0, 65536, "ROM/EPROM/FLASH",    0x0200};

    _Mem Ram6808[] = {
        {0,     0,      0,       "REG_BANK_0", 0x0001},
        {0x0,   0,      0,       "REG_BANK_1", 0x0002},
        {0x0,   0,      0,       "REG_BANK_2", 0x0004},
        {0x0,   0,      0,       "REG_BANK_3", 0x0008},
        {0x0,   0,      0,       "BSEG_BYTES", 0x0010},
        {0,     0,      256,    "UNUSED",     0x0000},
        {0xff,  0,      256,    "DATA",       0x0020},
        {0,             0,      256, "TOTAL:",     0x0000}
    };

    _Mem IRam6808 =  {0xff,   0,     0, "INDIRECT RAM",           0x0080};
    _Mem Stack6808 = {0xff,   0,     1, "STACK",                  0x0000};
    _Mem XRam6808 =  {0xffff, 0, 65536, "EXTERNAL RAM",           0x0100};
    _Mem Rom6808 =   {0xffff, 0, 65536, "ROM/EPROM/FLASH",        0x0200};

    _Mem *Ram = NULL;

    _Mem IRam =  {0, 0, 0, "", 0};
    _Mem Stack = {0, 0, 0, "", 0};
    _Mem XRam =  {0, 0, 0, "", 0};
    _Mem Rom =   {0, 0, 0, "", 0};

    if (TARGET_IS_8051) {
        Ram = Ram8051;
        memcpy(&IRam, &IRam8051, sizeof (_Mem));
        memcpy(&Stack, &Stack8051, sizeof (_Mem));
        memcpy(&XRam, &XRam8051, sizeof (_Mem));
        memcpy(&Rom, &Rom8051, sizeof (_Mem));
    }
    else {
        Ram = Ram6808;
        memcpy(&IRam, &IRam6808, sizeof (_Mem));
        memcpy(&Stack, &Stack6808, sizeof (_Mem));
        memcpy(&XRam, &XRam6808, sizeof (_Mem));
        memcpy(&Rom, &Rom6808, sizeof (_Mem));
    }

    if (stacksize == 0) stacksize = MIN_STACK;

    if (TARGET_IS_8051) {
        if(rflag) /*For the DS390*/
        {
            XRam.Max=0x1000000; /*24 bits*/
            XRam.Start=0xffffff;
            Rom.Max=0x1000000;
            Rom.Start=0xffffff;
        }

        if((iram_size<=0)||(iram_size>0x100)) /*Default: 8052 like memory*/
        {
            Ram[5].Max=0x80;
            Ram[6].Max=0x80;
            Ram[7].Max=0x80;
            IRam.Max=0x80;
            iram_size=0x100;
        }
        else if(iram_size<0x80)
        {
            Ram[5].Max=iram_size;
            Ram[6].Max=iram_size;
            Ram[7].Max=iram_size;
            IRam.Max=0;
        }
        else
        {
            Ram[5].Max=0x80;
            Ram[6].Max=0x80;
            Ram[7].Max=0x80;
            IRam.Max=iram_size-0x80;
        }
    }

    for(j=0; j<(int)iram_size; j++) dram[j]=0;
    for(; j<0x100; j++) dram[j]=0x8000; /*Memory not available*/

    /* Open Memory Summary File*/
    of = afile(linkp->f_idp, "mem", 1);
    if (of == NULL)
    {
        lkexit(1);
    }

    xp=areap;
    while (xp)
    {
        /**/ if (EQ(xp->a_id, "REG_BANK_0"))
        {
            Ram[0].Size=xp->a_size;
        }
        else if (EQ(xp->a_id, "REG_BANK_1"))
        {
            Ram[1].Size=xp->a_size;
        }
        else if (EQ(xp->a_id, "REG_BANK_2"))
        {
            Ram[2].Size=xp->a_size;
        }
        else if (EQ(xp->a_id, "REG_BANK_3"))
        {
            Ram[3].Size=xp->a_size;
        }
        else if (EQ(xp->a_id, "BSEG_BYTES"))
        {
            if (TARGET_IS_8051)
            	Ram[4].Size+=xp->a_size;
            else
                Ram[4].Size=xp->a_size;
        }

        else if (EQ(xp->a_id, "SSEG"))
        {
            Stack.Size+=xp->a_size;
            if(xp->a_addr<Stack.Start) Stack.Start=xp->a_addr;
        }

        else if (EQ(xp->a_id, "ISEG"))
        {
            IRam.Size+=xp->a_size;
            if(xp->a_addr<IRam.Start) IRam.Start=xp->a_addr;
        }

        else if (TARGET_IS_8051)
        {
            if(xp->a_flag & A_XDATA)
            {
                if(xp->a_size>0)
                {
                    XRam.Size+=xp->a_size;
                    if(xp->a_addr<XRam.Start) XRam.Start=xp->a_addr;
                }
            }

            else if (EQ(xp->a_id, "BIT_BANK"))
            {
                Ram[4].Size+=xp->a_size;
            }

            else if(xp->a_flag & A_CODE)
            {
                if(xp->a_size>0)
                {
                    Rom.Size+=xp->a_size;
                    if(xp->a_addr<Rom.Start) Rom.Start=xp->a_addr;
                }
            }
        }

        else if(TARGET_IS_6808)
        {
            if ( EQ(xp->a_id, "DSEG") || EQ(xp->a_id, "OSEG") )
            {
                Ram[6].Size+=xp->a_size;
                if(xp->a_addr<Ram[6].Start) Ram[6].Start=xp->a_addr;
            }

            else if( EQ(xp->a_id, "CSEG") || EQ(xp->a_id, "GSINIT") ||
                         EQ(xp->a_id, "GSFINAL") || EQ(xp->a_id, "HOME") )
            {
                Rom.Size+=xp->a_size;
                if(xp->a_addr<Rom.Start) Rom.Start=xp->a_addr;
            }

            else if (EQ(xp->a_id, "XSEG") || EQ(xp->a_id, "XISEG"))
            {
                    XRam.Size+=xp->a_size;
                    if(xp->a_addr<XRam.Start) XRam.Start=xp->a_addr;
            }
        }

        /*If is not a register bank, bit, stack, or idata, then it should be data*/
        else if((TARGET_IS_8051 && xp->a_flag & (A_CODE|A_BIT|A_XDATA))==0)
        {
            if(xp->a_size)
            {
                Ram[6].Size+=xp->a_size;
                if(xp->a_addr<Ram[6].Start) Ram[6].Start=xp->a_addr;
            }
        }

        xp=xp->a_ap;
    }

    for(j=0; j<7; j++)
        for(k=Ram[j].Start; (k<(Ram[j].Start+Ram[j].Size))&&(k<0x100); k++)
            dram[k]|=Ram[j].flag; /*Mark as used*/

    if (TARGET_IS_8051) {
        for(k=IRam.Start; (k<(IRam.Start+IRam.Size))&&(k<0x100); k++)
            dram[k]|=IRam.flag; /*Mark as used*/
    }

    /*Compute the amount of unused memory in direct data Ram.  This is the
    gap between the last register bank or bit segment and the data segment.*/
    for(k=Ram[6].Start-1; (dram[k]==0) && (k>0); k--);
    Ram[5].Start=k+1;
    Ram[5].Size=Ram[6].Start-Ram[5].Start; /*It may be zero (which is good!)*/

    /*Compute the data Ram totals*/
    for(j=0; j<7; j++)
    {
        if(Ram[7].Start>Ram[j].Start) Ram[7].Start=Ram[j].Start;
        Ram[7].Size+=Ram[j].Size;
    }
    Total_Last=Ram[6].Size+Ram[6].Start-1;

    /*Report the Ram totals*/
    fprintf(of, "Direct Internal RAM:\n");
    fprintf(of, format, "Name", "Start", "End", "Size", "Max");

    for(j=0; j<8; j++)
    {
        if((j==0) || (j==7)) fprintf(of, format, line, line, line, line, line);
        if((j!=5) || (Ram[j].Size>0))
        {
            sprintf(start, "0x%02lx", Ram[j].Start);
            if(Ram[j].Size==0)
                end[0]=0;/*Empty string*/
            else
                sprintf(end,  "0x%02lx", j==7?Total_Last:Ram[j].Size+Ram[j].Start-1);
            sprintf(size, "%5lu", Ram[j].Size);
            sprintf(max, "%5lu", Ram[j].Max);
            fprintf(of, format, Ram[j].Name, start, end, size, max);
        }
    }

    if (TARGET_IS_8051) {
        for(k=Ram[6].Start; (k<(Ram[6].Start+Ram[6].Size))&&(k<0x100); k++)
        {
            if(dram[k]!=Ram[6].flag)
            {
                sprintf(buff, "Internal memory overlap starting at 0x%02x.\n", k);
                REPORT_ERROR(buff, 1);
                break;
            }
        }

        if(Ram[4].Size>Ram[4].Max)
        {
            k=Ram[4].Size-Ram[4].Max;
            sprintf(buff, "Insufficient bit addressable memory.  "
                        "%d byte%s short.\n", k, (k==1)?"":"s");
            REPORT_ERROR(buff, 1);
        }

        if(Ram[5].Size!=0)
        {
            sprintf(buff, "%ld bytes in data memory wasted.  "
                        "SDCC link could use: --data-loc 0x%02lx\n",
                        Ram[5].Size, Ram[6].Start-Ram[5].Size);
            REPORT_WARNING(buff, 1);
        }

        if((Ram[6].Start+Ram[6].Size)>Ram[6].Max)
        {
            k=(Ram[6].Start+Ram[6].Size)-Ram[6].Max;
            sprintf(buff, "Insufficient space in data memory.   "
                        "%d byte%s short.\n", k, (k==1)?"":"s");
            REPORT_ERROR(buff, 1);
        }
    }

    /*Report the position of the beginning of the stack*/
    fprintf(of, "\n%stack starts at: 0x%02lx (sp set to 0x%02lx)",
        rflag ? "16 bit mode initial s" : "S", Stack.Start, Stack.Start-1);

    if (TARGET_IS_8051) {
        /*Check that the stack pointer is landing in a safe place:*/
        if( (dram[Stack.Start] & 0x8000) == 0x8000 )
        {
            fprintf(of, ".\n");
            sprintf(buff, "Stack set to unavailable memory.\n");
            REPORT_ERROR(buff, 1);
        }
        else if(dram[Stack.Start])
        {
            fprintf(of, ".\n");
            sprintf(buff, "Stack overlaps area ");
            REPORT_ERROR(buff, 1);
            for(j=0; j<7; j++)
            {
                if(dram[Stack.Start]&Ram[j].flag)
                {
                    sprintf(buff, "'%s'\n", Ram[j].Name);
                    break;
                }
            }
            if(dram[Stack.Start]&IRam.flag)
            {
                sprintf(buff, "'%s'\n", IRam.Name);
            }
            REPORT_ERROR(buff, 0);
        }
        else
        {
            for(j=Stack.Start, k=0; (j<(int)iram_size)&&(dram[j]==0); j++, k++);
            fprintf(of, " with %d bytes available\n", k);
            if ((int)k<stacksize)
            {
                sprintf(buff, "Only %d byte%s available for stack.\n",
                    k, (k==1)?"":"s");
                REPORT_WARNING(buff, 1);
            }
        }
    }

    fprintf(of, "\nOther memory:\n");
    fprintf(of, format, "Name", "Start", "End", "Size", "Max");
    fprintf(of, format, line, line, line, line, line);

    /*Report IRam totals:*/
    if(IRam.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%02lx", IRam.Start);
        sprintf(end,  "0x%02lx", IRam.Size+IRam.Start-1);
    }
    sprintf(size, "%5lu", IRam.Size);
    sprintf(max, "%5lu", IRam.Max);
    fprintf(of, format, IRam.Name, start, end, size, max);

    /*Report XRam totals:*/
    if(XRam.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%04lx", XRam.Start);
        sprintf(end,  "0x%04lx", XRam.Size+XRam.Start-1);
    }
    sprintf(size, "%5lu", XRam.Size);
    sprintf(max, "%5lu", xram_size<0?XRam.Max:xram_size);
    fprintf(of, format, XRam.Name, start, end, size, max);

    /*Report Rom/Flash totals:*/
    if(Rom.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%04lx", Rom.Start);
        sprintf(end,  "0x%04lx", Rom.Size+Rom.Start-1);
    }
    sprintf(size, "%5lu", Rom.Size);
    sprintf(max, "%5lu", code_size<0?Rom.Max:code_size);
    fprintf(of, format, Rom.Name, start, end, size, max);

    /*Report any excess:*/
    if (TARGET_IS_8051) {
        if((IRam.Start+IRam.Size)>(IRam.Max+0x80))
        {
            sprintf(buff, "Insufficient INDIRECT RAM memory.\n");
            REPORT_ERROR(buff, 1);
        }
    }
    if( ((XRam.Start+XRam.Size)>XRam.Max) ||
        (((int)XRam.Size>xram_size)&&(xram_size>=0)) )
    {
        sprintf(buff, "Insufficient EXTERNAL RAM memory.\n");
        REPORT_ERROR(buff, 1);
    }
    if( ((Rom.Start+Rom.Size)>Rom.Max) ||
        (((int)Rom.Size>code_size)&&(code_size>=0)) )
    {
        sprintf(buff, "Insufficient ROM/EPROM/FLASH memory.\n");
        REPORT_ERROR(buff, 1);
    }

    fclose(of);
    return toreturn;
  }
  else {
    assert (0);
    return 0;
  }
}

int summary2(struct area * areap)
{
  if (TARGET_IS_8051) {
    /* only for 8051 target */

    #define EQ(A,B) !strcasecmp((A),(B))

    char buff[128];
    int toreturn = 0;
    unsigned int j;
    unsigned long int Stack_Start=0, Stack_Size;

    struct area * xp;
    struct area * xstack_xp = NULL;
    FILE * of;

    /*Artifacts used for printing*/
    char start[15], end[15], size[15], max[15];
    char format[]="   %-16.16s %-8.8s %-8.8s %-8.8s %-8.8s\n";
    char line[]="---------------------";

    typedef struct
    {
        unsigned long Start;
        unsigned long End;
        unsigned long Size;
        unsigned long Max;
        char Name[NCPS];
        unsigned long flag;
    } _Mem;

    _Mem Stack={0xff,   0, 0,     1, "STACK",           0x0000};
    _Mem Paged={0xffff, 0, 0,   256, "PAGED EXT. RAM",  A3_PAG};
    _Mem XRam= {0xffff, 0, 0, 65536, "EXTERNAL RAM",    0x0100};
    _Mem Rom=  {0xffff, 0, 0, 65536, "ROM/EPROM/FLASH", 0x0200};

    if(rflag) /*For the DS390*/
    {
        XRam.Max=0x1000000; /*24 bits*/
        XRam.Start=0xffffff;
        Rom.Max=0x1000000;
        Rom.Start=0xffffff;
    }

    /* Open Memory Summary File*/
    of = afile(linkp->f_idp, "mem", 1);
    if (of == NULL)
    {
        lkexit(1);
    }

    xp=areap;
    while (xp)
    {
        if (xp->a_flag & A_CODE)
        {
            if(xp->a_size)
            {
                Rom.Size += xp->a_size;
                if(xp->a_addr < Rom.Start)
                    Rom.Start = xp->a_addr;
                if(xp->a_addr + xp->a_size > Rom.End)
                    Rom.End = xp->a_addr + xp->a_size;
            }
        }

        else if (EQ(xp->a_id, "SSEG"))
        {
            Stack.Size += xp->a_size;
            if(xp->a_addr < Stack.Start)
                Stack.Start = xp->a_addr;
            if(xp->a_addr + xp->a_size > Stack.End)
                Stack.End = xp->a_addr + xp->a_size;
        }

        else if (EQ(xp->a_id, "PSEG"))
        {
            Paged.Size += xp->a_size;
            if(xp->a_addr < Paged.Start)
                Paged.Start = xp->a_addr;
            if(xp->a_addr + xp->a_size > Paged.End)
                Paged.End = xp->a_addr + xp->a_size;
        }

        else if (EQ(xp->a_id, "XSTK"))
        {
            xstack_xp = xp;
            Paged.Size += xp->a_size;
            if(xp->a_addr < Paged.Start)
                Paged.Start = xp->a_addr;
            if(xp->a_addr + xp->a_size > Paged.End)
                Paged.End = xp->a_addr + xp->a_size;
        }

        else if (xp->a_flag & A_XDATA)
        {
            if(xp->a_size)
            {
                XRam.Size += xp->a_size;
                if(xp->a_addr < XRam.Start)
                    XRam.Start = xp->a_addr;
                if(xp->a_addr + xp->a_size > XRam.End)
                    XRam.End = xp->a_addr + xp->a_size;
            }
        }

        xp = xp->a_ap;
    }

    /*Report the Ram totals*/
    fprintf(of, "Internal RAM layout:\n");
    fprintf(of, "      0 1 2 3 4 5 6 7 8 9 A B C D E F");
    for(j=0; j<256; j++)
    {
        if(j%16==0) fprintf(of, "\n0x%02x:|", j);
        fprintf(of, "%c|", idatamap[j]);
    }
    fprintf(of, "\n0-3:Reg Banks, T:Bit regs, a-z:Data, B:Bits, Q:Overlay, I:iData, S:Stack, A:Absolute\n");

    for(j=0; j<256; j++)
    {
        if(idatamap[j]=='S')
        {
            Stack_Start=j;
            break;
        }
    }

    for(j=Stack_Start, Stack_Size=0; j<((iram_size)?iram_size:256); j++)
    {
        if((idatamap[j]=='S')||(idatamap[j]==' ')) Stack_Size++;
        else break;
    }

    xp=areap;
    while (xp)
    {
        if(xp->a_unaloc>0)
        {
            fprintf(of, "\nERROR: Couldn't get %d byte%s allocated"
                        " in internal RAM for area %s.",
                        xp->a_unaloc, xp->a_unaloc>1?"s":"", xp->a_id);
            toreturn=1;
        }
        xp=xp->a_ap;
    }

    /*Report the position of the begining of the stack*/
    if(Stack_Start!=256)
        fprintf(of, "\n%s starts at: 0x%02lx (sp set to 0x%02lx) with %ld bytes available.",
            rflag ? "16 bit mode initial stack" : "Stack", Stack_Start, Stack_Start-1, Stack_Size);
    else
        fprintf(of, "\nI don't have a clue where the stack ended up! Sorry...");

    /*Report about xstack*/
    if (xstack_xp)
    {
        Stack_Start = xstack_xp->a_addr;
        Stack_Size = xstack_xp->a_size;
        fprintf(of, "\nXstack starts at: 0x%04lx with %ld bytes available.",
            Stack_Start, Stack_Size);
    }

    fprintf(of, "\n\nOther memory:\n");
    fprintf(of, format, "Name", "Start", "End", "Size", "Max");
    fprintf(of, format, line, line, line, line, line);

    /*Report Paged XRam totals:*/
    if(Paged.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%04lx", Paged.Start);
        sprintf(end,  "0x%04lx", Paged.End-1);
    }
    sprintf(size, "%5lu", Paged.Size);
    sprintf(max, "%5lu", xram_size<0 ? Paged.Max : xram_size<256 ? xram_size : 256);
    fprintf(of, format, Paged.Name, start, end, size, max);

    /*Report XRam totals:*/
    if(XRam.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%04lx", XRam.Start);
        sprintf(end,  "0x%04lx", XRam.End-1);
    }
    sprintf(size, "%5lu", XRam.Size);
    sprintf(max, "%5lu", xram_size<0?XRam.Max:xram_size);
    fprintf(of, format, XRam.Name, start, end, size, max);

    /*Report Rom/Flash totals:*/
    if(Rom.Size==0)
    {
        start[0]=0;/*Empty string*/
        end[0]=0;/*Empty string*/
    }
    else
    {
        sprintf(start, "0x%04lx", Rom.Start);
        sprintf(end,  "0x%04lx", Rom.End-1);
    }
    sprintf(size, "%5lu", Rom.Size);
    sprintf(max, "%5lu", code_size<0?Rom.Max:code_size);
    fprintf(of, format, Rom.Name, start, end, size, max);

    /*Report any excess:*/
    if( ((XRam.End) > XRam.Max) ||
        (((int)XRam.Size>xram_size)&&(xram_size>=0)) )
    {
        sprintf(buff, "Insufficient EXTERNAL RAM memory.\n");
        REPORT_ERROR(buff, 1);
    }
    if( ((Rom.End) > Rom.Max) ||
        (((int)Rom.Size>code_size)&&(code_size>=0)) )
    {
        sprintf(buff, "Insufficient ROM/EPROM/FLASH memory.\n");
        REPORT_ERROR(buff, 1);
    }

    fclose(of);
    return toreturn;
  }
  else {
    assert (0);
    return 0;
  }
}

