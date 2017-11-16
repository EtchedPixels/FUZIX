/*      File preproc.c: 2.3 (84/11/27,11:47:40) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "data.h"

/**
 * remove "brackets" surrounding include file name
 * @see DEFLIB
 */
int fix_include_name (void) {
        char    c1, c2, *p, *ibp;
        char buf[20];
        int fp;
        char buf2[100];

        ibp = &buf[0];

        if ((c1 = gch ()) != '"' && c1 != '<')
                return -1;
        for (p = line + lptr; *p ;)
                *ibp++ = *p++;
        c2 = *(--p);
        if (c1 == '"' ? (c2 != '"') : (c2 != '>')) {
                error ("incorrect delimiter");
                return -1;
        }
        *(--ibp) = 0;
        fp = -1;
        if (c1 == '<' || ((fp = open(buf, O_RDONLY)) == -1)) {
#ifndef __linux__
                strcpy(buf2, DEFLIB);
                strlcat(buf2, buf, sizeof(buf2));
#else
                snprintf(buf2, sizeof(buf2), "%s%s", DEFLIB, buf);
#endif
                fp = open(buf2, O_RDONLY);
        }
        return (fp);
}

/**
 * open an include file
 */
void doinclude(void)
{
        int     inp2;

        blanks ();
        if ((inp2 = fix_include_name ()) != -1)
                if (inclsp < INCLSIZ) {
                        inclstk[inclsp++] = input2;
                        input2 = inp2;
                } else {
                        close (inp2);
                        error ("too many nested includes");
                }
        else {
                error ("Could not open include file");
        }
        do_kill();

}

/**
 * "asm" pseudo-statement
 * enters mode where assembly language statements are passed
 * intact through parser
 */
void doasm(void)
{
        cmode = 0;
        FOREVER {
                readline ();
                if (match ("#endasm"))
                        break;
                if (input_eof)
                        break;
                output_string (line);
                newline ();
        }
        do_kill();
        cmode = 1;

}

void dodefine(void)
{
        addmac();
}

void doundef(void)
{
        int     mp;
        char    sname[NAMESIZE];

        if (!symname(sname)) {
                illname();
                do_kill();
                return;
        }

        if ((mp = findmac(sname)) != 0)
                delmac(mp);
        do_kill();

}

void preprocess(void)
{
        if (ifline()) return;
        while (cpp());
}

void doifdef(int ifdef)
{
        char sname[NAMESIZE];
        int k;

        blanks();
        ++iflevel;
        if (skiplevel) return;
        k = symname(sname) && findmac(sname);
        if (k != ifdef) skiplevel = iflevel;

}

int ifline(void)
{
        FOREVER {
                readline();
                if (input_eof) return(1);
                if (match("#ifdef")) {
                        doifdef(YES);
                        continue;
                } else if (match("#ifndef")) {
                        doifdef(NO);
                        continue;
                } else if (match("#else")) {
                        if (iflevel) {
                                if (skiplevel == iflevel) skiplevel = 0;
                                else if (skiplevel == 0) skiplevel = iflevel;
                        } else noiferr();
                        continue;
                } else if (match("#endif")) {
                        if (iflevel) {
                                if (skiplevel == iflevel) skiplevel = 0;
                                --iflevel;
                        } else noiferr();
                        continue;
                }
                if (!skiplevel) return(0);
        }

}

void noiferr(void)
{
        error("no matching #if...");

}

/**
 * preprocess - copies mline to line with special treatment of preprocess cmds
 * @return 
 */
int cpp(void)
{
        int     k;
        char    c, sname[NAMESIZE];
        int     tog;
        int     cpped;          /* non-zero if something expanded */

        cpped = 0;
        /* don't expand lines with preprocessor commands in them */
        if (!cmode || line[0] == '#') return(0);

        mptr = lptr = 0;
        while (ch ()) {
                if ((ch () == ' ') | (ch () == 9)) {
                        keepch (' ');
                        while ((ch () == ' ') | (ch () == 9))
                                gch ();
                } else if (ch () == '"') {
                        keepch (ch ());
                        gch ();
                        while (ch () != '"') {
                                if (ch () == 0) {
                                        error ("missing quote");
                                        break;
                                }
                                if (ch() == '\\') keepch(gch());
                                keepch (gch ());
                        }
                        gch ();
                        keepch ('"');
                } else if (ch () == '\'') {
                        keepch ('\'');
                        gch ();
                        while (ch () != '\'') {
                                if (ch () == 0) {
                                        error ("missing apostrophe");
                                        break;
                                }
                                if (ch() == '\\') keepch(gch());
                                keepch (gch ());
                        }
                        gch ();
                        keepch ('\'');
                } else if ((ch () == '/') & (nch () == '*')) {
                        inchar ();
                        inchar ();
                        while ((((c = ch ()) == '*') & (nch () == '/')) == 0)
                                if (c == '$') {
                                        inchar ();
                                        tog = TRUE;
                                        if (ch () == '-') {
                                                tog = FALSE;
                                                inchar ();
                                        }
                                        if (alpha (c = ch ())) {
                                                inchar ();
                                                toggle (c, tog);
                                        }
                                } else {
                                        if (ch () == 0)
                                                readline ();
                                        else
                                                inchar ();
                                        if (input_eof)
                                                break;
                                }
                        inchar ();
                        inchar ();
                } else if ((ch () == '/') & (nch () == '/')) { // one line comment
                        while(gch());
                } else if (alphanumeric(ch ())) {
                        k = 0;
                        while (alphanumeric(ch ())) {
                                if (k < NAMEMAX)
                                        sname[k++] = ch ();
                                gch ();
                        }
                        sname[k] = 0;
                        if ((k = findmac (sname)) != 0) {
                                cpped = 1;
                                while ((c = macq[k++]) != 0)
                                        keepch (c);
                        } else {
                                k = 0;
                                while ((c = sname[k++]) != 0)
                                        keepch (c);
                        }
                } else
                        keepch (gch ());
        }
        keepch (0);
        if (mptr >= MPMAX)
                error ("line too long");
        lptr = mptr = 0;
        while((line[lptr++] = mline[mptr++]) != 0);
        lptr = 0;
        printf(">%s\n", line);
        return(cpped);

}

int keepch(char c)
{
        mline[mptr] = c;
        if (mptr < MPMAX)
                mptr++;
        return (c);

}

void defmac(char *s)
{
        do_kill();
        strcpy(line, s);
        addmac();
}

void addmac(void)
{
        char    sname[NAMESIZE];
        int     k;
        int     mp;

        if (!symname (sname)) {
                illname();
                do_kill();
                return;
        }
        if ((mp = findmac(sname)) != 0) {
                error("Duplicate define");
                delmac(mp);
        }
        k = 0;
        while (putmac (sname[k++]));
        while (ch () == ' ' || ch () == 9)
                gch ();
        //while (putmac (gch ()));
        while (putmac(remove_one_line_comment(gch ())));
        if (macptr >= MACMAX)
                error ("macro table full");

}

/**
 * removes one line comments from defines
 * @param c
 * @return 
 */
int remove_one_line_comment(char c) {
    if ((c == '/') && (ch() == '/')) {
        while(gch());
        return 0;
    } else {
        return c;
    }
}

void delmac(int mp) {
        --mp; --mp;     /* step over previous null */
        while (mp >= 0 && macq[mp]) macq[mp--] = '%';

}

int putmac(char c)
{
        macq[macptr] = c;
        if (macptr < MACMAX)
                macptr++;
        return (c);

}

int findmac(char *sname)
{
        int     k;

        k = 0;
        while (k < macptr) {
                if (astreq (sname, macq + k, NAMEMAX)) {
                        while (macq[k++]);
                        return (k);
                }
                while (macq[k++]);
                while (macq[k++]);
        }
        return (0);

}

void toggle(char name, int onoff) {
        switch (name) {
        case 'C':
                ctext = onoff;
                break;
        }
}

