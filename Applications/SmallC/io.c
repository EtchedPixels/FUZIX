/*      File io.c: 2.1 (83/03/20,16:02:07) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "data.h"

/*
 *      open input file
 */
int openin(char *p)
{
        strcpy(fname, p);
        fixname (fname);
        if (!checkname (fname))
                return (NO);
        if ((input = open(fname, O_RDONLY)) == -1) {
                pl ("Open failure\n");
                return (NO);
        }
        do_kill();
        return (YES);

}

/*
 *      open output file
 */
int openout(void)
{
        outfname (fname);
        if ((output = open (fname, O_WRONLY|O_TRUNC|O_CREAT, 0644)) == -1) {
                pl ("Open failure");
                return (NO);
        }
        do_kill();
        return (YES);

}

/*
 *      change input filename to output filename
 */
void outfname(char *s)
{
        while (*s)
                s++;
        *--s = 's';

}

/**
 * remove NL from filenames
 */
void fixname(char *s)
{
        while (*s && *s++ != LF);
        if (!*s) return;
        *(--s) = 0;

}

/**
 * check that filename is "*.c"
 */
int checkname(char *s)
{
        while (*s)
                s++;
        if (*--s != 'c')
                return (NO);
        if (*--s != '.')
                return (NO);
        return (YES);

}

void do_kill(void) {
        lptr = 0;
        line[lptr] = 0;
}

int igetc(int unit)
{
        unsigned char c;
        int err;
        err = read(unit, &c, 1);
        if (err == 1)
                return (int)c;
        if (err == 0) {
                if (unit == input)
                        input_eof = 1;
                return -1;
        }
        writee("I/O error");
        exit(1);
}

void readline(void) {
        int k;
        int unit;

        FOREVER {
                if (input_eof)
                        return;
                if ((unit = input2) == -1)
                        unit = input;
                do_kill();
                while ((k = igetc(unit)) != EOF) {
                        if ((k == CR) || (k == LF) | (lptr >= LINEMAX))
                                break;
                        line[lptr++] = k;
                }
                line[lptr] = 0;
                if (k <= 0)
                        if (input2 != -1) {
                                input2 = inclstk[--inclsp];
                                close (unit);
                        }
                if (lptr) {
                        if ((ctext) & (cmode)) {
                                gen_comment ();
                                output_string (line);
                                newline ();
                        }
                        lptr = 0;
                        return;
                }
        }
}

int inbyte(void) {
        while (ch () == 0) {
                if (input_eof)
                        return (0);
                preprocess ();
        }
        return (gch ());
}

int inchar(void) {
        if (ch () == 0)
                readline ();
        if (input_eof)
                return (0);
        return (gch ());
}

/**
 * gets current char from input line and moves to the next one
 * @return current char
 */
int gch(void) {
        if (ch () == 0)
                return (0);
        else
                return (line[lptr++] & 127);
}

/**
 * returns next char
 * @return next char
 */
int nch (void) {
        if (ch () == 0)
                return (0);
        else
                return (line[lptr + 1] & 127);
}

/**
 * returns current char
 * @return current char
 */
int ch (void) {
        return (line[lptr] & 127);
}

/*
 *      print a carriage return and a string only to console
 *
 */
void pl (char *str)
{
        write(1, "\n", 1);
        write(1, str, strlen(str));
}


void writee(char *str)
{
        write(2, str, strlen(str));
}
