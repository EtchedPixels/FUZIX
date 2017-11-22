/*      File gen.c: 2.1 (83/03/20,16:02:06) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "data.h"

char obuf[512];		/* Shared with outstack for defer uses */
static char *optr = obuf;

void oflush(void)
{
    if (write(output, obuf, optr-obuf) != optr-obuf)
        error("I/O error");
    optr = obuf;
}

/**
 * return next available internal label number
 */
int getlabel(void) {
    return (nxtlab++);
}

/**
 * print specified number as label
 * @param label
 */
void print_label(int label) {
    output_label_prefix ();
    output_decimal (label);
}

/**
 * glabel - generate label
 * @param lab label number
 */
void glabel(char *lab) {
    output_label_name(lab);
    output_label_terminator ();
    newline ();
}

/**
 * gnlabel - generate numeric label
 * @param nlab label number
 * @return 
 */
void generate_label(int nlab) {
    print_label (nlab);
    output_label_terminator ();
    newline ();
}

/**
 * outputs one byte
 * @param c
 * @return 
 */
int output_byte(char c) {
    if (c == 0)
        return (0);
    if (optr == obuf + sizeof(obuf))
        oflush();
    *optr++ = c;
//FIX
    oflush();
    return (c);
}

/**
 * outputs a string
 * @param ptr the string
 * @return 
 */
void output_string(char *ptr) {
    int k;
    k = 0;
    while (output_byte (ptr[k++]));
}

/**
 * outputs a tab
 * @return 
 */
void print_tab(void) {
    output_byte ('\t');
}

/**
 * output line
 * @param ptr
 * @return 
 */
void output_line(char *ptr)
{
    output_with_tab (ptr);
    newline ();
}

/**
 * tabbed output
 * @param ptr
 * @return 
 */
void output_with_tab(char *ptr) {
    print_tab ();
    output_string (ptr);
}

#ifdef __linux__
char *_itoa(int n)
{
    static char buf[32];
    snprintf(buf, 32, "%d", n);
    return buf;
}
#endif
    
/**
 * output decimal number
 * @param number
 * @return 
 */
void output_decimal(int number) {
    char *p = _itoa(number);
    output_string(p);
}

/**
 * stores values into memory
 * @param lval
 * @return 
 */
void store(LVALUE *lval) {
    if (lval->indirect == 0)
        gen_put_memory (lval->symbol);
    else
        gen_put_indirect (lval->indirect);
}

int rvalue(LVALUE *lval, int reg) {
    if ((lval->symbol != 0) && (lval->indirect == 0))
        gen_get_memory (lval->symbol);
    else
        gen_get_indirect (lval->indirect, reg);
    return HL_REG;
}

/**
 * parses test part "(expression)" input and generates assembly for jump
 * @param label
 * @param ft : false - test jz, true test jnz
 * @return 
 */
void test(int label, int ft) {
    needbrack ("(");
    expression (YES);
    needbrack (")");
    gen_test_jump (label, ft);
}

/**
 * scale constant depending on type
 * @param type
 * @param otag
 * @param size
 * @return 
 */
void scale_const(int type, int otag, int *size) {
    switch (type) {
        case CINT:
        case UINT:
            *size += *size;
            break;
        case STRUCT:
            *size *= tag_table[otag].size;
            break;
        default:
            break;
    }
}
