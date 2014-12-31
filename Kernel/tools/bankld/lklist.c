/* lklist.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 *
 */

/*
 * 28-Oct-97 JLH:
 *           - lstarea: show s_id as string rather than array [NCPS]
 *           - lstarea: show a_id as string rather than array [NCPS]
 * 31-Oct-97 JLH: add NoICE output file genration in lstarea
 * 02-Apr-98 JLH: add XDATA, DATA, BIT flags to area output
 */

#include "aslink.h"

/*)Module       lklist.c
 *
 *      The module lklist.c contains the functions which
 *      output the linker .map file and produce a relocated
 *      listing .rst file.
 *
 *      lklist.c contains the following functions:
 *              int     dgt()
 *              VOID    newpag()
 *              VOID    slew()
 *              VOID    lstarea()
 *              VOID    lkulist()
 *              VOID    lkalist()
 *              VOID    lkglist()
 *
 *      lklist.c contains no local variables.
 */

/*)Function     VOID    newpag()
 *
 *      The function newpag() outputs a page skip, writes the
 *      first page header line, sets the line count to 1, and
 *      increments the page counter.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              int     lop             current line number on page
 *              int     page            current page number
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              The page and line counters are updated.
 */

VOID
newpag(FILE *fp)
{
        fprintf(fp, "\fASxxxx Linker %s,  page %u.\n", VERSION, ++page);
        lop = 1;
}

/*)Function     int     dgt(rdx,str,n)
 *
 *              int     rdx             radix bit code
 *              char    *str            pointer to the test string
 *              int     n               number of characters to check
 *
 *      The function dgt() verifies that the string under test
 *      is of the specified radix.
 *
 *      local variables:
 *              int     i               loop counter
 *
 *      global variables:
 *              ctype[]                 array of character types
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              none
 */

int
dgt(int rdx, char *str, int n)
{
        int i;

        for (i=0; i<n; i++) {
                if ((ctype[*str++ & 0x007F] & rdx) == 0)
                        return(0);
        }
        return(1);
}

/*)Function     VOID    slew(xp, yp)
 *
 *              area *  xp              pointer to an area structure
 *              bank *  yp              pointer to a  bank structure
 *
 *      The function slew() increments the page line counter.
 *      If the number of lines exceeds the maximum number of
 *      lines per page then a page skip and a page header are
 *      output.
 *
 *      local variables:
 *              a_uint  ai              temporary
 *              a_uint  aj              temporary
 *              int     i               loop counter
 *              int     n               repeat counter
 *              char *  frmta           temporary format specifier
 *              char *  frmtb           temporary format specifier
 *              char *  ptr             pointer to an id string
 *
 *      global variables:
 *              int     a_bytes         T line address bytes
 *              int     a_mask          addressing mask
 *              int     lop             current line number on page
 *              FILE    *mfp            Map output file handle
 *              int     wflag           Wide format listing
 *              int     xflag           Map file radix type flag
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    newpag()        lklist.c
 *              char    putc()          c_library
 *
 *      side effects:
 *              The page line and the page count may be updated.
 */

VOID
slew(struct area *xp, struct bank *yp)
{
        int i, n;
        char *frmta, *frmtb, *ptr;
        a_uint  ai, aj;

        if (lop++ >= NLPP) {
                newpag(mfp);
                switch(xflag) {
                default:
                case 0: frmta = "Hexadecimal"; break;
                case 1: frmta = "Octal"; break;
                case 2: frmta = "Decimal"; break;
                }
                fprintf(mfp, "%s  [%d-Bits]\n", frmta, a_bytes*8);
                if (*yp->b_id) {
                        fprintf(mfp, "[ Bank == %s ]\n", yp->b_id);
                        lop += 1;
                }
                fprintf(mfp, "\n");
                if (wflag) {
                        fprintf(mfp,
                                "Area                                    Addr");
                        fprintf(mfp,
                                "        Size        Decimal Bytes (Attributes)\n");
                        fprintf(mfp,
                                "--------------------------------        ----");
                        fprintf(mfp,
                                "        ----        ------- ----- ------------\n");
                } else {
                        fprintf(mfp,
                                "Area                       Addr   ");
                        fprintf(mfp,
                                "     Size        Decimal Bytes (Attributes)\n");
                        fprintf(mfp,
                                "--------------------       ----   ");
                        fprintf(mfp,
                                "     ----        ------- ----- ------------\n");
                }

                ai = xp->a_addr & a_mask;
                aj = xp->a_size & a_mask;

                /*
                 * Output Area Header
                 */
                ptr = &xp->a_id[0];
                if (wflag) {
                        fprintf(mfp, "%-32.32s", ptr);
                } else {
                        fprintf(mfp, "%-19.19s", ptr);
                }
#ifdef  LONGINT
                switch(a_bytes) {
                default:
                case 2:
                        switch(xflag) {
                        default:
                        case 0: frmta = "        %04lX        %04lX"; break;
                        case 1: frmta = "      %06lo      %06lo"; break;
                        case 2: frmta = "       %05lu       %05lu"; break;
                        }
                        frmtb = " =      %6lu. bytes "; break;
                case 3:
                        switch(xflag) {
                        default:
                        case 0: frmta = "      %06lX      %06lX"; break;
                        case 1: frmta = "    %08lo    %08lo"; break;
                        case 2: frmta = "    %08lu    %08lu"; break;
                        }
                        frmtb = " =    %8lu. bytes "; break;
                case 4:
                        switch(xflag) {
                        default:
                        case 0: frmta = "    %08lX    %08lX"; break;
                        case 1: frmta = " %011lo %011lo"; break;
                        case 2: frmta = "  %010lu  %010lu"; break;
                        }
                        frmtb = " =  %10lu. bytes "; break;
                }
#else
                switch(a_bytes) {
                default:
                case 2:
                        switch(xflag) {
                        default:
                        case 0: frmta = "        %04X        %04X"; break;
                        case 1: frmta = "      %06o      %06o"; break;
                        case 2: frmta = "       %05u       %05u"; break;
                        }
                        frmtb = " =      %6u. bytes "; break;
                case 3:
                        switch(xflag) {
                        default:
                        case 0: frmta = "      %06X      %06X"; break;
                        case 1: frmta = "    %08o    %08o"; break;
                        case 2: frmta = "    %08u    %08u"; break;
                        }
                        frmtb = " =    %8u. bytes "; break;
                case 4:
                        switch(xflag) {
                        default:
                        case 0: frmta = "    %08X    %08X"; break;
                        case 1: frmta = " %011o %011o"; break;
                        case 2: frmta = "  %010u  %010u"; break;
                        }
                        frmtb = " =  %10u. bytes "; break;
                }
#endif
                fprintf(mfp, frmta, ai, aj);
                fprintf(mfp, frmtb, aj);

                if (xp->a_flag & A3_ABS) {
                        fprintf(mfp, "(ABS");
                } else {
                        fprintf(mfp, "(REL");
                }
                if (xp->a_flag & A3_OVR) {
                        fprintf(mfp, ",OVR");
                } else {
                        fprintf(mfp, ",CON");
                }
                if (xp->a_flag & A3_PAG) {
                        fprintf(mfp, ",PAG");
                }

                /* sdld specific */
                if (xp->a_flag & A_CODE) {
                        fprintf(mfp, ",CODE");
                }
                if (xp->a_flag & A_XDATA) {
                        fprintf(mfp, ",XDATA");
                }
                if (xp->a_flag & A_BIT) {
                        fprintf(mfp, ",BIT");
                }
                /* end sdld specific */

                fprintf(mfp, ")\n");
                if (wflag) {
                        putc('\n', mfp);
                        fprintf(mfp,
                        "      Value  Global                           ");
                        fprintf(mfp,
                        "   Global Defined In Module\n");
                        fprintf(mfp,
                        "      -----  --------------------------------");
                        fprintf(mfp,
                        "   ------------------------\n");
                } else {
                        switch(a_bytes) {
                        default:
                        case 2: frmta = "   Value  Global   ";
                                frmtb = "   -----  ------   ";
                                n = 4; break;
                        case 3:
                        case 4: frmta = "        Value  Global    ";
                                frmtb = "        -----  ------    ";
                                n = 3; break;
                        }
                        putc('\n', mfp);
                        for(i=0;i<n;++i)
                                fprintf(mfp, "%s", frmta);
                        putc('\n', mfp);
                        for(i=0;i<n;++i)
                                fprintf(mfp, "%s", frmtb);
                        putc('\n', mfp);
                }

                lop += 9;
        }
}

/* sdld specific */
/* Used for qsort call in lstsym */
static int _cmpSymByAddr(const void *p1, const void *p2)
{
    struct sym **s1 = (struct sym **)(p1);
    struct sym **s2 = (struct sym **)(p2);
    int delta = ((*s1)->s_addr + (*s1)->s_axp->a_addr) -
                ((*s2)->s_addr + (*s2)->s_axp->a_addr);

    /* Sort first by address, then by name. */
    if (delta)
    {
        return delta;
    }
    return strcmp((*s1)->s_id,(*s2)->s_id);
}
/* end sdld specific */

/*)Function     VOID    lstarea(xp, yp)
 *
 *              area *  xp              pointer to an area structure
 *              bank *  yp              pointer to a  bank structure
 *
 *      The function lstarea() creates the linker map output for
 *      the area specified by pointer xp.  The generated output
 *      area header includes the area name, starting address,
 *      size of area, number of words (in decimal), and the
 *      area attributes.  The symbols defined in this area are
 *      sorted by ascending address and output four per line
 *      in the selected radix (one per line in wide format).
 *
 *      local variables:
 *              areax * oxp             pointer to an area extension structure
 *              int     i               loop counter
 *              int     j               bubble sort update status
 *              int     n               repeat counter
 *              char *  frmt            temporary format specifier
 *              char *  ptr             pointer to an id string
 *              int     nmsym           number of symbols in area
 *              a_uint  a0              temporary
 *              a_uint  ai              temporary
 *              a_uint  aj              temporary
 *              sym *   sp              pointer to a symbol structure
 *              sym **  p               pointer to an array of
 *                                      pointers to symbol structures
 *
 *      global variables:
 *              int     a_bytes         T line address bytes
 *              FILE    *mfp            Map output file handle
 *              sym *symhash[NHASH]     array of pointers to NHASH
 *                                      linked symbol lists
 *              int     wflag           Wide format listing
 *              int     xflag           Map file radix type flag
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    free()          c_library
 *              char *  malloc()        c_library
 *              char    putc()          c_library
 *              VOID    slew()          lklist.c
 *
 *      side effects:
 *              Map output generated.
 */

VOID
lstarea(struct area *xp, struct bank *yp)
{
        struct areax *oxp;
        int i, j, n;
        char *frmt, *ptr;
        int nmsym;
        a_uint a0, ai, aj;
        struct sym *sp;
        struct sym **p;
        /* sdld specific */
        int memPage;
        /* end sdld specific */

        /*
         * Find number of symbols in area
         */
        nmsym = 0;
        oxp = xp->a_axp;
        while (oxp) {
                for (i=0; i<NHASH; i++) {
                        sp = symhash[i];
                        while (sp != NULL) {
                                if (oxp == sp->s_axp)
                                        ++nmsym;
                                sp = sp->s_sp;
                        }
                }
                oxp = oxp->a_axp;
        }

        if ((nmsym == 0) && (xp->a_size == 0)) {
                return;
        }

        lop = NLPP;
        slew(xp, yp);

        if (nmsym == 0) {
                return;
        }

        /*
         * Allocate space for an array of pointers to symbols
         * and load array.
         */
        if ( (p = (struct sym **) malloc (nmsym*sizeof(struct sym *))) == NULL) {
                fprintf(mfp, "Insufficient space to build Map Segment.\n");
                return;
        }
        nmsym = 0;
        oxp = xp->a_axp;
        while (oxp) {
                for (i=0; i<NHASH; i++) {
                        sp = symhash[i];
                        while (sp != NULL) {
                                if (oxp == sp->s_axp) {
                                        p[nmsym++] = sp;
                                }
                                sp = sp->s_sp;
                        }
                }
                oxp = oxp->a_axp;
        }

        if (is_sdld()) {
                /*
                 * Quick Sort of Addresses in Symbol Table Array
                 */
                qsort(p, nmsym, sizeof(struct sym *), _cmpSymByAddr);
        } else {
                /*
                 * Bubble Sort of Addresses in Symbol Table Array
                 */
                j = 1;
                while (j) {
                        j = 0;
                        sp = p[0];
                        a0 = sp->s_addr + sp->s_axp->a_addr;
                        for (i=1; i<nmsym; ++i) {
                                sp = p[i];
                                ai = sp->s_addr + sp->s_axp->a_addr;
                                if (a0 > ai) {
                                        j = 1;
                                        p[i] = p[i-1];
                                        p[i-1] = sp;
                                }
                                a0 = ai;
                        }
                }
        }

        /*
         * Repeat Counter
         */
        switch(a_bytes) {
        default:
        case 2: n = 4; break;
        case 3:
        case 4: n = 3; break;
        }

        /*
         * Symbol Table Output
         */
        /* sdld specific */
        memPage = (xp->a_flag & A_CODE) ? 0x0C : ((xp->a_flag & A_XDATA) ? 0x0D : ((xp->a_flag & A_BIT) ? 0x0B : 0x00));
        /* end sdld specific */
        i = 0;
        while (i < nmsym) {
                if (wflag) {
                        slew(xp, yp);
                        if (is_sdld()) {
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "     "; break;
                                case 3:
                                case 4: frmt = ""; break;
                                }
                                if (memPage != 0)
                                        fprintf(mfp, "%s%X:", frmt, memPage);
                                else
                                        fprintf(mfp, "%s  ", frmt);
                        } else {
                                switch(a_bytes) {
                                default:
                                case 2: frmt = "        "; break;
                                case 3:
                                case 4: frmt = "   "; break;
                                }
                                fprintf(mfp, "%s", frmt);
                        }
                } else
                if ((i % n) == 0) {
                        slew(xp, yp);
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "  "; break;
                        case 3:
                        case 4: frmt = "  "; break;
                        }
                        fprintf(mfp, "%s", frmt);
                }

                sp = p[i];
                aj = (sp->s_addr + sp->s_axp->a_addr) & a_mask;
#ifdef  LONGINT
                switch(a_bytes) {
                default:
                case 2:
                        switch(xflag) {
                        default:
                        case 0: frmt = "  %04lX  "; break;
                        case 1: frmt = "%06lo  "; break;
                        case 2: frmt = " %05lu  "; break;
                        }
                        break;
                case 3:
                        switch(xflag) {
                        default:
                        case 0: frmt = "     %06lX  "; break;
                        case 1: frmt = "   %08lo  "; break;
                        case 2: frmt = "   %08lu  "; break;
                        }
                        break;
                case 4:
                        switch(xflag) {
                        default:
                        case 0: frmt = "   %08lX  "; break;
                        case 1: frmt = "%011lo  "; break;
                        case 2: frmt = " %010lu  "; break;
                        }
                        break;
                }
#else
                switch(a_bytes) {
                default:
                case 2:
                        switch(xflag) {
                        default:
                        case 0: frmt = "  %04X  "; break;
                        case 1: frmt = "%06o  "; break;
                        case 2: frmt = " %05u  "; break;
                        }
                        break;
                case 3:
                        switch(xflag) {
                        default:
                        case 0: frmt = "     %06X  "; break;
                        case 1: frmt = "   %08o  "; break;
                        case 2: frmt = "   %08u  "; break;
                        }
                        break;
                case 4:
                        switch(xflag) {
                        default:
                        case 0: frmt = "   %08X  "; break;
                        case 1: frmt = "%011o  "; break;
                        case 2: frmt = " %010u  "; break;
                        }
                        break;
                }
#endif
                fprintf(mfp, frmt, aj);

                ptr = &sp->s_id[0];

#if NOICE
                /*
                 * NoICE output of symbol
                 */
                if (jflag) DefineNoICE(ptr, aj, yp);
#endif

#if SDCDB
                /*
                 * SDCDB output of symbol
                 */
                if (yflag) DefineSDCDB(ptr, aj);
#endif

                if (wflag) {
                        fprintf(mfp, "%-32.32s", ptr);
                        i++;
                        ptr = &sp->m_id[0];
                        if(ptr) {
                                fprintf(mfp, "   %-.28s", ptr);
                        }
                } else {
                        switch(a_bytes) {
                        default:
                        case 2: frmt = "%-8.8s"; break;
                        case 3:
                        case 4: frmt = "%-9.9s"; break;
                        }
                        fprintf(mfp, frmt, ptr);
                        if (++i < nmsym)
                                if (i % n != 0)
                                        fprintf(mfp, " | ");
                }
                if (wflag || (i % n == 0)) {
                        putc('\n', mfp);
                }
        }
        if (i % n != 0) {
                putc('\n', mfp);
        }
        free(p);
}

/*)Function     VOID    lkulist(i)
 *
 *              int     i       i # 0   process LST to RST file
 *                              i = 0   copy remainder of LST file
 *                                      to RST file and close files
 *
 *      The function lkulist() creates a relocated listing (.rst)
 *      output file from the ASxxxx assembler listing (.lst)
 *      files.  The .lst file's program address and code bytes
 *      are changed to reflect the changes made by ASlink as
 *      the .rel files are combined into a single relocated
 *      output file.
 *
 *      local variables:
 *              a_uint  cpc             current program counter address in PC increments
 *              int     cbytes          bytes so far in T line
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              int     hilo            byte order
 *              int     gline           get a line from the LST file
 *                                      to translate for the RST file
 *              a_uint  pc              current program counter address in bytes
 *              int     pcb             bytes per instruction word
 *              char    rb[]            read listing file text line
 *              FILE    *rfp            The file handle to the current
 *                                      output RST file
 *              int     rtcnt           count of data words
 *              int     rtflg[]         output the data flag
 *              a_uint  rtval[]         relocated data
 *              int     rterr[]         error flag ???
 *              FILE    *tfp            The file handle to the current
 *                                      LST file being scanned
 *
 *      functions called:
 *              int     fclose()        c_library
 *              int     fgets()         c_library
 *              int     fprintf()       c_library
 *              VOID    lkalist()       lklist.c
 *              VOID    lkglist()       lklist.c
 *
 *      side effects:
 *              A .rst file is created for each available .lst
 *              file associated with a .rel file.
 */

VOID
lkulist(int i)
{
        a_uint cpc;
        int cbytes;

        /*
         * Exit if listing file is not open
         */
        if (tfp == NULL)
                return;

        /*
         * Normal processing of LST to RST
         */
        if (i) {
                /*
                 * Line with only address
                 */
                if (rtcnt == a_bytes) {
                        lkalist(pc);

                /*
                 * Line with address and code
                 */
                } else {
                        cpc = pc;
                        cbytes = 0;
                        for (i=a_bytes; i < rtcnt; i++) {
                                if (rtflg[i]) {
                                        lkglist(cpc, (int) (rtval[i] & 0xFF), rterr[i]);
                                        cbytes += 1;
                                        cpc += (cbytes % pcb) ? 0 : 1;
                                }
                        }
                }
        /*
         * Copy remainder of LST to RST
         */
        } else {
                if (gline == 0)
                        fprintf(rfp, "%s", rb);

                while (fgets(rb, sizeof(rb)-2, tfp) != 0) {
                        fprintf(rfp, "%s", rb);
                }
                fclose(tfp);
                tfp = NULL;
                fclose(rfp);
                rfp = NULL;
        }
}

/*)Function     VOID    lkalist(cpc)
 *
 *              int     cpc             current program counter value
 *
 *      The function lkalist() performs the following functions:
 *
 *      (1)     if the value of gline = 0 then the current listing
 *              file line is copied to the relocated listing file output.
 *
 *      (2)     the listing file is read line by line and copied to
 *              the relocated listing file until a valid source
 *              line number and a program counter value of the correct
 *              radix is found.  The new relocated pc value is substituted
 *              and the line is written to the RST file.
 *
 *      local variables:
 *              int     i               loop counter
 *              int     m               character count
 *              int     n               character index
 *              int     r               character radix
 *              char *  frmt            temporary format specifier
 *              char    str[]           temporary string
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              a_uint  a_mask          address masking parameter
 *              int     gcntr           data byte counter
 *              int     gline           get a line from the LST file
 *                                      to translate for the RST file
 *              char    rb[]            read listing file text line
 *              FILE    *rfp            The file handle to the current
 *                                      output RST file
 *              FILE    *tfp            The file handle to the current
 *                                      LST file being scanned
 *
 *      functions called:
 *              int     dgt()           lklist.c
 *              int     fclose()        c_library
 *              int     fgets()         c_library
 *              int     fprintf()       c_library
 *              int     sprintf()       c_library
 *              char *  strncpy()       c_library
 *
 *      side effects:
 *              Lines of the LST file are copied to the RST file,
 *              the last line copied has the code address
 *              updated to reflect the program relocation.
 */

/* The Output Formats,  No Cycle Count
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx xx LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo ooo LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd ddd LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

/* The Output Formats,  With Cycle Count [nn]
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx[nn]LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo [nn]LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd [nn]LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

VOID
lkalist(a_uint cpc)
{
        char str[16];
        char *frmt;
        int m, n, r;

        /*
         * Truncate (int) to N-Bytes
         */
        cpc &= a_mask;

        /*
         * Exit if listing file is not open
         */
loop:   if (tfp == NULL)
                return;

        /*
         * Copy current LST to RST
         */
        if (gline == 0) {
                fprintf(rfp, "%s", rb);
                gline = 1;
        }

        /*
         * Clear text line buffer
         */
        memset(rb, 0, sizeof(rb));

        /*
         * Get next LST text line
         */
        if (fgets(rb, sizeof(rb)-2, tfp) == NULL) {
                fclose(tfp);
                tfp = NULL;
                fclose(rfp);
                rfp = NULL;
                return;
        }

        /*
         * Must have an ASxxxx Listing line number
         */
         switch(a_bytes) {
         default:
         case 2: n = 30; break;
         case 3:
         case 4: n = 38; break;
         }
         if (!dgt(RAD10, &rb[n], 1)) {
                fprintf(rfp, "%s", rb);
                goto loop;
        }

        /*
         * Must have an address in the expected radix
         */
#ifdef  LONGINT
        switch(radix) {
        default:
        case 16:
                r = RAD16;
                switch(a_bytes) {
                default:
                case 2: n = 3; m = 4; frmt = "%04lX"; break;
                case 3: n = 6; m = 6; frmt = "%06lX"; break;
                case 4: n = 4; m = 8; frmt = "%08lX"; break;
                }
                break;
        case 10:
                r = RAD10;
                switch(a_bytes) {
                default:
                case 2: n = 4; m = 5; frmt = "%05lu"; break;
                case 3: n = 5; m = 8; frmt = "%08lu"; break;
                case 4: n = 3; m = 10; frmt = "%010lu"; break;
                }
                break;
        case 8:
                r = RAD8;
                switch(a_bytes) {
                default:
                case 2: n = 3; m = 6; frmt = "%06lo"; break;
                case 3: n = 5; m = 8; frmt = "%08lo"; break;
                case 4: n = 2; m = 11; frmt = "%011lo"; break;
                }
                break;
        }
#else
        switch(radix) {
        default:
        case 16:
                r = RAD16;
                switch(a_bytes) {
                default:
                case 2: n = 3; m = 4; frmt = "%04X"; break;
                case 3: n = 6; m = 6; frmt = "%06X"; break;
                case 4: n = 4; m = 8; frmt = "%08X"; break;
                }
                break;
        case 10:
                r = RAD10;
                switch(a_bytes) {
                default:
                case 2: n = 4; m = 5; frmt = "%05u"; break;
                case 3: n = 5; m = 8; frmt = "%08u"; break;
                case 4: n = 3; m = 10; frmt = "%010u"; break;
                }
                break;
        case 8:
                r = RAD8;
                switch(a_bytes) {
                default:
                case 2: n = 3; m = 6; frmt = "%06o"; break;
                case 3: n = 5; m = 8; frmt = "%08o"; break;
                case 4: n = 2; m = 11; frmt = "%011o"; break;
                }
                break;
        }
#endif
        if (!dgt(r, &rb[n], m)) {
                        fprintf(rfp, "%s", rb);
                        goto loop;
                }
        sprintf(str, frmt, cpc);
        strncpy(&rb[n], str, m);

        /*
         * Copy updated LST text line to RST
         */
        fprintf(rfp, "%s", rb);
        gcntr = 0;
}

/*)Function     VOID    lkglist(cpc,v,err)
 *
 *              int     cpc             current program counter value
 *              int     v               value of byte at this address
 *              int     err             error flag for this value
 *
 *      The function lkglist() performs the following functions:
 *
 *      (1)     if the value of gline = 1 then the listing file
 *              is read line by line and copied to the
 *              relocated listing file until a valid source
 *              line number and a program counter value of the correct
 *              radix is found.
 *
 *      (2)     The new relocated values and code address are
 *              substituted and the line may be written to the RST file.
 *
 *      local variables:
 *              int     a               string index for first byte
 *              int     i               loop counter
 *              int     m               character count
 *              int     n               character index
 *              int     r               character radix
 *              int     s               spacing
 *              int     u               repeat counter
 *              char *  afrmt           temporary format specifier
 *              char *  frmt            temporary format specifier
 *              char    str[]           temporary string
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              a_uint  a_mask          address masking parameter
 *              int     gcntr           data byte counter
 *                                      set to -1 for a continuation line
 *              int     gline           get a line from the LST file
 *                                      to translate for the RST file
 *              char    rb[]            read listing file text line
 *              char    *rp             pointer to listing file text line
 *              FILE    *rfp            The file handle to the current
 *                                      output RST file
 *              FILE    *tfp            The file handle to the current
 *                                      LST file being scanned
 *              char    *errmsg3[]      array of pointers to error strings
 *
 *      functions called:
 *              int     dgt()           lklist.c
 *              int     fclose()        c_library
 *              int     fgets()         c_library
 *              int     fprintf()       c_library
 *              int     sprintf()       c_library
 *              char *  strncpy()       c_library
 *
 *      side effects:
 *              Lines of the LST file are copied to the RST file
 *              with updated data values and code addresses.
 */

/* The Output Formats, No Cycle Count
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx xx LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo ooo LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd ddd LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx xx LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo ooo LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd ddd LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

/* The Output Formats,  With Cycle Count [nn]
| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
   |    |               |     | |
ee XXXX xx xx xx xx xx[nn]LLLLL *************   HEX(16)
ee 000000 ooo ooo ooo [nn]LLLLL *************   OCTAL(16)
ee  DDDDD ddd ddd ddd [nn]LLLLL *************   DECIMAL(16)
                     XXXX
                   OOOOOO
                    DDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
     |       |                  |     | |
ee    XXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(24)
ee   OO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(24)
ee   DDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(24)
                           XXXXXX
                         OOOOOOOO
                         DDDDDDDD

| Tabs- |       |       |       |       |       |
          11111111112222222222333333333344444-----
012345678901234567890123456789012345678901234-----
  |          |                  |     | |
ee  XXXXXXXX xx xx xx xx xx xx[nn]LLLLL *********       HEX(32)
eeOOOOO000000 ooo ooo ooo ooo [nn]LLLLL *********       OCTAL(32)
ee DDDDDDDDDD ddd ddd ddd ddd [nn]LLLLL *********       DECIMAL(32)
                         XXXXXXXX
                      OOOOOOOOOOO
                       DDDDDDDDDD
*/

VOID
lkglist(a_uint cpc, int v, int err)
{
        char str[16];
        char *afrmt, *frmt;
        int a, n, m, r, s, u;

        /*
         * Truncate (int) to N-Bytes
         */
         cpc &= a_mask;

        /*
         * Exit if listing file is not open
         */
loop:   if (tfp == NULL)
                return;

        /*
         * Get next LST text line
         */
        if (gline) {
                /*
                 * Clear text line buffer
                 */
                memset(rb, 0, sizeof(rb));

                /*
                 * Get next LST text line
                 */
                if (fgets(rb, sizeof(rb)-2, tfp) == NULL) {
                        fclose(tfp);
                        tfp = NULL;
                        fclose(rfp);
                        rfp = NULL;
                        return;
                }

                /*
                 * Check for a listing line number if required
                 */
                if (gcntr != -1) {
                        switch(a_bytes) {
                        default:
                        case 2: n = 30; break;
                        case 3:
                        case 4: n = 38; break;
                        }
                        if (!dgt(RAD10, &rb[n], 1)) {
                                fprintf(rfp, "%s", rb);
                                goto loop;
                        }
                        gcntr = 0;
                }
                gline = 0;
        }

        /*
         * Hex Listing
         */
#ifdef  LONGINT
         switch(radix) {
         default:
         case 16:
                r = RAD16;
                switch(a_bytes) {
                default:
                case 2: a = 8; s = 3; n = 3; m = 4; u = 6; afrmt = "%04lX"; break;
                case 3: a = 13; s = 3; n = 6; m = 6; u = 7; afrmt = "%06lX"; break;
                case 4: a = 13; s = 3; n = 4; m = 8; u = 7; afrmt = "%08lX"; break;
                }
                frmt = " %02X"; break;
        case 10:
                r = RAD10;
                switch(a_bytes) {
                default:
                case 2: a = 10; s = 4; n = 4; m = 5; u = 4; afrmt = "%05lu"; break;
                case 3: a = 14; s = 4; n = 5; m = 8; u = 5; afrmt = "%08lu"; break;
                case 4: a = 14; s = 4; n = 3; m = 10; u = 5; afrmt = "%010lu"; break;
                }
                frmt = " %03u"; break;
        case 8:
                r = RAD8;
                switch(a_bytes) {
                default:
                case 2: a = 10; s = 4; n = 3; m = 6; u = 4; afrmt = "%06lo"; break;
                case 3: a = 14; s = 4; n = 5; m = 8; u = 5; afrmt = "%08lo"; break;
                case 4: a = 14; s = 4; n = 2; m = 11; u = 5; afrmt = "%011lo"; break;
                }
                frmt = " %03o"; break;
                        }
#else
         switch(radix) {
         default:
         case 16:
                r = RAD16;
                switch(a_bytes) {
                default:
                case 2: a = 8; s = 3; n = 3; m = 4; u = 6; afrmt = "%04X"; break;
                case 3: a = 13; s = 3; n = 6; m = 6; u = 7; afrmt = "%06X"; break;
                case 4: a = 13; s = 3; n = 4; m = 8; u = 7; afrmt = "%08X"; break;
                }
                frmt = " %02X"; break;
        case 10:
                r = RAD10;
                switch(a_bytes) {
                default:
                case 2: a = 10; s = 4; n = 4; m = 5; u = 4; afrmt = "%05u"; break;
                case 3: a = 14; s = 4; n = 5; m = 8; u = 5; afrmt = "%08u"; break;
                case 4: a = 14; s = 4; n = 3; m = 10; u = 5; afrmt = "%010u"; break;
                }
                frmt = " %03u"; break;
        case 8:
                r = RAD8;
                switch(a_bytes) {
                default:
                case 2: a = 10; s = 4; n = 3; m = 6; u = 4; afrmt = "%06o"; break;
                case 3: a = 14; s = 4; n = 5; m = 8; u = 5; afrmt = "%08o"; break;
                case 4: a = 14; s = 4; n = 2; m = 11; u = 5; afrmt = "%011o"; break;
                }
                frmt = " %03o"; break;
        }
#endif
        /*
         * Data Byte Pointer
         */
        if (gcntr == -1) {
                rp = &rb[a];
        } else {
                rp = &rb[a + (s * gcntr)];
        }
        /*
         * Number must be of proper radix
         */
        if (!dgt(r, rp, s-1)) {
                fprintf(rfp, "%s", rb);
                gline = 1;
                goto loop;
        }
        /*
         * Output new data value, overwrite relocation codes
         */
        sprintf(str, frmt, v);
        strncpy(rp-1, str, s);
        if (gcntr == -1) {
                gcntr = 0;
        }
        /*
         * Output relocated code address
         */
        if (gcntr == 0) {
                if (dgt(r, &rb[n], m)) {
                        sprintf(str, afrmt, cpc);
                        strncpy(&rb[n], str, m);
                }
        }
        /*
         * Output an error line if required
         */
        if (err) {
                switch(ASxxxx_VERSION) {
                case 3:
                        fprintf(rfp, "?ASlink-Warning-%s\n", errmsg3[err]);
                        break;

                default:
                        break;
                }
        }
        /*
         * Fix 'u' if [nn], cycles, is specified
         */
        if (rb[a + (s*u) - 1] == CYCNT_END) {
                u -= 1;
        }
        /*
         * Output text line when updates finished
         */
        if (++gcntr == u) {
                fprintf(rfp, "%s", rb);
                gline = 1;
                gcntr = -1;
        }
}
