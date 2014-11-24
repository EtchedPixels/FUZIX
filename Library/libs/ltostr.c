/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

static char buf[34];


char * ultostr(unsigned long val, int radix) {
    register char *p;
    register int c;

    if( radix > 36 || radix < 2 ) return 0;

    p = buf+sizeof(buf);
    *--p = '\0';

    do {
        c = val%radix;
        val/=radix;
        if( c > 9 ) *--p = 'a'-10+c;
        else *--p = '0'+c;
    } while(val);
    return p;
}

char * ltostr(long val, int radix) {
    char *p;
    int flg = 0;
    if( val < 0 ) {
        flg++;
        val= -val;
    }
    p = ultostr(val, radix);
    if(p && flg) *--p = '-';
    return p;
}
