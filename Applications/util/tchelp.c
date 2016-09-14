/*
 *	The termcap libraries need lots of buffers and also
 *	suck in things we often don't need it large but tight
 *	programs. tchelper does a series of lookups and encodes
 *	them in stdout so it can easily be used via a pipe as
 *	a helper app
 *
 *	Passed an argument listing the needed properties it
 *	returns a block back with a leading length followed by
 *	the data in the form of ints or zero terminated strings
 *	(the caller is advised to do int/bools first so you get
 *	fixed offsets, then the strings.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termcap.h>

static char buf[1024];
static char output[1024];
static char result[2048];
static char id[3];

int main(int argc, char *argv[])
{
    char *op = output;
    char *rp = result;
    const char *t = getenv("TERM");

    char *prop, *pe;
    int n;

    if (argc != 2) {
        fprintf(stderr, "%s [props]\n", argv[0]);
        exit(1);
    }

    if (t == NULL)
        t = "dumb";
    if ((n = tgetent(buf, t)) != 1) {
        fprintf(stderr, "%s is not a known terminal type\n", t);
        putw(0, stdout);
        exit(1);
    }
    prop = argv[1];
    pe = prop + strlen(prop) - 2;
    while(prop < pe) {
        id[0] = *prop++;
        id[1] = *prop++;
        if (*prop == '$') {
            char *r = tgetstr(id, &op);
            if (r) {
                size_t l = strlen(r);
                memcpy(rp, r, l);
                rp += l;
            }
            *rp++= 0;
            prop++;
            continue;
        }
        if (*prop == '#') {
            n = tgetnum(id);
            prop++;
        } else
            n = tgetflag(id);
        /* Intentionally native type and endian */
        memcpy(rp, &n, sizeof(n));
        rp += sizeof(n);
    }
    putw(rp - result, stdout);
    fwrite(result, rp - result, 1, stdout);
    return 0;
}

        
            