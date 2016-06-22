/*      Basic CP/M file I/O:
fopen,fclose,fgetc,fputc,feof

Original:       Paul Tarvydas
Fixed by:       Chris Lewis
*/
#include <stdio.h>

#define EOL 10
#define EOL2 13
#define CPMEOF 26
#define CPMERR 255
#define UNIT_OFFSET 3
#define CPMCIN 1
#define CPMCOUT 2
#define READ_EOF 3
#define SETDMA 26
#define DEFAULT_DMA 128
#define CPMREAD 20
#define CPMWR 21
#define WRITE 2
#define READ 1
#define FREE 0
#define NBUFFS 4
#define BUFSIZ 512
#define FCBSIZ 33
#define ALLBUFFS 2048
#define ALLFCBS 132
#define CPMERA 19
#define CPMCREAT 22
#define CPMOPEN 15
#define NBLOCKS 4
#define BLKSIZ 128
#define BKSP 8
#define CTRLU 21
#define FWSP ' '
#define CPMCLOSE 16

char    buffs[ALLBUFFS],        /* disk buffers */
fcbs[ALLFCBS];          /* fcbs for buffers */
int     bptr[NBUFFS];           /* ptrs into buffers */
int     modes[NBUFFS];          /* mode for each open file */
int     eptr[NBUFFS];           /* buffers' ends */
char eofstdin;  /* flag end of file on stdin */

fgetc(unit) int unit;
{
    int c;
    while ((c = Xfgetc(unit)) == EOL2);
    return c;

}

Xfgetc(unit) int unit;
{
    int i;
    int c;
    char *buff;
    char *fcba;
    if ((unit == stdin) & !eofstdin) {
        c = bdos1(CPMCIN, 0);
        if (c == 4) {
            eofstdin = 1;
            return (EOF);
        }
        else if (c == 3)
            exit (1);
        else {
            if (c == EOL2) {
                c = EOL;
                bdos (CPMCOUT, EOL);
            }
            return (c);
        }
    }
    if (modes[unit = unit - UNIT_OFFSET] == READ) {
        if (bptr[unit] >= eptr[unit]) {
            fcba = fcbaddr(unit);
            /* fill da buffer again */
            i = 0;  /* block counter */
            buff = buffaddr(unit); /* dma ptr */
            /* if buffer wasn't totally
                    filled last time, we already
                    eof */
            if (eptr[unit] == buffaddr(unit + 1))
            do {
                bdos(SETDMA, buff);
                if (0!=bdos1(CPMREAD, fcba))
                    break;
                buff = buff + BLKSIZ;
            }
            while (++i<NBLOCKS);
            bdos(SETDMA, DEFAULT_DMA);
            /* if i still 0, no blocks read =>eof*/
            if (i==0) {
                modes[unit] = READ_EOF;
                return EOF;
            }
            /* o.k. set start & end ptrs */
            eptr[unit] =
                (bptr[unit]=buffaddr(unit))
                + (i * BLKSIZ);
        }
        c = (*(bptr[unit]++)) & 0xff;
        if (c == CPMEOF) {
            c = EOF;
            modes[unit] = READ_EOF;
        }
        return c;
    }
    return EOF;

}

fclose(unit) int unit;
{
    int i;
    if ((unit==stdin)|(unit==stdout)|(unit==stderr))
        return NULL;
    if (modes[unit = unit - UNIT_OFFSET] != FREE) {
        if (modes[unit] == WRITE)
            fflush(unit + UNIT_OFFSET);
        modes[unit] = FREE;
        return bdos1(CPMCLOSE, fcbaddr(unit));
    }
    return EOF;

}

fflush(unit) int unit;
{
    char *buffa;
    char *fcba;
    if ((unit!=stdin)|(unit!=stdout)|(unit!=stderr)) {
        /* put an eof at end of file */
        fputc(CPMEOF, unit);
        if (bptr[unit = unit - UNIT_OFFSET] !=
            (buffa = buffaddr(unit))) {
            /* some chars in buffer - flush them */
            fcba = fcbaddr(unit);
            do {
                bdos(SETDMA, buffa);
                if (0 != bdos1(CPMWR, fcba))
                    return (EOF);
            }
            while (bptr[unit] >
                (buffa=buffa+BLKSIZ));
            bdos(SETDMA, DEFAULT_DMA);
        }
    }
    return NULL;

}

fputc(c, unit) char c;
int unit;
{
    char *buffa;
    char *fcba;
    if (c == EOL) fputc(EOL2, unit);
    if ((unit == stdout) | (unit == stderr)) {
        bdos(CPMCOUT, c);
        return c;
    }
    if (WRITE == modes[unit = unit - UNIT_OFFSET]) {
        if (bptr[unit] >= eptr[unit]) {
            /* no room - dump buffer */
            fcba = fcbaddr(unit);
            buffa=buffaddr(unit);
            while (buffa < eptr[unit]) {
                bdos(SETDMA, buffa);
                if (0 != bdos1(CPMWR, fcba)) break;
                buffa = buffa + BLKSIZ;
            }
            bdos(SETDMA, DEFAULT_DMA);
            bptr[unit] = buffaddr(unit);
            if (buffa < eptr[unit]) return EOF;
        }
        *(bptr[unit]++) = c;
        return c;
    }
    return EOF;

}

allocunitno() {
    int i;
    /* returns # of first free buffer, EOF if none */
    /* buffer is not reserved (ie. mode remains FREE) */
    for (i = 0; i < NBUFFS; ++i)
        if (modes[i] == FREE) break;
    if (i >= NBUFFS) return EOF;
    else return (i + UNIT_OFFSET);

}

fopen(name, mode) char *name, *mode;
{
    int fileno, fno2;
    if (EOF != (fileno = allocunitno())) {
        /* internal file # excludes units 0,1 & 2
                since there's no buffers associated with
                these units */
        movname(clearfcb(fcbaddr(fno2 = fileno
            - UNIT_OFFSET)), name);
        if ('r' == *mode) {
            if (bdos1(CPMOPEN, fcbaddr(fno2)) != CPMERR)
            {
                modes[fno2] = READ;
                /* ptr>bufsiz => buffer empty*/
                eptr[fno2] =
                    bptr[fno2] = buffaddr(fno2+1 <tel:+1>);
                return fileno;
            }
        }
        else if ('w' == *mode) {
            bdos(CPMERA, fcbaddr(fno2));
            if (bdos1(CPMCREAT, fcbaddr(fno2)) != CPMERR){
                modes[fno2] = WRITE;
                bptr[fno2] = buffaddr(fno2);
                eptr[fno2] = buffaddr(fno2+1 <tel:+1>);
                return fileno;
            }
        }
    }
    return NULL;

}

clearfcb(fcb) char fcb[];
{
    int i;
    for (i=0; i<FCBSIZ; fcb[i++] = 0);
    /* blank out name field */
    for (i=1; i<12; fcb[i++] = ' ');
    return fcb;

}

movname(fcb, str) char fcb[], *str;
{
    int i;
    char c;
    i = 1; /* first char of name @ pos 1 */
    *fcb = 0;
    if (':' == str[1]) {
        c = toupper(str[0]);
        if (('A' <= c) & ('B' >= c)) {
            *fcb = (c - 'A' + 1);
            str++;
            str++;
        }
    }
    while ((NULL != *str) & (i<9)) {
        /* up to 8 chars into file name field */
        if ('.' == *str) break;
        fcb[i++] = toupper(*str++);
    }
    /* strip off excess chars - up to '.' (beginning of
        extension name ) */
    while ((NULL != *str) & ((*str) != '.')) ++str;
    if (*str)
        /* '.' is first char of *str now */
        /* copy 3 chars of ext. if there */
        for (   /* first char of ext @ pos 9 (8+1 <tel:+1>)*/
i = 8;
/* '.' is stripped by ++ 1st time */
/* around */
(NULL != *++str) & (12 > ++i);
fcb[i] = toupper(*str)
);
        return fcb;

}

stdioinit() {
    int i;
    eofstdin = 0;
    for (i=0; i<NBUFFS; modes[i++] = FREE);

}

fcbaddr(unit) int unit;
{
    /* returns address of fcb associated with given unit -
        unit taken with origin 0 (ie. std's not included) */
    return &fcbs[unit * FCBSIZ];

}

buffaddr(unit) int unit;
{
    return &buffs[unit * BUFSIZ];

}

feof (unit) FILE *unit;
{
    if ((unit == stdin) & eofstdin)
        return 1;
    if (modes[unit - UNIT_OFFSET] == READ_EOF)
        return 1;
    return 0;
}

