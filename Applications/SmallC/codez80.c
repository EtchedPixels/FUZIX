#include <stdio.h>
#include "defs.h"
#include "data.h"

/*
 *      Some predefinitions:
 *
 *      INTSIZE is the size of an integer in the target machine
 *      BYTEOFF is the offset of an byte within an integer on the
 *              target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *              360 = 3)
 *      This compiler assumes that an integer is the SAME length as
 *      a pointer - in fact, the compiler uses INTSIZE for both.
 */

/**
 * print all assembler info before any code is generated
 */
void header (void) {
    output_string ("; Small C Z80\n;\tCoder (ac0)\n;");
    frontend_version();
    newline ();
    output_line ("\t;program area SMALLC_GENERATED is RELOCATABLE");
    output_line ("\t.module SMALLC_GENERATED");
    output_line ("\t.list   (err, loc, bin, eqt, cyc, lin, src, lst, md)");
    output_line ("\t.nlist  (pag)");
}

/**
 * prints new line
 * @return 
 */
void newline (void) {
#if __CYGWIN__ == 1
    output_byte (CR);
#endif
    output_byte (LF);
}

void initmac(void) {
    defmac("cpm\t1");
    defmac("Z80\t1");
    defmac("smallc\t1");
}

/**
 * Output internal generated label prefix
 */
void output_label_prefix(void) {
    output_byte('$');
}

/**
 * Output a label definition terminator
 */
void output_label_terminator (void) {
    output_byte (':');
}

/**
 * begin a comment line for the assembler
 */
void gen_comment(void) {
    output_byte (';');
}

/**
 * print any assembler stuff needed after all code
 */
void trailer(void) {
    output_line (";\t.end");
}

/**
 * text (code) segment
 */
void code_segment_gtext(void) {
    output_line ("\t.area  SMALLC_GENERATED  (REL,CON,CSEG)");
}

/**
 * data segment
 */
void data_segment_gdata(void) {
    output_line ("\t.area  SMALLC_GENERATED_DATA  (REL,CON,DSEG)");
}

/**
 * Output the variable symbol at scptr as an extrn or a public
 * @param scptr
 */
void ppubext(SYMBOL *scptr)  {
    if (symbol_table[current_symbol_table_idx].storage == STATIC) return;
    output_with_tab (scptr->storage == EXTERN ? ";extrn\t" : ".globl\t");
    output_string (scptr->name);
    newline();
}

/**
 * Output the function symbol at scptr as an extrn or a public
 * @param scptr
 */
void fpubext(SYMBOL *scptr) {
    if (scptr->storage == STATIC) return;
    output_with_tab (scptr->offset == FUNCTION ? ".globl\t" : ";extrn\t");
    output_string (scptr->name);
    newline ();
}

/**
 * Output a decimal number to the assembler file, with # prefix
 * @param num
 */
void output_number(int num) {
    output_byte('#');
    output_decimal(num);
}

static void output_bracketed(char *p)
{
    output_byte('(');
    output_string(p);
    output_byte(')');
}

/**
 * fetch a static memory cell into the primary register
 * @param sym
 */
void gen_get_memory(SYMBOL *sym) {
    if ((sym->identity != POINTER) && (sym->type == CCHAR)) {
        output_with_tab ("ld a,");
        output_bracketed(sym->name);
        newline ();
        gen_call ("ccsxt");
    } else if ((sym->identity != POINTER) && (sym->type == UCHAR)) {
        output_with_tab("ld a,");
        output_bracketed(sym->name);
        newline();
        output_line("ld l,a");
        output_line("ld h,0");
    } else {
        output_with_tab ("ld hl,");
        output_bracketed(sym->name);
        newline ();
    }
}

/**
 * asm - fetch the address of the specified symbol into the primary register
 * @param sym the symbol name
 * @return which register pair contains result
 */
int gen_get_locale(SYMBOL *sym) {
    if (sym->storage == LSTATIC) {
        gen_immediate();
        output_byte('(');
        print_label(sym->offset);
        output_byte(')');
        newline();
        return HL_REG;
    } else {
        if (uflag && !(sym->identity == ARRAY)) {// ||
                //(sym->identity == VARIABLE && sym->type == STRUCT))) {
            output_with_tab("ldsi\t");
            output_number(sym->offset - stkp);
            newline ();
            return DE_REG;
        } else {
            gen_immediate();
            output_number(sym->offset - stkp);
            newline ();
            output_line ("add hl, sp");
            return HL_REG;
        }
    }
}

/**
 * asm - store the primary register into the specified static memory cell
 * @param sym
 */
void gen_put_memory(SYMBOL *sym) {
    if ((sym->identity != POINTER) && (sym->type & CCHAR)) {
        output_line ("ld a,l");
        output_with_tab ("ld ");
        output_bracketed(sym->name);
        output_string(",a");
    } else {
        output_with_tab("ld ");
        output_bracketed(sym->name);
        output_string(",hl");
    }
    newline ();
}

/**
 * store the specified object type in the primary register
 * at the address in secondary register (on the top of the stack)
 * @param typeobj
 */
void gen_put_indirect(char typeobj) {
    gen_pop ();
    if (typeobj & CCHAR) {
        //gen_call("ccpchar");
        output_line("ld a,l");
        output_line("ld (de),a");
    } else {
        gen_call("ccpint");
    }
}

/**
 * fetch the specified object type indirect through the primary
 * register into the primary register
 * @param typeobj object type
 */
void gen_get_indirect(char typeobj, int reg) {
    if (typeobj == CCHAR) {
        if (reg & DE_REG) {
            gen_swap();
        }
        gen_call("ccgchar");
    } else if (typeobj == UCHAR) {
        if (reg & DE_REG) {
            gen_swap();
        }
        //gen_call("cguchar");
        output_line("ld l,(hl)");
        output_line("ld h,0");
    } else { // int
        if (uflag) {
            if (reg & HL_REG) {
                gen_swap();
            }
            output_line("lhlx");
        } else {
            output_line("ld a,(hl)");
            output_line("inc hl");
            output_line("ld h,(hl)");
            output_line("ld l,a");
        }
    }
}

/**
 * swap the primary and secondary registers
 */
void gen_swap(void) {
    output_line("ex de,hl");
}

/**
 * print partial instruction to get an immediate value into
 * the primary register
 */
void gen_immediate(void) {
    output_with_tab ("ld hl,");
}

/**
 * push the primary register onto the stack
 */
void gen_push(int reg) {
    if (reg & DE_REG) {
        output_line ("push de");
        stkp = stkp - INTSIZE;
    } else {
        output_line ("push hl");
        stkp = stkp - INTSIZE;
    }
}

/**
 * pop the top of the stack into the secondary register
 */
void gen_pop(void) {
    output_line ("pop de");
    stkp = stkp + INTSIZE;
}

/**
 * swap the primary register and the top of the stack
 */
void gen_swap_stack(void) {
    output_line ("ex (sp),hl");
}

/**
 * call the specified subroutine name
 * @param sname subroutine name
 */
void gen_call(char *sname) {
    output_with_tab ("call ");
    output_string (sname);
    newline ();
}

/**
 * declare entry point
 */
void declare_entry_point(char *symbol_name) {
    output_string(symbol_name);
    output_label_terminator();
    //newline();
}

/**
 * return from subroutine
 */
void gen_ret(void) {
    output_line ("ret");
}

/**
 * perform subroutine call to value on top of stack
 */
void callstk(void) {
    gen_immediate ();
    output_string ("#.+5");
    newline ();
    gen_swap_stack ();
    output_line ("jp (hl)");
    stkp = stkp + INTSIZE;
}

/**
 * jump to specified internal label number
 * @param label the label
 */
void gen_jump(int label)
{
    output_with_tab ("jp ");
    print_label (label);
    newline ();
}

/**
 * test the primary register and jump if false to label
 * @param label the label
 * @param ft if true jnz is generated, jz otherwise
 */
void gen_test_jump(int label, int ft)
{
    output_line ("ld a,h");
    output_line ("or l");
    if (ft)
        output_with_tab ("jp nz,");
    else
        output_with_tab ("jp z,");
    print_label (label);
    newline ();
}

/**
 * print pseudo-op  to define a byte
 */
void gen_def_byte(void) {
    output_with_tab (".db ");
}

/**
 * print pseudo-op to define storage
 */
void gen_def_storage(void) {
    output_with_tab (".ds ");
}

/**
 * print pseudo-op to define a word
 */
void gen_def_word(void) {
    output_with_tab (".dw ");
}

/**
 * modify the stack pointer to the new value indicated
 * @param newstkp new value
 */
int gen_modify_stack(int newstkp) {
    int k;

    k = newstkp - stkp;
    if (k == 0)
        return (newstkp);
    if (k > 0) {
        if (k < 7) {
            if (k & 1) {
                output_line ("inc sp");
                k--;
            }
            while (k) {
                output_line ("pop bc");
                k = k - INTSIZE;
            }
            return (newstkp);
        }
    } else {
        if (k > -7) {
            if (k & 1) {
                output_line ("dec sp");
                k++;
            }
            while (k) {
                output_line ("push bc");
                k = k + INTSIZE;
            }
            return (newstkp);
        }
    }
    gen_swap ();
    gen_immediate ();
    output_number (k);
    newline ();
    output_line ("dec sp");
    output_line ("ld sp, hl");
    gen_swap ();
    return (newstkp);
}

/**
 * multiply the primary register by INTSIZE
 */
void gen_multiply_by_two(void) {
    output_line ("add hl,hl");
}

/**
 * divide the primary register by INTSIZE, never used
 */
void gen_divide_by_two(void) {
    gen_push(HL_REG);        /* push primary in prep for gasr */
    gen_immediate ();
    output_number (1);
    newline ();
    gen_arithm_shift_right ();  /* divide by two */
}

/**
 * Case jump instruction
 */
void gen_jump_case(void) {
    output_with_tab ("jp cccase");
    newline ();
}

/**
 * add the primary and secondary registers
 * if lval2 is int pointer and lval is not, scale lval
 * @param lval
 * @param lval2
 */
void gen_add(LVALUE *lval, LVALUE *lval2) {
    gen_pop ();
    if (dbltest (lval2, lval)) {
        gen_swap ();
        gen_multiply_by_two();
        gen_swap ();
    }
    output_line ("add hl,de");
}

/**
 * subtract the primary register from the secondary
 */
void gen_sub(void) {
    gen_pop ();
    gen_call ("ccsub");
}

/**
 * multiply the primary and secondary registers (result in primary)
 */
void gen_mult(void) {
    gen_pop();
    gen_call ("ccmul");
}

/**
 * divide the secondary register by the primary
 * (quotient in primary, remainder in secondary)
 */
void gen_div(void) {
    gen_pop();
    gen_call ("ccdiv");
}

/**
 * unsigned divide the secondary register by the primary
 * (quotient in primary, remainder in secondary)
 */
void gen_udiv(void) {
    gen_pop();
    gen_call ("ccudiv");
}

/**
 * compute the remainder (mod) of the secondary register
 * divided by the primary register
 * (remainder in primary, quotient in secondary)
 */
void gen_mod(void) {
    gen_div ();
    gen_swap ();
}

/**
 * compute the remainder (mod) of the secondary register
 * divided by the primary register
 * (remainder in primary, quotient in secondary)
 */
void gen_umod(void) {
    gen_udiv ();
    gen_swap ();
}

/**
 * inclusive 'or' the primary and secondary registers
 */
void gen_or(void) {
    gen_pop();
    gen_call ("ccor");
}

/**
 * exclusive 'or' the primary and secondary registers
 */
void gen_xor(void) {
    gen_pop();
    gen_call ("ccxor");
}

/**
 * 'and' the primary and secondary registers
 */
void gen_and(void) {
    gen_pop();
    gen_call ("ccand");
}

/**
 * arithmetic shift right the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_arithm_shift_right(void) {
    gen_pop();
    gen_call ("ccasr");
}

/**
 * logically shift right the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_logical_shift_right(void) {
    gen_pop();
    gen_call ("cclsr");
}

/**
 * arithmetic shift left the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_arithm_shift_left(void) {
    gen_pop ();
    gen_call ("ccasl");
}

/**
 * two's complement of primary register
 */
void gen_twos_complement(void) {
    gen_call ("ccneg");
}

/**
 * logical complement of primary register
 */
void gen_logical_negation(void) {
    gen_call ("cclneg");
}

/**
 * one's complement of primary register
 */
void gen_complement(void) {
    gen_call ("cccom");
}

/**
 * Convert primary value into logical value (0 if 0, 1 otherwise)
 */
void gen_convert_primary_reg_value_to_bool(void) {
    gen_call ("ccbool");
}

/**
 * increment the primary register by 1 if char, INTSIZE if int
 */
void gen_increment_primary_reg(LVALUE *lval) {
    switch (lval->ptr_type) {
        case STRUCT:
            gen_immediate2();
            output_number(lval->tagsym->size);
            newline();
            output_line("add hl,de");
            break ;
        case CINT:
        case UINT:
            output_line("inc hl");
        default:
            output_line("inc hl");
            break;
    }
}

/**
 * decrement the primary register by one if char, INTSIZE if int
 */
void gen_decrement_primary_reg(LVALUE *lval) {
    output_line("dec hl");
    switch (lval->ptr_type) {
        case CINT:
        case UINT:
            output_line("dec hl");
            break;
        case STRUCT:
            gen_immediate2();
            output_number(lval->tagsym->size - 1);
            newline();
            // two's complement
            output_line("ld a,d");
            output_line("cpl");
            output_line("ld d,a");
            output_line("ld a,e");
            output_line("cpl");
            output_line("ld e,a");
            output_line("inc de");
            // substract
            output_line("add hl,de");
            break ;
        default:
            break;
    }
}

/**
 * following are the conditional operators.
 * they compare the secondary register against the primary register
 * and put a literal 1 in the primary if the condition is true,
 * otherwise they clear the primary register
 */

/**
 * equal
 */
void gen_equal(void) {
    gen_pop();
    gen_call ("cceq");
}

/**
 * not equal
 */
void gen_not_equal(void) {
    gen_pop();
    gen_call ("ccne");
}

/**
 * less than (signed)
 */
void gen_less_than(void) {
    gen_pop();
    gen_call ("cclt");
}

/**
 * less than or equal (signed)
 */
void gen_less_or_equal(void) {
    gen_pop();
    gen_call ("ccle");
}

/**
 * greater than (signed)
 */
void gen_greater_than(void) {
    gen_pop();
    gen_call ("ccgt");
}

/**
 * greater than or equal (signed)
 */
void gen_greater_or_equal(void) {
    gen_pop();
    gen_call ("ccge");
}

/**
 * less than (unsigned)
 */
void gen_unsigned_less_than(void) {
    gen_pop();
    gen_call ("ccult");
}

/**
 * less than or equal (unsigned)
 */
void gen_unsigned_less_or_equal(void) {
    gen_pop();
    gen_call ("ccule");
}

/**
 * greater than (unsigned)
 */
void gen_usigned_greater_than(void) {
    gen_pop();
    gen_call ("ccugt");
}

/**
 * greater than or equal (unsigned)
 */
void gen_unsigned_greater_or_equal(void) {
    gen_pop();
    gen_call ("ccuge");
}

char *inclib(void) {
#ifdef  cpm
        return("B:");
#endif
#ifdef  unix
#ifdef  INCDIR
        return(INCDIR);
#else
        return "";
#endif
#endif
}

/**
 * Squirrel away argument count in a register that modstk doesn't touch.
 * @param d
 */
void gnargs(int d)
{
    output_with_tab ("ld a,");
    output_number(d);
    newline ();
}

/**
 * print partial instruction to get an immediate value into
 * the secondary register
 */
void gen_immediate2(void) {
    output_with_tab ("ld de,");
}

/**
 * add offset to primary register
 * @param val the value
 */
void add_offset(int val) {
    gen_immediate2();
    output_number(val);
    newline();
    output_line ("add hl,de");
}

/**
 * multiply the primary register by the length of some variable
 * @param type
 * @param size
 */
void gen_multiply(int type, int size) {
	switch (type) {
        case CINT:
        case UINT:
            gen_multiply_by_two();
            break;
        case STRUCT:
            gen_immediate2();
            output_number(size);
            newline();
            gen_call("ccmul");
            break ;
        default:
            break;
    }
}
