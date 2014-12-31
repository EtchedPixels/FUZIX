/* lkout.c */

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
 *   With enhancements by
 *
 *      G. Osborn
 *      gary@s-4.com.
 */

#include "aslink.h"

/*)Module       lkout.c
 *
 *      The module lkout.c contains the dispatch
 *      function to create the relocated object
 *      code output in the required format.
 *
 *      lkout.c contains the following functions:
 *              VOID    lkout()
 *              VOID    lkflush()
 *              VOID    ixx()
 *              VOID    iflush()
 *              VOID    dbx()
 *              VOID    dflush()
 *
 *      lkout.c contains no local variables.
 */

/*)Function     lkout(i)
 *
 *              int     i               1 - process data
 *                                      0 - end of data
 *
 *      The function lkout() dispatches to the
 *      required output format routine.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              int     oflag           output type flag
 *              int     obj_flag        Output enabled flag
 *              FILE *  ofp             output file handle
 *              a_uint  pc              Current relocation address
 *              int     pcb             Current pc bytes per address
 *
 *      functions called:
 *              VOID    ixx()           lkout.c
 *              VOID    s19()           lks19.c
 *              VOID    dbx()           lkout.c
 *              VOID    elf()           lkelf.c
 *
 *      side effects:
 *              The REL data is output in the required format.
 */

VOID
lkout(int i, int b)
{
        int j;

        if (i && obj_flag) { return; }
        if (ofp == NULL)   { return; }

        /*
         * Create the Byte Output Address
         */
        for (j=1; j<pcb; j++) {
                adb_xb(pc, 0);
        }

        /*
         * Intel Formats
         */
        if (oflag == 1) {
                ixx(i, b);
        } else
        /*
         * Motorola Formats
         */
        if (oflag == 2) {
                s19(i);
        } else
        /*
         * Disk Basic Formats
         */
        if (oflag == 3) {
                dbx(i);
        } else
        /*
         * Elf Formats
         */
        if (oflag == 4) {
                elf(i);
        }
}


/*)Function     lkflush()
 *
 *      The function lkflush() dispatches
 *      to the required data flushing routine.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              int     oflag           output type flag
 *              FILE *  ofp             output file handle
 *
 *      functions called:
 *              VOID    iflush()        lkout.c
 *              VOID    sflush()        lks19.c
 *              VOID    dflush()        lkout.c
 *
 *      side effects:
 *              Any remaining REL data is flushed
 *              to the output file.
 */

VOID
lkflush()
{
        if (ofp == NULL)   { return; }

        /*
         * Intel Formats
         */
        if (oflag == 1) {
                iflush();
        } else
        /*
         * Motorola Formats
         */
        if (oflag == 2) {
                sflush();
        } else
        /*
         * Disk Basic Formats
         */
        if (oflag == 3) {
                dflush();
        }
}


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
 *                              address record this is the program entry
 *                              address (low), or in a segment record
 *                              this is the high order address.
 *
 *      Record Type Field    -  This  field  identifies the record type,
 *                              which is either 0 for data records,  1
 *                              for an End of File record, 3 for a
 *                              start address, or 4 for a
 *                              segment record.  It consists
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

/*)Function     ixx(i, b)
 *
 *              int     i               1 - process data
 *                                      0 - end of data
 *		int     b		- bank tag
 *
 *      The function ixx() loads the output buffer with
 *      the relocated data.
 *
 *      local variables:
 *              a_uint  chksum          byte checksum
 *              a_uint  lo_addr         address within segment
 *              a_uint  hi_addr         segment number
 *              int     i               loop counter
 *              a_uint  j               temporary
 *              int     k               loop counter
 *              struct sym *sp          symbol pointer
 *              a_uint  symadr          symbol address
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              int     hilo            byte order
 *              FILE *  ofp             output file handle
 *              int     rtaflg          first output flag
 *              int     rtcnt           count of data words
 *              int     rtflg[]         output the data flag
 *              a_uint  rtval[]         relocated data
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *              a_uint  rtadr2          address temporary
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    iflush()        lkout.c
 *
 *      side effects:
 *              The data is placed into the output buffer.
 */

/*
 * The number of Data Field bytes is:
 *
 *      1       Record Mark Field
 *      2       Record Length Field
 *      4       Load Address Field
 *      2       Record Type Field
 *      2       Checksum Field
 *
 *      Plus 32 data bytes (64 characters)
 */

static int i_lb = -1;

VOID
ixx(int i, int b)
{
        int k;
        struct sym *sp;
        a_uint j, symadr, chksum;

        if (i) {
                if (TARGET_IS_6808 && ap->a_flag & A_NOLOAD)
                        return;

                if (hilo == 0) {
                        switch(a_bytes){
                        default:
                        case 2:
                                j = rtval[0];
                                rtval[0] = rtval[1];
                                rtval[1] = j;
                                break;
                        case 3:
                                j = rtval[0];
                                rtval[0] = rtval[2];
                                rtval[2] = j;
                                break;
                        case 4:
                                j = rtval[0];
                                rtval[0] = rtval[3];
                                rtval[3] = j;
                                j = rtval[2];
                                rtval[2] = rtval[1];
                                rtval[1] = j;
                                break;
                        }
                }
                for (i=0,rtadr2=0; i<a_bytes; i++) {
                        rtadr2 = (rtadr2 << 8) | rtval[i];
                }
                if ((rtadr2 != rtadr1) || rtaflg || b != i_lb) {
                        /*
                         * data bytes not contiguous between records
                         */
                        iflush();
                        i_lb = b;
                        rtadr0 = rtadr1 = rtadr2;
                        rtaflg = 0;
                }
                for (k=a_bytes; k<rtcnt; k++) {
                        if (rtflg[k]) {
                                rtbuf[(int) (rtadr1++ - rtadr0)] = rtval[k];
                                if ((rtadr1 & 0xffff) == 0) {
                                        iflush();
                                }
                                if (rtadr1 - rtadr0 == IXXMAXBYTES) {
                                        iflush();
                                }
                        }
                }
        } else {
                sp = lkpsym(".__.END.", 0);
                if (sp && (sp->s_axp->a_bap->a_ofp == ofp)) {
                        symadr = symval(sp);
                        chksum =  0x04;
                        chksum += 0x05;
                        chksum += symadr;
                        chksum += symadr >> 8;
                        chksum += symadr >> 16;
                        chksum += symadr >> 24;
#ifdef  LONGINT
                        fprintf(ofp, ":04000005%08lX%02lX\n", symadr, (~chksum + 1) & 0x00ff);
#else
                        fprintf(ofp, ":04000005%08X%02X\n", symadr, (~chksum + 1) & 0x00ff);
#endif
                }

                fprintf(ofp, ":00000001FF\n");
        }
}


/*)Function     iflush()
 *
 *      The function iflush() outputs the relocated data
 *      in the standard Intel format.
 *
 *      local variables:
 *              a_uint  chksum          byte checksum
 *              a_uint  lo_addr         address within segment
 *              a_uint  hi_addr         segment number
 *              int     i               loop counter
 *              int     max             number of data bytes
 *              int     reclen          record length
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              FILE *  ofp             output file handle
 *              int     rtaflg          first output flag
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              The data is output to the file defined by ofp.
 */

/*
 * This function derived from the work
 * of G. Osborn, gary@s-4.com.
 * The new version concatenates the assembler
 * output records when they represent contiguous
 * memory segments to produce IXXMAXBYTES data byte
 * Intel Hex output lines whenever possible, resulting
 * in a substantial reduction in file size.
 * More importantly, the download time
 * to the target system is much improved.
 */

VOID
iflush(void)
{
        int i, max, reclen;
        a_uint chksum, lo_addr, hi_addr;

        max = (int) (rtadr1 - rtadr0);
        if (max) {
                if (a_bytes > 2) {
                        static a_uint prev_hi_addr = 0;

                        hi_addr = (rtadr0 >> 16) & 0xffff;
                        if ((hi_addr != prev_hi_addr) || rtaflg) {
                                chksum =  0x02;
                                chksum += 0x04;
                                chksum += hi_addr;
                                chksum += hi_addr >> 8;
#ifdef  LONGINT
                                fprintf(ofp, "%02X:02000004%04lX%02lX\n", i_lb, hi_addr, (~chksum + 1) & 0x00ff);
#else
                                fprintf(ofp, "%02X:02000004%04X%02X\n", i_lb, hi_addr, (~chksum + 1) & 0x00ff);
#endif
                                prev_hi_addr = hi_addr;
                        }
                }

                /*
                 * Only the ":" and the checksum itself are excluded
                 * from the checksum.  The record length includes
                 * only the data bytes.
                 */
                lo_addr = rtadr0 & 0xffff;
                reclen = max;
                chksum = reclen;
                chksum += lo_addr;
                chksum += lo_addr >> 8;
#ifdef  LONGINT
                fprintf(ofp, "%02X:%02X%04lX00", i_lb, reclen, lo_addr);
#else
                fprintf(ofp, "%02X:%02X%04X00", i_lb, reclen, lo_addr);
#endif
                for (i=0; i<max; i++) {
                        chksum += rtbuf[i];
                        fprintf(ofp, "%02X", rtbuf[i] & 0x00ff);
                }
                /*
                 * 2's complement
                 */
#ifdef  LONGINT
                fprintf(ofp, "%02lX\n", (~chksum + 1) & 0x00ff);
#else
                fprintf(ofp, "%02X\n", (~chksum + 1) & 0x00ff);
#endif
                rtadr0 = rtadr1;
        }

}

#if 0
/*)S19/S28/S37 Formats
 *      Record Type Field    -  This  field  signifies  the  start  of a
 *                              record and  identifies  the  the  record
 *                              type as follows:
 *
 *             2-Byte Address:      Ascii S1 - Data Record
 *                                  Ascii S9 - End of File Record
 *             3-Byte Address:      Ascii S2 - Data Record
 *                                  Ascii S8 - End of File Record
 *             4-Byte Address:      Ascii S3 - Data Record
 *                                  Ascii S7 - End of File Record
 *
 *      Record Length Field  -  This  field  specifies the record length
 *                              which includes the  address,  data,  and
 *                              checksum   fields.   The  8  bit  record
 *                              length value is converted to  two  ascii
 *                              characters, high digit first.
 *
 *      Load Address Field   -  This  field  consists  of the 4/6/8 ascii
 *                              characters which result from  converting
 *                              the  the  binary value of the address in
 *                              which to begin loading this record.  The
 *                              order is as follows:
 *
 *           S37:                   High digit of fourth byte of address.
 *                                  Low digit of fourth byte of address.
 *           S28/S37:               High digit of third byte of address.
 *                                  Low digit of third byte of address.
 *           S19/S28/S37:           High digit of high byte of address.
 *                                  Low digit of high byte of address.
 *                                  High digit of low byte of address.
 *                                  Low digit of low byte of address.
 *
 *                              In an End of File record this field con-
 *                              sists of either 4/6/8 ascii zeros or  the
 *                              program  entry  address.
 *
 *      Data Field           -  This  field consists of the actual data,
 *                              converted to two ascii characters,  high
 *                              digit first.  There are no data bytes in
 *                              the End of File record.
 *
 *      Checksum Field       -  The  checksum  field is the 8 bit binary
 *                              sum of the record length field, the load
 *                              address field, and the data field.  This
 *                              sum is then  complemented  (1's  comple-
 *                              ment)   and   converted   to  two  ascii
 *                              characters, high digit first.
 */

/*)Function     sxx(i)
 *
 *              int     i               1 - process data
 *                                      0 - end of data
 *
 *      The function s19() loads the output buffer with
 *      the relocated data.
 *
 *      local variables:
 *              a_uint  addr            address temporary
 *              a_uint  chksum          byte checksum
 *              char *  frmt            format string pointer
 *              int     i               loop counter
 *              a_uint  j               temporary
 *              int     k               loop counter
 *              int     max             number of data bytes
 *              int     reclen          record length
 *              struct sym *sp          symbol pointer
 *              a_uint  symadr          symbol address
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              int     hilo            byte order
 *              FILE *  ofp             output file handle
 *              int     rtcnt           count of data words
 *              int     rtflg[]         output the data flag
 *              a_uint  rtval[]         relocated data
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *              a_uint  rtadr2          address temporary
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    sflush()        lkout.c
 *
 *      side effects:
 *              The data is placed into the output buffer.
 */

/*
 * Number of Data Field bytes is:
 *
 *      2       Record Type Field
 *      2       Record Length Field
 *      4/6/8   Load Address Field
 *      2       Checksum Field
 *
 *      Plus 32 data bytes (64 characters)
 */

VOID
sxx(i)
int i;
{
        struct sym *sp;
        char *frmt;
        int k, reclen;
        a_uint  j, addr, symadr, chksum;

        if (i) {
                if (hilo == 0) {
                        switch(a_bytes){
                        default:
                        case 2:
                                j = rtval[0];
                                rtval[0] = rtval[1];
                                rtval[1] = j;
                                break;
                        case 3:
                                j = rtval[0];
                                rtval[0] = rtval[2];
                                rtval[2] = j;
                                break;
                        case 4:
                                j = rtval[0];
                                rtval[0] = rtval[3];
                                rtval[3] = j;
                                j = rtval[2];
                                rtval[2] = rtval[1];
                                rtval[1] = j;
                                break;
                        }
                }
                for (i=0,rtadr2=0; i<a_bytes; i++) {
                        rtadr2 = (rtadr2 << 8) | rtval[i];
                }

                if (rtadr2 != rtadr1) {
                        /*
                         * data bytes not contiguous between records
                         */
                        sflush();
                        rtadr0 = rtadr1 = rtadr2;
                }
                for (k=a_bytes; k<rtcnt; k++) {
                        if (rtflg[k]) {
                                rtbuf[(int) (rtadr1++ - rtadr0)] = rtval[k];
                                if (rtadr1 - rtadr0 == SXXMAXBYTES) {
                                        sflush();
                                }
                        }
                }
        } else {
                /*
                 * Only the "S_" and the checksum itself are excluded
                 * from the checksum.  The record length does not
                 * include "S_" and the pair count.  It does
                 * include the address bytes, the data bytes,
                 * and the checksum.
                 */
                reclen = 1 + a_bytes;
                chksum = reclen;
                sp = lkpsym(".__.END.", 0);
                if (sp && (sp->s_axp->a_bap->a_ofp == ofp)) {
                        symadr = symval(sp);
                        for (i=0,addr=symadr; i<a_bytes; i++,addr>>=8) {
                                chksum += addr;
                        }
                } else {
                        symadr = 0;
                }
#ifdef  LONGINT
                switch(a_bytes) {
                default:
                case 2: frmt = "S9%02X%04lX"; addr = symadr & 0x0000ffffl; break;
                case 3: frmt = "S8%02X%06lX"; addr = symadr & 0x00ffffffl; break;
                case 4: frmt = "S7%02X%08lX"; addr = symadr & 0xffffffffl; break;
                }
#else
                switch(a_bytes) {
                default:
                case 2: frmt = "S9%02X%04X"; addr = symadr & 0x0000ffff; break;
                case 3: frmt = "S8%02X%06X"; addr = symadr & 0x00ffffff; break;
                case 4: frmt = "S7%02X%08X"; addr = symadr & 0xffffffff; break;
                }
#endif
                fprintf(ofp, frmt, reclen, addr);
                /*
                 * 1's complement
                 */
#ifdef  LONGINT
                fprintf(ofp, "%02lX\n", (~chksum) & 0x00ff);
#else
                fprintf(ofp, "%02X\n", (~chksum) & 0x00ff);
#endif
        }
}


/*)Function     sflush()
 *
 *      The function sflush() outputs the relocated data
 *      in the standard Motorola format.
 *
 *      local variables:
 *              a_uint  addr            address temporary
 *              a_uint  chksum          byte checksum
 *              char *  frmt            format string pointer
 *              int     i               loop counter
 *              int     max             number of data bytes
 *              int     reclen          record length
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              FILE *  ofp             output file handle
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              The data is output to the file defined by ofp.
 */

/*
 * Written by G. Osborn, gary@s-4.com, 6-17-98.
 * The new version concatenates the assembler
 * output records when they represent contiguous
 * memory segments to produce SXXMAXBYTES data byte
 * S_ output lines whenever possible, resulting
 * in a substantial reduction in file size.
 * More importantly, the download time
 * to the target system is much improved.
 */

VOID
sflush()
{
        char *frmt;
        int i, max, reclen;
        a_uint  addr, chksum;

        max = (int) (rtadr1 - rtadr0);
        if (max == 0) {
                return;
        }

        /*
         * Only the "S_" and the checksum itself are excluded
         * from the checksum.  The record length does not
         * include "S_" and the pair count.  It does
         * include the address bytes, the data bytes,
         * and the checksum.
         */
        reclen = max + 1 + a_bytes;
        chksum = reclen;
        for (i=0,addr=rtadr0; i<a_bytes; i++,addr>>=8) {
                chksum += addr;
        }
#ifdef  LONGINT
        switch(a_bytes) {
        default:
        case 2: frmt = "S1%02X%04lX"; addr = rtadr0 & 0x0000ffffl; break;
        case 3: frmt = "S2%02X%06lX"; addr = rtadr0 & 0x00ffffffl; break;
        case 4: frmt = "S3%02X%08lX"; addr = rtadr0 & 0xffffffffl; break;
        }
#else
        switch(a_bytes) {
        default:
        case 2: frmt = "S1%02X%04X"; addr = rtadr0 & 0x0000ffff; break;
        case 3: frmt = "S2%02X%06X"; addr = rtadr0 & 0x00ffffff; break;
        case 4: frmt = "S3%02X%08X"; addr = rtadr0 & 0xffffffff; break;
        }
#endif
        fprintf(ofp, frmt, reclen, addr);
        for (i=0; i<max; i++) {
                chksum += rtbuf[i];
                fprintf(ofp, "%02X", rtbuf[i] & 0x00ff);
        }
        /*
         * 1's complement
         */
#ifdef  LONGINT
        fprintf(ofp, "%02lX\n", (~chksum) & 0x00ff);
#else
        fprintf(ofp, "%02X\n", (~chksum) & 0x00ff);
#endif
        rtadr0 = rtadr1;
}
#endif

/*)Disk BASIC Format
 *
 * Each code segment starts with the following record:
 *
 *      Record Preamble      -  This field is either $00 (for start of new
 *                              record) or $FF (for last record in file).
 *
 *      Record Length Field  -  This field specifies the record length
 *                              that follows the Load Address Field.
 *
 *           16-Bit Length   -  2-bytes
 *           24-Bit Length   -  3-bytes
 *           32-Bit Length   -  4-bytes
 *
 *      Load Address Field   -  This field consists of the address where
 *                              the record will be loaded into memory.
 *
 *           16-Bit Address  -  2-bytes
 *           24-Bit Address  -  3-bytes
 *           32-Bit Address  -  4-bytes
 *
 *      Binary Data Bytes    -  Record Length data bytes.
 *
 * After the last code segment, a final record like the one above is
 * placed.  In this final segment, the Record Preamble is $FF, the
 * Record Length Field is $0000 and the Load Adress Field is the
 * execution address.
 */

/*)Function     dbx(i)
 *
 *              int     i               1 - process data
 *                                      0 - end of data
 *
 *      The function decb() loads the output buffer with
 *      the relocated data.
 *
 *      local variables:
 *              int     k               loop counter
 *              struct sym *sp          symbol pointer
 *              a_uint  symadr          start address
 *
 *      global variables:
 *              int     a_bytes         T Line Address Bytes
 *              FILE *  ofp             output file handle
 *              int     rtcnt           count of data words
 *              int     rtflg[]         output the data flag
 *              a_uint  rtval[]         relocated data
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *              a_uint  rtadr2          address temporary
 *
 *      functions called:
 *              int     putc()          c_library
 *              VOID    dflush()        lkout.c
 *
 *      side effects:
 *              The data is placed into the output buffer.
 */

VOID
dbx(i)
int i;
{
        struct sym *sp;
        int k;
        a_uint  j, symadr;

        if (i) {
                if (hilo == 0) {
                        switch(a_bytes){
                        default:
                        case 2:
                                j = rtval[0];
                                rtval[0] = rtval[1];
                                rtval[1] = j;
                                break;
                        case 3:
                                j = rtval[0];
                                rtval[0] = rtval[2];
                                rtval[2] = j;
                                break;
                        case 4:
                                j = rtval[0];
                                rtval[0] = rtval[3];
                                rtval[3] = j;
                                j = rtval[2];
                                rtval[2] = rtval[1];
                                rtval[1] = j;
                                break;
                        }
                }
                for (i=0,rtadr2=0; i<a_bytes; i++) {
                        rtadr2 = (rtadr2 << 8) | rtval[i];
                }

                if (rtadr2 != rtadr1) {
                        /*
                         * data bytes not contiguous between records
                         */
                        dflush();
                        rtadr0 = rtadr1 = rtadr2;
                }
                for (k=a_bytes; k<rtcnt; k++) {
                        if (rtflg[k]) {
                                rtbuf[(int) (rtadr1++ - rtadr0)] = rtval[k];
                                if (rtadr1 - rtadr0 == (unsigned) (DBXMAXBYTES - (2 * a_bytes) - 1)) {
                                        dflush();
                                }
                        }
                }
        } else {
                /* Disk BASIC BIN Trailer */
                sp = lkpsym(".__.END.", 0);
                if (sp && (sp->s_axp->a_bap->a_ofp == ofp)) {
                        symadr = symval(sp);
                } else {
                        symadr = 0;
                }
                /* Terminator */
                putc(0xFF, ofp);

                /* Size (0) */
                switch(a_bytes) {
                case 4: putc((int) (0 >> 24) & 0xFF, ofp);
                case 3: putc((int) (0 >> 16) & 0xFF, ofp);
                default:
                case 2: putc((int) (0 >>  8) & 0xFF, ofp);
                        putc((int) (0 >>  0) & 0xFF, ofp);
                        break;
                }

                /* Starting Address */
                switch(a_bytes) {
                case 4: putc((int) (symadr >> 24) & 0xFF, ofp);
                case 3: putc((int) (symadr >> 16) & 0xFF, ofp);
                default:
                case 2: putc((int) (symadr >>  8) & 0xFF, ofp);
                        putc((int) (symadr >>  0) & 0xFF, ofp);
                        break;
                }
        }
}


/*)Function     dflush()
 *
 *      The function dflush() outputs the relocated data
 *      in the Disk BASIC loadable format
 *
 *      local variables:
 *              int     i               loop counter
 *              int     max             number of data bytes
 *
 *      global variables:
 *              FILE *  ofp             output file handle
 *              char    rtbuf[]         output buffer
 *              a_uint  rtadr0          address temporary
 *              a_uint  rtadr1          address temporary
 *
 *      functions called:
 *              int     putc()          c_library
 *
 *      side effects:
 *              The data is output to the file defined by ofp.
 */

/*
 * Written by Boisy G. Pitre, boisy@boisypitre.com, 6-7-04
 */

VOID
dflush()
{
        int i, max;

        max = (int) (rtadr1 - rtadr0);
        if (max == 0) {
                return;
        }

        /* Preamble Byte */
        putc(0, ofp);

        /* Record Size */
        switch(a_bytes){
        case 4: putc((int) (max >> 24) & 0xFF, ofp);
        case 3: putc((int) (max >> 16) & 0xFF, ofp);
        default:
        case 2: putc((int) (max >>  8) & 0xFF, ofp);
                putc((int) (max >>  0) & 0xFF, ofp);
                break;
        }

        /* Load Address */
        switch(a_bytes){
        case 4: putc((int) (rtadr0 >> 24) & 0xFF, ofp);
        case 3: putc((int) (rtadr0 >> 16) & 0xFF, ofp);
        default:
        case 2: putc((int) (rtadr0 >>  8) & 0xFF, ofp);
                putc((int) (rtadr0 >>  0) & 0xFF, ofp);
                break;
        }

        for (i = 0; i < max; i++) {
                putc(rtbuf[i], ofp);
        }

        rtadr0 = rtadr1;
}
