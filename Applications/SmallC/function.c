/*
 * File function.c: 2.1 (83/03/20,16:02:04)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

int argtop;

/**
 * begin a function
 * called from "parse", this routine tries to make a function out
 * of what follows
 * modified version.  p.l. woods
 */
void newfunc(void) {
    char n[NAMESIZE];
    int idx, type;
    fexitlab = getlabel();

    if (!symname(n)) {
        error("illegal function or declaration");
        do_kill();
        return;
    }
    if ((idx = find_global(n)) > -1) {
        if (symbol_table[idx].identity != FUNCTION)
            multidef(n);
        else if (symbol_table[idx].offset == FUNCTION)
            multidef(n);
        else
            symbol_table[idx].offset = FUNCTION;
    } else
        add_global(n, FUNCTION, CINT, FUNCTION, PUBLIC);
    if (!match("("))
        error("missing open paren");
    output_string(n);
    output_label_terminator();
    newline();
    local_table_index = NUMBER_OF_GLOBALS; //locptr = STARTLOC;
    argstk = 0;
    // ANSI style argument declaration
    if (doAnsiArguments() == 0) {
        // K&R style argument declaration
        while (!match(")")) {
            if (symname(n)) {
                if (find_locale(n) > -1)
                    multidef(n);
                else {
                    add_local(n, 0, 0, argstk, AUTO);
                    argstk = argstk + INTSIZE;
                }
            } else {
                error("illegal argument name");
                junk();
            }
            blanks();
            if (!streq(line + lptr, ")")) {
                if (!match(","))
                    error("expected comma");
            }
            if (endst())
                break;
        }
        stkp = 0;
        argtop = argstk;
        while (argstk) {
            if ((type = get_type()) != 0) {
                getarg(type);
                need_semicolon();
            } else {
                error("wrong number args");
                break;
            }
        }
    }
    statement(YES);
    print_label(fexitlab);
    output_label_terminator();
    newline();
    gen_modify_stack(0);
    gen_ret();
    stkp = 0;
    local_table_index = NUMBER_OF_GLOBALS; //locptr = STARTLOC;
}

/**
 * declare argument types
 * called from "newfunc", this routine adds an entry in the local
 * symbol table for each named argument
 * completely rewritten version.  p.l. woods
 * @param t argument type (char, int)
 * @return 
 */
void getarg(int t) {
    int j, legalname, address, argptr;
    char n[NAMESIZE];

    FOREVER
    {
        if (argstk == 0)
            return;
        if (match("*"))
            j = POINTER;
        else
            j = VARIABLE;
        if (!(legalname = symname(n)))
            illname();
        if (match("[")) {
            while (inbyte() != ']')
                if (endst())
                    break;
            j = POINTER;
        }
        if (legalname) {
            if ((argptr = find_locale(n)) > -1) {
                symbol_table[argptr].identity = j;
                symbol_table[argptr].type = t;
                address = argtop - symbol_table[argptr].offset;
                symbol_table[argptr].offset = address;
            } else
                error("expecting argument name");
        }
        argstk = argstk - INTSIZE;
        if (endst())
            return;
        if (!match(","))
            error("expected comma");
    }
}

int doAnsiArguments(void) {
    int type;
    type = get_type();
    if (type == 0) {
        return 0; // no type detected, revert back to K&R style
    }
    argtop = argstk;
    argstk = 0;
    FOREVER
    {
        if (type) {
            doLocalAnsiArgument(type);
        } else {
            error("wrong number args");
            break;
        }
        if (match(",")) {
            type = get_type();
            continue;
        }
        if (match(")")) {
            break;
        }
    }
    return 1;
}

void doLocalAnsiArgument(int type) {
    char symbol_name[NAMESIZE];
    int identity, address, argptr, ptr;

    if (match("*")) {
        identity = POINTER;
    } else {
        identity = VARIABLE;
    }
    if (symname(symbol_name)) {
        if (find_locale(symbol_name) > -1) {
            multidef(symbol_name);
        } else {
            argptr = add_local (symbol_name, identity, type, 0, AUTO);
            argstk = argstk + INTSIZE;
            ptr = local_table_index;
            while (ptr != NUMBER_OF_GLOBALS) { // modify stack offset as we push more params
                ptr = ptr - 1;
                address = symbol_table[ptr].offset;
                symbol_table[ptr].offset = address + INTSIZE;
            }
        }
    } else {
        error("illegal argument name");
        junk();
    }
    if (match("[")) {
        while (inbyte() != ']') {
            if (endst()) {
                break;
            }
        }
        identity = POINTER;
        symbol_table[argptr].identity = identity;
    }
}

