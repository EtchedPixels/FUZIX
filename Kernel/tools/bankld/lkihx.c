/* lkihx.c

   Copyright (C) 1989-1995 Alan R. Baldwin
   721 Berkeley St., Kent, Ohio 44240

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
#include "sdld.h"
#include "aslink.h"

/*)Module   lkihx.c
 *
 *  The module lkihx.c contains the function to
 *  output the relocated object code in the
 *  Intel Hex format.
 *
 *  lkihx.c contains the following functions:
 *      VOID    hexRecord(addr, rtvalIndex)
 *      VOID    ihx(i)
 *      VOID    ihxExtendedLinearAddress(a)
 *
 *  local variables: hexPageOverrun, lastHexAddr
 */

/*Intel Format
 *      Record Mark Field    -  This  field  signifies  the  start  of a
 *                              record, and consists of an  ascii  colon
 *                              (:).
 *
 *      Record Length Field  -  This   field   consists   of  two  ascii
 *                              characters which indicate the number  of
 *                              data   bytes   in   this   record.   The
 *                              characters are the result of  converting
 *                              the  number  of  bytes  in binary to two
 *                              ascii characters, high digit first.   An
 *                              End  of  File  record contains two ascii
 *                              zeros in this field.
 *
 *      Load Address Field   -  This  field  consists  of the four ascii
 *                              characters which result from  converting
 *                              the  the  binary value of the address in
 *                              which to begin loading this record.  The
 *                              order is as follows:
 *
 *                                  High digit of high byte of address.
 *                                  Low digit of high byte of address.
 *                                  High digit of low byte of address.
 *                                  Low digit of low byte of address.
 *
 *                              In an End of File record this field con-
 *                              sists of four ascii zeros, in a start
 *				address record this is the program entry
 *				address (low), or in a segment record
 *				this is the high order address.
 *
 *      Record Type Field    -  This  field  identifies the record type,
 *                              which is either 0 for data records,  1
 *                              for an End of File record, 3 for a
 *				start address, or 4 for a
 *				segment record.  It consists
 *                              of two ascii characters, with  the  high
 *                              digit of the record type first, followed
 *                              by the low digit of the record type.
 *
 *      Data Field           -  This  field consists of the actual data,
 *                              converted to two ascii characters,  high
 *                              digit first.  There are no data bytes in
 *                              the End of File record.
 *
 *      Checksum Field       -  The  checksum  field is the 8 bit binary
 *                              sum of the record length field, the load
 *                              address  field,  the  record type field,
 *                              and the data field.  This  sum  is  then
 *                              negated  (2's  complement) and converted
 *                              to  two  ascii  characters,  high  digit
 *                              first.
 */

/* Static variable which holds the count of hex page overruns
 * (crossings of the 64kB boundary). Cleared at explicit extended
 * address output.
 */
static int hexPageOverrun = 0;

/* Global which holds the last (16 bit) address of hex record.
 * Cleared at begin of new area or when the extended address is output.
 */
unsigned int lastHexAddr = 0;


/*)Function hexRecord(addr, rtvalIndex)
 *
 *      unsigned addr       starting address of hex record
 *      int rtvalIndex      starting index into the rtval[] array
 *
 *  The function hexRecord() outputs the relocated data
 *  in the standard Intel Hex format (with inserting
 *  the extended address record if necessary).
 *
 *  local variables:
 *      a_uint  chksum      byte checksum
 *      int     i           index for loops
 *      int     overrun     temporary storage for hexPageOverrun
 *      int     bytes       counter for bytes written
 *
 *  global variables:
 *      FILE *  ofp         output file handle
 *      int     rtcnt       count of data words
 *      int     rtflg[]     output the data flag
 *      a_uint  rtval[]     relocated data
 *
 *  functions called:
 *      int     fprintf()           c_library
 *      ihxExtendedLinearAddress()  lkihx.c
 *      hexRecord()                 lkihx.c     (recursion)
 *
 *  side effects:
 *      hexPageOverrun is eventually incremented,
 *      lastHexAddr is updated
 */

VOID
hexRecord(unsigned addr, int rtvalIndex)
{
    a_uint chksum;
    int i, overrun, bytes;

    for (i = rtvalIndex, chksum = 0; i < rtcnt; i++) {
        if (rtflg[i]) {
            if (addr + ++chksum > 0xffff)
                break;
        }
    }
    if (chksum == 0)
        return;         // nothing to output

    /* Is this record in the same bank as previous? */
    if ((TARGET_IS_8051 && ((lastHexAddr>>16) != (addr>>16)) && (rflag)) ||
        (TARGET_IS_6808 && lastHexAddr > addr) {
        overrun = hexPageOverrun + 1;
        ihxExtendedLinearAddress(lastExtendedAddress + overrun);
        hexPageOverrun = overrun;
        hexRecord(addr, rtvalIndex);
        return;
    }

    lastHexAddr = addr;
    fprintf(ofp, "%02X:%02X%04X00", addr >>16, chksum, addr);
    chksum += (addr >> 8) + (addr & 0xff);
    for (i = rtvalIndex, bytes = 0; i < rtcnt; i++) {
        if (rtflg[i]) {
            fprintf(ofp, "%02X", rtval[i]);
            chksum += rtval[i];
            if (TARGET_IS_8051) {
                if (addr + ++bytes > 0xffff) {
                    if (rflag) {
                        fprintf(ofp, "%02X\n", (0-chksum) & 0xff);
                        overrun = hexPageOverrun + 1;
                        ihxExtendedLinearAddress(lastExtendedAddress + overrun);
                        hexPageOverrun = overrun;
                        hexRecord(0, i + 1);
                        return;
                    } else {
                        fprintf(stderr,
                            "warning: extended linear address encountered; "
                            "you probably want the -r flag.\n");
                    }
                }
            }
        }
    }
    fprintf(ofp, "%02X\n", (0-chksum) & 0xff);
}

/*)Function ihx(i)
 *
 *      int     i           0 - process data
 *                          1 - end of data
 *
 *  The function ihx() calls the hexRecord() function for processing data
 *  or writes the End of Data record to the file defined by ofp.
 *
 *  local variables:
 *      a_uint  n           auxiliary variable
 *
 *  global variables:
 *      int     hilo        byte order
 *      FILE *  ofp         output file handle
 *      a_uint  rtval[]     relocated data
 *
 *  functions called:
 *      VOID hexRecord()    lkihx.c
 *      int fprintf()       c_library
 *
 *  side effects:
 *      The sequence of rtval[0], rtval[1] is eventually changed.
 */

VOID
ihx(int i)
{
    a_uint n;
    if (i) {
        if (TARGET_IS_6808 && ap->a_flag & A_NOLOAD)
            return;

        if (hilo == 0) {
            n = rtval[0];
            rtval[0] = rtval[1];
            rtval[1] = n;
        }
        hexRecord((rtval[0]<<8) + rtval[1], 2);
    } else {
        if (TARGET_IS_8051 && rflag) /* linear start address, hardcoded as reset vector (0x0000) */
            fprintf(ofp, ":0400000500000000F7\n");
        fprintf(ofp, ":00000001FF\n");
    }
}

/*)Function ihxNewArea(i)
 * The function ihxNewArea() is called when processing of new area is started.
 * It resets the value of lastHexAddr.
 */

VOID
ihxNewArea()
{
    lastHexAddr = 0;
}

/*)Function ihxExtendedLinearAddress(i)
 *
 *      a_uint  i           16 bit extended linear address.
 *
 *  The function ihxExtendedLinearAddress() writes an extended
 *  linear address record (type 04) to the output file.
 *
 *  local variables:
 *      a_uint  chksum      byte checksum
 *
 *  global variables:
 *      FILE *  ofp         output file handle
 *
 *  functions called:
 *      int     fprintf()   c_library
 *
 *  side effects:
 *      The data is output to the file defined by ofp.
 *      hexPageOverrun and lastHexAddr is cleared
 */
VOID
ihxExtendedLinearAddress(a_uint a)
{
    a_uint  chksum;

    /* The checksum is the complement of the bytes in the
     * record: the 2 is record length, 4 is the extended linear
     * address record type, plus the two address bytes.
     */
    chksum = 2 + 4 + (a & 0xff) + ((a >> 8) & 0xff);

    fprintf(ofp, ":02000004%04X%02X\n", a & 0xffff, (0-chksum) & 0xff);
    hexPageOverrun = 0;
    lastHexAddr = 0;
}
