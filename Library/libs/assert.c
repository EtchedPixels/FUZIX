/* Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void __errput(char *str) {
    write(2, str, strlen(str));
}

void __assert(char *assertion, char *filename, int linenumber) {
    __errput("Failed '");
    __errput(assertion);
    __errput("', file ");
    __errput(filename);
    __errput(", line ");
    __errput(_itoa(linenumber));
    __errput(".\n");
    abort();
}
