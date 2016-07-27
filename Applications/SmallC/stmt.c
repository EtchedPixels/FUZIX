/*
 * File stmt.c: 2.1 (83/03/20,16:02:17)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * statement parser
 * called whenever syntax requires a statement.  this routine
 * performs that statement and returns a number telling which one
 * @param func func is true if we require a "function_statement", which
 * must be compound, and must contain "statement_list" (even if
 * "declaration_list" is omitted)
 * @return statement type
 */
int statement(int func) {
    if ((ch () == 0) && input_eof)
        return (0);
    lastst = 0;
    if (func) {
        if (match ("{")) {
            do_compound (YES);
            return (lastst);
        } else
            error ("function requires compound statement");
    }
    if (match ("{"))
        do_compound (NO);
    else {
        do_statement ();
        gen_statement_end();
    }
    return (lastst);
}

/**
 * declaration
 */
int statement_declare(void) {
    if (amatch("register", 8))
        do_local_declares(REGISTER);
    else if (amatch("auto", 4))
        do_local_declares(DEFAUTO);
    else if (amatch("static", 6))
        do_local_declares(LSTATIC);
    else if (do_local_declares(AUTO)) ;
    else
        return (NO);
    return (YES);
}

/**
 * local declarations
 * @param stclass
 * @return 
 */
int do_local_declares(int stclass) {
    int type = 0;
    int otag;   // tag of struct object being declared
    int sflag;  // TRUE for struct definition, zero for union
    char sname[NAMESIZE];
    blanks();
    if ((sflag=amatch("struct", 6)) || amatch("union", 5)) {
        if (symname(sname) == 0) { // legal name ?
            illname();
        }
        if ((otag=find_tag(sname)) == -1) { // structure not previously defined
            otag = define_struct(sname, stclass, sflag);
        }
        declare_local(STRUCT, stclass, otag);
    } else if ((type = get_type()) != -1) {
        declare_local(type, stclass, -1);
    } else if (stclass == LSTATIC || stclass == DEFAUTO) {
        declare_local(CINT, stclass, -1);
    } else {
        return(0);
    }
    need_semicolon();
    return(1);
}

/**
 * non-declaration statement
 */
void do_statement(void) {
    if (amatch ("if", 2)) {
        doif ();
        lastst = STIF;
    } else if (amatch ("while", 5)) {
        dowhile ();
        lastst = STWHILE;
    } else if (amatch ("switch", 6)) {
        doswitch ();
        lastst = STSWITCH;
    } else if (amatch ("do", 2)) {
        dodo ();
        need_semicolon ();
        lastst = STDO;
    } else if (amatch ("for", 3)) {
        dofor ();
        lastst = STFOR;
    } else if (amatch ("return", 6)) {
        doreturn ();
        need_semicolon ();
        lastst = STRETURN;
    } else if (amatch ("break", 5)) {
        dobreak ();
        need_semicolon ();
        lastst = STBREAK;
    } else if (amatch ("continue", 8)) {
        docont();
        need_semicolon ();
        lastst = STCONT;
    } else if (match (";"))
        ;
    else if (amatch ("case", 4)) {
        docase ();
        lastst = statement (NO);
    } else if (amatch ("default", 7)) {
        dodefault ();
        lastst = statement (NO);
    } else if (match ("#asm")) {
        doasm ();
        lastst = STASM;
    } else if (match ("{"))
        do_compound (NO);
    else {
        expression (YES);
/*      if (match (":")) {
            dolabel ();
            lastst = statement (NO);
        } else {
*/          need_semicolon ();
            lastst = STEXP;
/*      }
*/  }
}

/**
 * compound statement
 * allow any number of statements to fall between "{" and "}"
 * 'func' is true if we are in a "function_statement", which
 * must contain "statement_list"
 */
void do_compound(int func) {
        int     decls;

        decls = YES;
        ncmp++;
        while (!match ("}")) {
                if (input_eof)
                        return;
                if (decls) {
                        if (!statement_declare ()) {
                                /* Any deferred movement now happens */
                                gen_modify_stack(stkp);
                                decls = NO;
                        }
                } else {
                        do_statement ();
                        gen_statement_end();
                }
        }
        ncmp--;
}

/**
 * "if" statement
 */
void doif(void) {
        int     fstkp, flab1, flab2;
        int     flev;

        flev = local_table_index;
        fstkp = stkp;
        flab1 = getlabel ();
        test (flab1, FALSE);
        statement (NO);
        stkp = gen_modify_stack (fstkp);
        local_table_index = flev;
        if (!amatch ("else", 4)) {
                generate_label (flab1);
                return;
        }
        gen_jump (flab2 = getlabel ());
        generate_label (flab1);
        statement (NO);
        stkp = gen_modify_stack (fstkp);
        local_table_index = flev;
        generate_label (flab2);
}

/**
 * "while" statement
 */
void dowhile(void) {
        WHILE ws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSWHILE;
        ws.case_test = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        generate_label (ws.case_test);
        test (ws.while_exit, FALSE);
        statement (NO);
        gen_jump (ws.case_test);
        generate_label (ws.while_exit);
        local_table_index = ws.symbol_idx;
        stkp = gen_modify_stack (ws.stack_pointer);
        delwhile ();
}

/**
 * "do" statement
 */
void dodo(void) {
        WHILE ws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSDO;
        ws.body_tab = getlabel ();
        ws.case_test = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        generate_label (ws.body_tab);
        statement (NO);
        if (!match ("while")) {
                error ("missing while");
                return;
        }
        generate_label (ws.case_test);
        test (ws.body_tab, TRUE);
        generate_label (ws.while_exit);
        local_table_index = ws.symbol_idx;
        stkp = gen_modify_stack (ws.stack_pointer);
        delwhile ();
}

/**
 * "for" statement
 */
void dofor(void) {
        WHILE ws;
        WHILE *pws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSFOR;
        ws.case_test = getlabel ();
        ws.incr_def = getlabel ();
        ws.body_tab = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        pws = readwhile ();
        needbrack ("(");
        if (!match (";")) {
                expression (YES);
                need_semicolon ();
                gen_statement_end();
        }
        generate_label (pws->case_test);
        if (!match (";")) {
                expression (YES);
                gen_test_jump (pws->body_tab, TRUE);
                gen_jump (pws->while_exit);
                gen_statement_end();
                need_semicolon ();
        } else
                pws->case_test = pws->body_tab;
        generate_label (pws->incr_def);
        if (!match (")")) {
                expression (YES);
                gen_statement_end();
                needbrack (")");
                gen_jump (pws->case_test);
        } else
                pws->incr_def = pws->case_test;
        generate_label (pws->body_tab);
        statement (NO);
        gen_jump (pws->incr_def);
        generate_label (pws->while_exit);
        local_table_index = pws->symbol_idx;
        stkp = gen_modify_stack (pws->stack_pointer);
        delwhile ();
}

/**
 * "switch" statement
 */
void doswitch(void) {
        WHILE ws;
        WHILE *ptr;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSSWITCH;
        ws.case_test = swstp;
        ws.body_tab = getlabel ();
        ws.incr_def = ws.while_exit = getlabel ();
        addwhile (&ws);
        gen_immediate ();
        print_label (ws.body_tab);
        newline ();
        gen_push (HL_REG);
        needbrack ("(");
        expression (YES);
        needbrack (")");
        stkp = stkp + INTSIZE;  // '?case' will adjust the stack
        gen_jump_case ();
        statement (NO);
        ptr = readswitch ();
        gen_jump (ptr->while_exit);
        dumpsw (ptr);
        generate_label (ptr->while_exit);
        local_table_index = ptr->symbol_idx;
        stkp = gen_modify_stack (ptr->stack_pointer);
        swstp = ptr->case_test;
        delwhile ();
}

/**
 * "case" label
 */
void docase(void) {
        int     val;

        val = 0;
        if (readswitch ()) {
                if (!number (&val))
                        if (!quoted_char (&val))
                                error ("bad case label");
                addcase (val);
                if (!match (":"))
                        error ("missing colon");
        } else
                error ("no active switch");
}

/**
 * "default" label
 */
void dodefault(void) {
        WHILE *ptr;
        int        lab;

        if ((ptr = readswitch ()) != 0) {
                ptr->incr_def = lab = getlabel ();
                generate_label (lab);
                if (!match (":"))
                        error ("missing colon");
        } else
                error ("no active switch");
}

/**
 * "return" statement
 */
void doreturn(void) {
        if (endst () == 0)
                expression (YES);
        gen_jump(fexitlab);
}

/**
 * "break" statement
 */
void dobreak(void) {
        WHILE *ptr;

        if ((ptr = readwhile ()) == 0)
                return;
        gen_modify_stack (ptr->stack_pointer);
        gen_jump (ptr->while_exit);
}

/**
 * "continue" statement
 */
void docont(void) {
        WHILE *ptr; //int     *ptr;

        if ((ptr = findwhile ()) == 0)
                return;
        gen_modify_stack (ptr->stack_pointer);
        if (ptr->type == WSFOR)
                gen_jump (ptr->incr_def);
        else
                gen_jump (ptr->case_test);
}

/**
 * dump switch table
 */
void dumpsw(WHILE *ws) {
        int     i,j;

        data_segment_gdata ();
        generate_label (ws->body_tab);
        if (ws->case_test != swstp) {
                j = ws->case_test;
                while (j < swstp) {
                        gen_def_word ();
                        i = 4;
                        while (i--) {
                                output_number (swstcase[j]);
                                output_byte (',');
                                print_label (swstlab[j++]);
                                if ((i == 0) | (j >= swstp)) {
                                        newline ();
                                        break;
                                }
                                output_byte (',');
                        }
                }
        }
        gen_def_word ();
        print_label (ws->incr_def);
        output_string (",0");
        newline ();
        code_segment_gtext ();
}
