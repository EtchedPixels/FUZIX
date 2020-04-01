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

    if (!symname(n)) {
        error("illegal function or declaration");
        do_kill();
        return;
    }
    if (!match("("))
        error("missing open paren");
    newfunc_typed(PUBLIC, n, CINT);
}

void newfunc_typed(int storage, char *n, int type)
{
    int idx;
    SYMBOL *symbol;
    char an[NAMESIZE];

    fexitlab = getlabel();

    if ((idx = find_global(n)) > -1) {
        symbol = &symbol_table[idx];
        if (symbol->identity != FUNCTION)
            multidef(n);
    } else {
        /* extern implies global scope */
        idx = add_global(n, FUNCTION, CINT, 0, storage == EXTERN ? PUBLIC : storage);
        symbol = &symbol_table[idx];
    }
    local_table_index = NUMBER_OF_GLOBALS; //locptr = STARTLOC;
    argstk = 0;
    // ANSI style argument declaration
    if (doAnsiArguments()) {
        if (storage == EXTERN) {
            need_semicolon();
            return;
        }
        /* No body .. just a definition */
        if (match(";"))
            return;
    } else {
        // K&R style argument declaration
        while (!match(")")) {
            if (symname(an)) {
                if (find_locale(an) > -1)
                    multidef(an);
                else {
                    /* FIXME: struct */
                    add_local(an, 0, 0, argstk, AUTO);
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
        if (storage == EXTERN) {
            need_semicolon();
            return;
        }
        /* No body .. just a definition */
        if (match(";"))
            return;
        stkp = 0;
        argtop = argstk;
        while (argstk) {
            if ((type = get_type()) != -1) {
                notvoid(type);
                getarg(type);
                need_semicolon();
            } else {
                error("wrong number args");
                break;
            }
        }
    }
    if (symbol->offset == FUNCTION)
            multidef(n);
    symbol->offset = FUNCTION;
    output_label_name(n);
    output_label_terminator();
    newline();
    gen_prologue();
    statement(YES);
    print_label(fexitlab);
    output_label_terminator();
    newline();
    gen_epilogue();
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
    if (type == -1) {
        return 0; // no type detected, revert back to K&R style
    }
    argtop = argstk;
    argstk = 0;
    FOREVER
    {
        /* We don't need to pull a variable for void */
        if (type != -1) {
            doLocalAnsiArgument(type);
        } else {
            error("wrong number of args");
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
        if (type == STRUCT) {
            error("cannot pass struct");
            return;
        }
        identity = VARIABLE;
        if (type == VOID)
            return;
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
                /* Struct etc FIXME */
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

