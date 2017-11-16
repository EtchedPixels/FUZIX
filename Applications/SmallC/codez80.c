#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "data.h"

static int nextreg = 0;
static int regv[8];

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
    output_line("\t.code");
}

/**
 * prints new line
 * @return 
 */
void newline (void) {
    output_byte (LF);
}

void initmac(void) {
}

/**
 * Output internal generated label prefix
 */
void output_label_prefix(void) {
    output_byte('L');
}

/**
 * Output a label with leading _
 */

void output_label_name(char *p)
{
    output_byte('_');
    output_string(p);
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
    output_line ("\t.code");
}

/**
 * data segment
 */
void data_segment_gdata(void) {
    output_line ("\t.data");
}

/**
 * Output the variable symbol at scptr as an extrn or a public
 * @param scptr
 */
void ppubext(SYMBOL *scptr)  {
    if (symbol_table[current_symbol_table_idx].storage == STATIC) return;
    if (scptr->storage != EXTERN) {
        output_with_tab (".export\t");
        output_label_name (scptr->name);
        newline();
    }
}

/**
 * Output the function symbol at scptr as an extrn or a public
 * @param scptr
 */
void fpubext(SYMBOL *scptr) {
    if (scptr->storage == STATIC) return;
    if (scptr->offset == FUNCTION) {
        output_with_tab(".export\t");
        output_label_name (scptr->name);
        newline ();
    }
}

/**
 * Output a decimal number to the assembler file, with # prefix
 * @param num
 */
void output_number(int num) {
    output_decimal(num);
}

static void output_number_signed(int num)
{
    if (num >= 0)
        output_byte('+');
    output_decimal(num);
}

static void describe_access(SYMBOL *sym, int s)
{
    if (sym->storage == REGISTER) {
        if (s == 2)
            output_string("bc");
        else
            output_string("c");
    } else if (sym->storage == LSTATIC) {
        output_byte('(');
        print_label(sym->offset);
        output_byte(')');
    } else if (sym->storage == AUTO || sym->storage == DEFAUTO) {
        output_string("(ix");
        output_number_signed(sym->offset + 2);
        output_byte(')');
    } else {
        output_byte('(');
        output_label_name(sym->name);
        output_byte(')');
    }
}


static void describe_access_hl(SYMBOL *sym, int d)
{
    if (sym->storage == REGISTER) {
        if (d) {
            output_line("ld h,b");
            output_line("ld l,c");
        } else {
            output_line("ld b,h");
            output_line("ld c,l");
        }
        return;
    }
    if (sym->storage != AUTO && sym->storage != DEFAUTO) {
        output_with_tab("ld hl,");
        describe_access(sym, 2);
        newline();
        return;
    }
    if (d) {
        output_with_tab("ld l,(ix");
        output_number_signed(sym->offset + 2 );
        output_byte(')');
        newline();
        output_with_tab("ld h,(ix");
        output_number_signed(sym->offset + 3);
        output_byte(')');
        newline();
    } else {
        output_with_tab("ld (ix");
        output_number_signed(sym->offset + 2);
        output_string("),l");
        newline();
        output_with_tab("ld (ix");
        output_number_signed(sym->offset + 3);
        output_string("),h");
        newline();
    }
}

/**
 * fetch a static memory cell into the primary register
 * @param sym
 */
void gen_get_memory(SYMBOL *sym) {
    if ((sym->identity != POINTER) && (sym->type == CCHAR)) {
        output_with_tab ("ld a,");
        describe_access(sym, 1);
        newline();
        gen_call ("ccsxt");
    } else if ((sym->identity != POINTER) && (sym->type == UCHAR)) {
        output_with_tab("ld a,");
        describe_access(sym, 1);
        newline();
        output_line("ld l,a");
        output_line("ld h,0");
    } else {
        describe_access_hl(sym,1);
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
        print_label(sym->offset);
        newline();
        return HL_REG;
    } else {
        gen_immediate();
        output_number(sym->offset - stkp);
/*        output_string(" ; ");
        output_number(sym->offset + 2);*/
        newline ();
        output_line ("add hl,sp");
        return HL_REG;
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
        describe_access(sym, 1);
        output_string(",a");
    } else {
        describe_access_hl(sym, 0);
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
        output_line("ld a,(hl)");
        output_line("inc hl");
        output_line("ld h,(hl)");
        output_line("ld l,a");
    }
}

/**
 * platform level analysis of whether a symbol access needs to be
 * direct or indirect (globals are always direct)
 */
int gen_indirected(SYMBOL *s)
{
    if (s->storage == LSTATIC || s->storage == REGISTER)
        return 0;
    if (abs(s->offset) < 124)
        return 0;
    return 1;
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
    output_label_name (sname);
    newline ();
}

/**
 * declare entry point
 */
void declare_entry_point(char *symbol_name) {
    output_label_name(symbol_name);
    output_label_terminator();
    newline();
}

void gen_prologue(void)
{
    output_line("push ix");
    output_line("ld ix,0");
    output_line("add ix,sp");
    stkp = stkp - INTSIZE;
    nextreg = 0;
}

void gen_epilogue(void)
{
    if (nextreg)
        gen_modify_stack(regv[0]);
    if (nextreg)
        output_line("pop bc");
    stkp += 2;
    output_line("ld sp,ix");
    output_line("pop ix");
    stkp = 0;
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
    gen_pop();
    gen_call("callhl");		/* call to a jp(hl) */
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
    output_with_tab (".byte ");
}

/**
 * print pseudo-op to define storage
 */
void gen_def_storage(void) {
    output_with_tab (".blkb ");
}

/**
 * print pseudo-op to define a word
 */
void gen_def_word(void) {
    output_with_tab (".word ");
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
                output_line ("pop af");
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

/* FIXME */
int gen_defer_modify_stack(int newstkp)
{
    return gen_modify_stack(newstkp);
}

int gen_register(int vp, int size, int typ)
{
    if (size != 2)
        return -1;
    if (nextreg > 0)
        return -1;
    stkp = stkp - 2;
    regv[nextreg] = stkp;
    output_line("push bc");
    return nextreg++;
}

/**
 * multiply the primary register by INTSIZE
 */
void gen_multiply_by_two(void) {
    output_line ("add hl,hl");
}

/**
 * divide the primary register by INTSIZE
 */
void gen_divide_by_two(void) {
    output_line("srl h");
    output_line("rr l");
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
    if (dbltest (lval2, lval))
        output_line("add hl,de");
    output_line ("add hl,de");
}

/**
 * subtract the primary register from the secondary
 */
void gen_sub(void) {
    gen_pop ();
    output_line("ex de,hl");
    output_line("or a");
    output_line("sbc hl,de");
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

/**
 * To help the optimizer know when r1/r2 are discardable
 */
void gen_statement_end(void)
{
    output_line(";end");
}
