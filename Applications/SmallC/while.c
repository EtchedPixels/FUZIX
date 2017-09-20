/*      File while.c: 2.1 (83/03/20,16:02:22) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"

char *noactive = "mo active do/for/while/switch";

void addwhile(WHILE *ptr) {
    if (while_table_index == WSTABSZ) {
        error ("too many active whiles");
        return;
    }
    memcpy(&ws[while_table_index++], ptr, sizeof(WHILE));
}

void delwhile(void) {
    if (readwhile ())
        while_table_index--;
}

WHILE *readwhile(void) {
    if (while_table_index == 0) {
        error (noactive);
        return (0);
    } else {
        return &ws[while_table_index - 1];
    }
}

WHILE *findwhile(void) {
    int while_table_idx;

    while_table_idx = while_table_index;
    for (; while_table_idx != 0;) {
        while_table_idx--;
        if (ws[while_table_idx].type != WSSWITCH)
            return &ws[while_table_idx];
    }
    error (noactive);
    return (0);
}

WHILE *readswitch(void) {
    WHILE *ptr;

    if ((ptr = readwhile ()) != 0) {
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
