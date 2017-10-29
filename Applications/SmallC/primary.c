/*
 * File primary.c: 2.4 (84/11/27,16:26:07)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

int primary(LVALUE *lval) {
    char    sname[NAMESIZE];
    int     num[1], k, symbol_table_idx, offset, reg;
    SYMBOL *symbol;

    lval->ptr_type = 0;  // clear pointer/array type
    lval->tagsym = 0;
    if (match ("(")) {
        k = hier1 (lval);
        needbrack (")");
        return (k);
    }
    if (amatch("sizeof", 6)) {
        needbrack("(");
        gen_immediate();
        if (amatch("int", 3)) output_number(INTSIZE);
        else if (amatch("char", 4)) output_number(1);
        else if (symname(sname)) {
            if (((symbol_table_idx = find_locale(sname)) > -1) ||
                ((symbol_table_idx = find_global(sname)) > -1)) {
                symbol = &symbol_table[symbol_table_idx];
                if (symbol->storage == LSTATIC)
                    error("sizeof local static");
                offset = symbol->offset;
                if ((symbol->type & CINT) ||
                    (symbol->identity == POINTER))
                    offset *= INTSIZE;
                else if (symbol->type == STRUCT)
                    offset *= tag_table[symbol->tagidx].size;
                output_number(offset);
            } else {
                error("sizeof undeclared variable");
                output_number(0);
            }
        } else {
            error("sizeof only on type or variable");
        }
        needbrack(")");
        newline();
        lval->symbol = 0;
        lval->indirect = 0;
        return(0);
    }
    if (symname (sname)) {
        int local = 1;
        symbol_table_idx = find_locale(sname);
        if (symbol_table_idx == -1) {
            local = 0;
            symbol_table_idx = find_global(sname);
            /* Only valid undeclared name is a function reference */
            if (symbol_table_idx == -1) {
                blanks();
                if (ch() != '(')
                    error("undeclared variable");
                symbol_table_idx = add_global(sname, FUNCTION, CINT, 0, PUBLIC);
                symbol = &symbol_table[symbol_table_idx];
                lval->symbol = symbol;
                lval->indirect = 0;
                return 0;
            }
        }
        symbol = &symbol_table[symbol_table_idx];

        if (local && gen_indirected(symbol)) {
            reg = gen_get_locale(symbol);
            lval->symbol = symbol;
            lval->indirect = symbol->type;
            if (symbol->type == STRUCT) {
                lval->tagsym = &tag_table[symbol->tagidx];
            }
            if (symbol->identity == ARRAY ||
                (symbol->identity == VARIABLE && symbol->type == STRUCT)) {
                lval->ptr_type = symbol->type;
                return reg;
            }
            if (symbol->identity == POINTER) {
                lval->indirect = CINT;
                lval->ptr_type = symbol->type;
            }
            return FETCH | reg;
        }

	/* Globals, function names */
	lval->symbol = symbol;
	lval->indirect = 0;
        if (symbol->identity != FUNCTION) {

	    /* Globals and anything we can directly access */
            if (symbol->type == STRUCT) {
                lval->tagsym = &tag_table[symbol->tagidx];
            }
            if (symbol->identity != ARRAY &&
                (symbol->identity != VARIABLE || symbol->type != STRUCT)) {
                if (symbol->identity == POINTER) {
                    lval->ptr_type = symbol->type;
                }
                return FETCH | HL_REG;
            }

            if (symbol->storage == LSTATIC) {
                gen_get_locale(symbol);
            } else {
                gen_immediate();
                output_label_name(symbol->name);
                newline();
            }
            lval->indirect = symbol->type;
            lval->ptr_type = symbol->type;
        } else {

	    /* Function call */
            lval->ptr_type = symbol->type;
	}
	return 0;
    }
    lval->symbol = 0;
    lval->indirect = 0;
    if (constant(num))
        return 0;
    else {
        error("invalid expression");
        gen_immediate();
        output_number(0);
        newline();
        junk();
        return 0;
    }
}

/**
 * true if val1 -> int pointer or int array and val2 not pointer or array
 * @param val1
 * @param val2
 * @return 
 */
int dbltest(LVALUE *val1, LVALUE *val2) {
    if (val1 == NULL)
        return (FALSE);
    if (val1->ptr_type) {
        if (val1->ptr_type & CCHAR)
            return (FALSE);
        if (val2->ptr_type)
            return (FALSE);
        return (TRUE);
    }
    return (FALSE);
}

/**
 * determine type of binary operation
 * @param lval
 * @param lval2
 * @return 
 */
void result(LVALUE *lval, LVALUE *lval2) {
    if (lval->ptr_type && lval2->ptr_type)
        lval->ptr_type = 0;
    else if (lval2->ptr_type) {
        lval->symbol = lval2->symbol;
        lval->indirect = lval2->indirect;
        lval->ptr_type = lval2->ptr_type;
    }
}

int constant(int val[]) {
    if (number (val))
        gen_immediate ();
    else if (quoted_char (val))
        gen_immediate ();
    /* Quoted strings are constants so we don't need to do any mucking about
       with segments - however we move them to data as we'd otherwise put
       them mid code stream ! */
    else if (quoted_string (NULL, val)) {
        gen_immediate ();
        print_label (val[0]);
        newline();
        return 1;
    } else
        return (0);
    output_number (val[0]);
    newline ();
    return (1);
}

int number(int val[]) {
    int     k, minus, base;
    char    c;

    k = minus = 1;
    while (k) {
        k = 0;
        if (match("+"))
            k = 1;
        if (match("-")) {
            minus = (-minus);
            k = 1;
        }
    }
    if (!numeric(c = ch ()))
        return (0);
    if (match("0x") || match ("0X"))
        while (numeric(c = ch ()) ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F')) {
            inbyte ();
            k = k * 16 + (numeric (c) ? (c - '0') : ((c & 07) + 9));
        }
    else {
        base = (c == '0') ? 8 : 10;
        while (numeric(ch())) {
            c = inbyte ();
            k = k * base + (c - '0');
        }
    }
    if (minus < 0)
            k = (-k);
    val[0] = k;
    if(k < 0) {
        return (UINT);
    } else {
        return (CINT);
    }
}

/**
 * Test if we have one char enclosed in single quotes
 * @param value returns the char found
 * @return 1 if we have, 0 otherwise
 */
int quoted_char(int *value) {
    int     k;
    char    c;

    k = 0;
    if (!match ("'"))
        return (0);
    while ((c = gch ()) != '\'') {
        c = (c == '\\') ? spechar(): c;
        k = (k & 255) * 256 + (c & 255);
    }
    *value = k;
    return (1);
}

/**
 * Test if we have string enclosed in double quotes. e.g. "abc".
 * Load the string into literal pool.
 * @param position returns label for this string
 * @return 1 if such string found, 0 otherwise
 */
int quoted_string(int *len, int *position) {
    char    c;
    int     x;
    int     l;

    if (!match ("\""))
        return (0);
    if (position) {
        data_segment_gdata();
        *position = getlabel();
        generate_label(*position);
    }
    x = 0;
    while (ch () != '"') {
        if (ch () == 0) {
            /* Should error ?? FIXME */
            break;
        }
        c = gch();
        c = (c == '\\') ? spechar(): c;
        if (x == 0)
            gen_def_byte();
        else
            output_byte(',');
         output_number(c);
         if (x++ == 7) {
             x = 0;
             newline();
        }
        l++;
    }
    gch ();
    if (x != 0)
        newline();
    gen_def_byte();
    output_number(0);
    newline();
    if (len)
        *len = l + 1;
    if (position)
        code_segment_gtext();
    return (1);
}

#define BADHEX 127

int hexdigit(char c)
{
    if (c >= 'a')
        c -= 32;
    if (c >= 'A' && c <= 'F')
        return c - 'A';
    if (c >= '0' && c <= '9')
        return c - '0';
    return BADHEX;
}

/**
 * decode special characters (preceeded by back slashes)
 */
int spechar(void) {
    char c;
    int r;

    c = ch();

    if      (c == 'n') c = LF;
    else if (c == 't') c = TAB;
    else if (c == 'r') c = CR;
    else if (c == 'f') c = FFEED;
    else if (c == 'b') c = BKSP;
    else if (c == EOS) return 0;

    gch();
    if (c == 'x') {
        c = hexdigit(ch());
        if (c == BADHEX) {
            error("bad escape");
            return '?';
        }
        r = c;
        gch();
        c = hexdigit(ch());
        if (c == BADHEX)
            return r;
        r <<= 4;
        r |= c;
        gch();
        return r;
    }
    if (c == '0') {
        int n;
        n = 0;
        r = 0;

        while(n < 3) {
            c = ch();
            if (c < '0' || c > '7')
                return r;
            r <<= 3;
            r |= c - '0';
            n++;
            gch();
        }
        return r;
    }
    return (c);
}

/**
 * perform a function call
 * called from "hier11", this routine will either call the named
 * function, or if the supplied ptr is zero, will call the contents
 * of HL
 * @param ptr name of the function
 */
void callfunction(char *ptr) {
    int     nargs;
    int     i;

    nargs = 0;

    blanks ();
    if (ptr == 0)
        gen_push (HL_REG);
    while (!streq (line + lptr, ")")) {
        if (endst ())
            break;
        defer_output();
        expression (NO);
        if (ptr == 0)
            gen_swap_stack ();
        /* Worth making the first argument pass in HL ?? */
        gen_push (HL_REG);
        nargs++;
        /* Will need to track sizes later */
        if (!match (","))
            break;
    }
    /* Now paste the argument generation blocks into the output
       in reverse order so the stack is right */
    for (i = 0; i < nargs; i++)
        end_defer();
    needbrack (")");
    if (aflag)
        gnargs(nargs);
    if (ptr)
        gen_call (ptr);
    else
        callstk ();
    stkp = gen_modify_stack (stkp + nargs * INTSIZE);
}

void needlval(void) {
    error ("must be lvalue");
}

