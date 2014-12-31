/* lkrloc.c */

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
 *   With enhancements from:
 *
 *      John L. Hartman (JLH)
 *      jhartman@compuserve.com
 *
 *      Bill McKinnon (BM)
 *      w_mckinnon@conknet.com
 */

#include <ctype.h>
#include "aslink.h"

/*)Module       lkrloc.c
 *
 *      The module lkrloc.c contains the functions which
 *      perform the relocation calculations.
 *
 *      lkrloc.c contains the following functions:
 *              a_uint  adb_1b()
 *              a_uint  adb_2b()
 *              a_uint  adb_3b()
 *              a_uint  adb_4b()
 *              a_uint  adb_xb()
 *              a_uint  evword()
 *              VOID    prntval()
 *              VOID    reloc()
 *
 *
 */

/*)Function VOID        reloc(c)
 *
 *                      int c           process code
 *
 *      The function reloc() calls the proper version
 *      of the linker code.
 *
 *      local variable:
 *              none
 *
 *      global variables:
 *              ASxxxx_VERSION          ASxxxx REL file version
 *
 *      called functions:
 *              VOID    reloc3()        lkrloc3.c
 *              VOID    reloc4()        lkrloc4.c
 *
 *      side effects:
 *              Refer to the called relocation functions.
 *
 */

VOID
reloc(int c)
{
        switch(ASxxxx_VERSION) {
        case 3:
                reloc3(c);
                break;

//      case 4:
//              reloc4(c);
//              break;

        default:
                fprintf(stderr, "Internal Version Error");
                lkexit(ER_FATAL);
                break;
        }
}


/*)Function a_uint              evword()
 *
 *      The function evword() combines two byte values
 *      into a single word value.
 *
 *      local variable:
 *              a_uint  v               temporary evaluation variable
 *
 *      global variables:
 *              hilo                    byte ordering parameter
 *
 *      called functions:
 *              int eval()              lkeval.c
 *
 *      side effects:
 *              Relocation text line is scanned to combine
 *              two byte values into a single word value.
 *
 */

a_uint
evword(void)
{
        a_uint v;

        if (hilo) {
                v =  (eval() << 8);
                v +=  eval();
        } else {
                v =   eval();
                v += (eval() << 8);
        }
        return(v);
}

/*)Function a_uint              adb_1b(v, i)
 *
 *              a_uint v        value to add to byte
 *              int i           rtval[] index
 *
 *      The function adb_1b() adds the value of v to
 *      the single byte value contained in rtval[i].
 *      The new value of rtval[i] is returned.
 *
 *      local variable:
 *              a_uint  j               temporary evaluation variable
 *
 *      global variables:
 *              none
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              The byte value of rtval[] is changed.
 *
 */

a_uint
adb_1b(a_uint v, int i)
{
        a_uint j;

        j = v + rtval[i];
        rtval[i] = j & ((a_uint) 0x000000FF);

        return(j);
}

/*)Function     a_uint  adb_2b(v, i)
 *
 *              a_uint  v               value to add to word
 *              int     i               rtval[] index
 *
 *      The function adb_2b() adds the value of v to the
 *      2 byte value contained in rtval[i] and rtval[i+1].
 *      The new value of rtval[i] / rtval[i+1] is returned.
 *
 *      local variable:
 *              a_uint  j               temporary evaluation variable
 *
 *      global variables:
 *              hilo                    byte ordering parameter
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              The 2 byte value of rtval[] is changed.
 *
 */

a_uint
adb_2b(a_uint v, int i)
{
        a_uint j;

        if (hilo) {
                j = v + (rtval[i+0] << 8) +
                        (rtval[i+1] << 0);
                rtval[i+0] = (j >> 8) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >> 0) & ((a_uint) 0x000000FF);
        } else {
                j = v + (rtval[i+0] << 0) +
                        (rtval[i+1] << 8);
                rtval[i+0] = (j >> 0) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >> 8) & ((a_uint) 0x000000FF);
        }
        return(j);
}

/*)Function     a_uint  adb_3b(v, i)
 *
 *              a_uint  v               value to add to word
 *              int     i               rtval[] index
 *
 *      The function adb_3b() adds the value of v to the
 *      three byte value contained in rtval[i], rtval[i+1], and rtval[i+2].
 *      The new value of rtval[i] / rtval[i+1] / rtval[i+2] is returned.
 *
 *      local variable:
 *              a_uint  j               temporary evaluation variable
 *
 *      global variables:
 *              hilo                    byte ordering parameter
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              The 3 byte value of rtval[] is changed.
 *
 */

a_uint
adb_3b(a_uint v, int i)
{
        a_uint j;

        if (hilo) {
                j = v + (rtval[i+0] << 16) +
                        (rtval[i+1] <<  8) +
                        (rtval[i+2] <<  0);
                rtval[i+0] = (j >> 16) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >>  8) & ((a_uint) 0x000000FF);
                rtval[i+2] = (j >>  0) & ((a_uint) 0x000000FF);
        } else {
                j = v + (rtval[i+0] <<  0) +
                        (rtval[i+1] <<  8) +
                        (rtval[i+2] << 16);
                rtval[i+0] = (j >>  0) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >>  8) & ((a_uint) 0x000000FF);
                rtval[i+2] = (j >> 16) & ((a_uint) 0x000000FF);
        }
        return(j);
}

/*)Function     a_uint  adb_4b(v, i)
 *
 *              a_uint  v               value to add to word
 *              int     i               rtval[] index
 *
 *      The function adb_4b() adds the value of v to the
 *      four byte value contained in rtval[i], ..., rtval[i+3].
 *      The new value of rtval[i], ...,  rtval[i+3] is returned.
 *
 *      local variable:
 *              a_uint  j               temporary evaluation variable
 *
 *      global variables:
 *              hilo                    byte ordering parameter
 *
 *      called functions:
 *              none
 *
 *      side effects:
 *              The 4 byte value of rtval[] is changed.
 *
 */

a_uint
adb_4b(a_uint v, int i)
{
        a_uint j;

        if (hilo) {
                j = v + (rtval[i+0] << 24) +
                        (rtval[i+1] << 16) +
                        (rtval[i+2] <<  8) +
                        (rtval[i+3] <<  0);
                rtval[i+0] = (j >> 24) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >> 16) & ((a_uint) 0x000000FF);
                rtval[i+2] = (j >>  8) & ((a_uint) 0x000000FF);
                rtval[i+3] = (j >>  0) & ((a_uint) 0x000000FF);
        } else {
                j = v + (rtval[i+0] <<  0) +
                        (rtval[i+1] <<  8) +
                        (rtval[i+2] << 16) +
                        (rtval[i+3] << 24);
                rtval[i+0] = (j >>  0) & ((a_uint) 0x000000FF);
                rtval[i+1] = (j >>  8) & ((a_uint) 0x000000FF);
                rtval[i+2] = (j >> 16) & ((a_uint) 0x000000FF);
                rtval[i+3] = (j >> 24) & ((a_uint) 0x000000FF);
        }
        return(j);
}

/*)Function     a_uint  adb_xb(v, i)
 *
 *              a_uint  v               value to add to x-bytes
 *              int     i               rtval[] index
 *
 *      The function adb_xb() adds the value of v to
 *      the value contained in rtval[i] for x-bytes.
 *      The new value of rtval[i] for x-bytes is returned.
 *
 *      local variable:
 *              none
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *
 *      called functions:
 *              a_uint  adb_1b()        lkrloc.c
 *              a_uint  adb_2b()        lkrloc.c
 *              a_uint  adb_3b()        lkrloc.c
 *              a_uint  adb_4b()        lkrloc.c
 *
 *      side effects:
 *              The x-byte value of rtval[] is changed.
 *
 */

a_uint
adb_xb(a_uint v, int i)
{
        a_uint j;

        switch(a_bytes){
        case 1:
                j = adb_1b(v, i);
                j = (j & ((a_uint) 0x00000080) ? j | ~((a_uint) 0x0000007F) : j & ((a_uint) 0x0000007F));
                break;
        case 2:
                j = adb_2b(v, i);
                j = (j & ((a_uint) 0x00008000) ? j | ~((a_uint) 0x00007FFF) : j & ((a_uint) 0x00007FFF));
                break;
        case 3:
                j = adb_3b(v, i);
                j = (j & ((a_uint) 0x00800000) ? j | ~((a_uint) 0x007FFFFF) : j & ((a_uint) 0x007FFFFF));
                break;
        case 4:
                j = adb_4b(v, i);
                j = (j & ((a_uint) 0x80000000) ? j | ~((a_uint) 0x7FFFFFFF) : j & ((a_uint) 0x7FFFFFFF));
                break;
        default:
                j = 0;
                break;
        }
        return(j);
}

/*)Function VOID prntval(fptr, v)
 *
 *              FILE    *fptr           output file handle
 *              a_uint  v               value to output
 *
 *      The function prntval() outputs the value v, in the
 *      currently selected radix, to the device specified
 *      by fptr.
 *
 *      local variable:
 *              none
 *
 *      global variables:
 *              int xflag               current radix
 *
 *      called functions:
 *              int fprintf()   c_library
 *
 *      side effects:
 *              none
 *
 */

VOID
prntval(FILE *fptr, a_uint v)
{
        char *frmt;

        switch(xflag) {
        default:
        case 0:
                switch(a_bytes) {
                default:
                case 2: frmt = "       %04X\n"; break;
                case 3: frmt = "     %06X\n"; break;
                case 4: frmt = "   %08X\n"; break;
                }
                break;
        case 1:
                switch(a_bytes) {
                default:
                case 2: frmt = "     %06o\n"; break;
                case 3: frmt = "   %08o\n"; break;
                case 4: frmt = "%011o\n"; break;
                }
                break;
        case 2:
                switch(a_bytes) {
                default:
                case 2: frmt = "      %05u\n"; break;
                case 3: frmt = "   %08u\n"; break;
                case 4: frmt = " %010u\n"; break;
                }
                break;
        }
        fprintf(fptr, frmt, v & a_mask);
}
