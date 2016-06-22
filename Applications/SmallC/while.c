/*      File while.c: 2.1 (83/03/20,16:02:22) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"

void addwhile(WHILE *ptr) {
//int     ptr[];
    //int     k;

    //if (wsptr == WSMAX) {
    if (while_table_index == WSTABSZ) {
        error ("too many active whiles");
        return;
    }
    //k = 0;
    //while (k < WSSIZ)
    //        *wsptr++ = ptr[k++];
    memcpy(&ws[while_table_index++], ptr, sizeof(WHILE));
}

void delwhile(void) {
    if (readwhile ()) {
        //wsptr = wsptr - WSSIZ;
        while_table_index--;
    }
}

WHILE *readwhile(void) {
    if (while_table_index == 0) {
    //if (wsptr == ws) {
        error ("no active do/for/while/switch");
        return (0);
    } else {
        //return (wsptr-WSSIZ);
        return &ws[while_table_index - 1];
    }
}

WHILE *findwhile(void) {
    //int     *ptr;
    int while_table_idx;

    //for (ptr = wsptr; ptr != ws;) {
    while_table_idx = while_table_index;
    for (; while_table_idx != 0;) {
        //ptr = ptr - WSSIZ;
        while_table_idx--;
        //if (ptr[WSTYP] != WSSWITCH)
        //        return (ptr);
        if (ws[while_table_idx].type != WSSWITCH)
            return &ws[while_table_idx];
    }
    error ("no active do/for/while");
    return (0);
}

WHILE *readswitch(void) {
    WHILE *ptr; //int     *ptr;

    if ((ptr = readwhile ()) != 0) {
        //if (ptr[WSTYP] == WSSWITCH)
        if (ptr->type == WSSWITCH) {
            return (ptr);
        }
    }
    return (0);
}

void addcase(int val) {
    int     lab;

    if (swstp == SWSTSZ)
        error ("too many case labels");
    else {
        swstcase[swstp] = val;
        swstlab[swstp++] = lab = getlabel ();
        print_label (lab);
        output_label_terminator ();
        newline ();
    }
}
