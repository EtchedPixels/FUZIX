#include <stdio.h>
#include "defs.h"
#include "data.h"

static int nextreg = 0;
static int xdirty = 0;
static int xsprel = 0;
static int xtype = 0;	/* 0 = D, 1 = S */
static int regv[8];
static int indata;

/*
 *      Some predefinitions:
 *
 *      INTSIZE is the size of an integer in the target machine
 *      BYTEOFF is the offset of an byte within an integer on the
 *              target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *              360 = 3)
 *      This compiler assumes that an integer is the SAME length as
 *      a pointer - in fact, the compiler uses INTSIZE for both.
 *
 *	For 6801 we keep the secondary as top of stack as much as possible
 *
 *	D holds the accumulator values
 *	X is used for scratch pointer work
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
    code_segment_gtext();
}

/**
 * prints new line
 * @return 
 */
void newline (void) {
    output_byte (LF);
}

void initmac(void) {
    defmac("MC6801\t1");
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
    if (!indata)
        xdirty = 1;
}

/**
 * Output a C label with leading _
 */
void output_label_name(char *p)
{
    output_byte('_');
    output_string(p);
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
    output_line ("\t.text");
    indata = 0;
}

/**
 * data segment
 */
void data_segment_gdata(void) {
    output_line ("\t.data");
    indata = 1;
}

/**
 * Output the variable symbol at scptr as an extrn or a public
 * @param scptr
 */
void ppubext(SYMBOL *scptr)  {
    if (symbol_table[current_symbol_table_idx].storage == STATIC) return;
    output_with_tab (scptr->storage == EXTERN ? ".extrn\t" : ".globl\t");
    output_string (scptr->name);
    newline();
}

/**
 * Output the function symbol at scptr as an extrn or a public
 * @param scptr
 */
void fpubext(SYMBOL *scptr) {
    if (scptr->storage == STATIC) return;
    output_with_tab (scptr->offset == FUNCTION ? ".globl\t" : ".extrn\t");
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

static void describe_access(SYMBOL *sym)
{
    if (sym->storage == REGISTER) {
        output_string("<_reg");
        output_number(sym->offset);
        newline();
        return;
    }
    if (sym->storage == AUTO || sym->storage == DEFAUTO) {
        output_decimal(sym->offset - xsprel);
        output_string(",x");
    } else if (sym->storage == LSTATIC)
        print_label(sym->offset);
    else
        output_label_name(sym->name);
    newline();
}

static void gen_tsx(int offset)
{
//    printf("GENTSX %d [D%d T%d V%d]\n", offset, xdirty, xtype, xsprel);
    /* Having a valid tsx state isn't good enoguh because it may be that
       it's out of range of or variable */
    if (xdirty == 0  && xtype == 1) {
//        printf("OFFSET %d\n", offset - stkp);
        /* Ok we have x holding a value of s but it may be out of range */
        if (offset - xsprel >= 0 && offset - xsprel < 256)
            return;
    }
    output_line("tsx");
    xdirty = 0;
    xsprel = stkp;
    xtype = 1;
    offset -= stkp;
    /* Now it gets fun - we might be out of range ? */
    if (offset > 255) {
        output_line("pshb");
        output_line("ldab #255");
        while(offset > 255) {
            output_line("abx");
            offset -= 256;
            xsprel += 256;
        }
        output_line("pulb");
    }
}

/**
 * fetch a static memory cell into the primary register
 * @param sym
 */
void gen_get_memory(SYMBOL *sym) {
    if ((sym->identity != POINTER) && (sym->type == CCHAR)) {
        output_with_tab ("ldab ");
        describe_access(sym);
        output_line("sex");
        newline ();
    } else if ((sym->identity != POINTER) && (sym->type == UCHAR)) {
        output_with_tab("ldab ");
        describe_access(sym);
        output_line("clra");
    } else {
        output_with_tab ("ldd ");
        describe_access(sym);
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
        output_line("sts _tmp");
        output_line("ldd _tmp");
        output_with_tab("add ");
        output_number(sym->offset - stkp);
        newline ();
        return HL_REG;
    }
}

/**
 * asm - store the primary register into the specified static memory cell
 * @param sym
 */
void gen_put_memory(SYMBOL *sym) {
    if ((sym->identity != POINTER) && (sym->type & CCHAR)) {
        output_with_tab("stab ");
    } else {
        output_with_tab("std ");
    }
    describe_access(sym);
}

/**
 * store the specified object type in the primary register
 * at the address in secondary register (on the top of the stack)
 * @param typeobj
 */
void gen_put_indirect(char typeobj) {
    gen_pop();
    if (typeobj & CCHAR) {
        output_line("stab ,x");
    } else {
        output_line("std ,x");
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
            /* FIXME DE cases */
            output_line("ldab ,x");
            output_line("sex");
        } else
            gen_call("_ldas_d");
    } else if (typeobj == UCHAR) {
        if (reg & DE_REG) {
            output_line("ldab ,x");
            output_line("clra");
        } else
            gen_call("_ldau_d");
    } else { // int
        if (reg & DE_REG)
            output_line("ldd ,x");
        else
            gen_call("_ldd_d");
    }
}

/**
 * platform level analysis of whether a symbol access needs to be
 * direct or indirect (globals are always direct)
 */
int gen_indirected(SYMBOL *s)
{
    if (s->storage == REGISTER || s->storage == LSTATIC)
        return 0;
    gen_tsx(s->offset);
    return 0;
}

/**
 * swap the primary and secondary registers
 */
void gen_swap(void) {
    gen_call("_swap");
}

/**
 * print partial instruction to get an immediate value into
 * the primary register
 */
void gen_immediate(void) {
    output_with_tab ("ldd ");
}

/**
 * push the primary register onto the stack
 */
void gen_push(int reg) {
    if (reg & DE_REG) {
        output_line ("pshx");
        stkp = stkp - INTSIZE;
    } else {
        output_line ("psha");	/* Check order */
        output_line ("pshb");
        stkp = stkp - INTSIZE;
    }
}

/**
 * pop the top of the stack into the secondary register
 */
void gen_pop(void) {
    output_line ("pulx");
    xdirty = 1;
    stkp = stkp + INTSIZE;
}

/**
 * swap the primary register and the top of the stack
 */
void gen_swap_stack(void) {
    output_line("std _tmp1");
    output_line("ldd ,s");
    output_line("ldx _tmp1");
    output_line("stx ,s");
    xdirty = 1;
}

/**
 * call the specified subroutine name
 * @param sname subroutine name
 */
void gen_call(char *sname) {
    output_with_tab ("jsr ");
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

void gen_prologue(void)
{
    xdirty = 1;
    nextreg = 0;
}

void gen_epilogue(void)
{
    int i;
    /* FIXME: usual case we can pop these in this order need to spot the
       sp position and optimize accordingly */
    for (i = nextreg - 1; i >= 0; i--) {
        if (stkp == regv[i]) {
            stkp += 2;
            output_line("pulx");
            xdirty = 1;
        } else {
            output_line("tsx");
            output_with_tab("ldx ");
            output_number(stkp - i);
            output_string(", x");
            newline();
            xdirty = 1;
        }
        output_with_tab("stx <_reg");
        output_number(i);
        newline();
    }
}

/**
 * return from subroutine
 */
void gen_ret(void) {
    output_line ("rts");
}

/**
 * perform subroutine call to value on top of stack
 */
void callstk(void) {
    gen_pop();
    output_line("jsr ,x");
}

/**
 * jump to specified internal label number
 * @param label the label
 */
void gen_jump(int label)
{
    output_with_tab ("jmp ");
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
    output_line ("tstd");
    if (ft)
        output_with_tab ("bne ");
    else
        output_with_tab ("beq ");
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
        if (k <= 8) {
            while(k >= 2) {
                output_line("pulx");
                xdirty = 1;
                k -= 2;
            }
            if (k == 1)
                output_line("ins");
            return newstkp;
        } else {
            output_line("tsx");
            if (k > 255) {
                output_line("ldab #255");
                while(k > 255) {
                    output_line("abx");
                    k -= 255;
                }
            }
            if (k) {
                output_with_tab("ldab ");
                output_number(k);
                newline();
                output_line("abx");
            }
            output_line("txs");
            return newstkp;
        }
    } else {
        if (k >= -16) {
            while (k <= -2) {
                output_line("pshx");
                k += 2;
            }
            if (k == -1)
                output_line("des");
            return newstkp;
        }
        /* TODO - must preserve AA */
        output_line("std _tmp");
        output_line("ldd ");
        output_number(newstkp);
        output_line("sts _tmp2");
        output_line("addd _tmp2");
        output_line("std _tmp2");
        output_line("lds _tmp2");
        output_line("ldd _tmp");
    }
    return newstkp;
}

/* FIXME */
int gen_defer_modify_stack(int newstkp)
{
    return gen_modify_stack(newstkp);
}

int gen_register(int vp, int size, int typ)
{
    return -1;
/*
    Not clear register helps at all on a 6801
    if (size > 2)
        return -1;
    if (nextreg > 8)
        return -1;
    stkp = stkp - 2;
    regv[nextreg] = stkp;
    output_with_tab("ldx <_reg");
    xdirty = 1;
    output_number(nextreg);
    newline();
    output_line("pshx");
    return nextreg++;
*/
}


/**
 * multiply the primary register by INTSIZE
 */
void gen_multiply_by_two(void) {
    output_line("aslb");
    output_line("rola");
}

/**
 * divide the primary register by INTSIZE, never used
 */
void gen_divide_by_two(void) {
    output_line("asra");
    output_line("rorb");
}

/**
 * Case jump instruction
 */
void gen_jump_case(void) {
    output_with_tab ("jmp _case");
    newline ();
}

/**
 * add the primary and secondary registers
 * if lval2 is int pointer and lval is not, scale lval
 * @param lval
 * @param lval2
 */
void gen_add(LVALUE *lval, LVALUE *lval2) {
    if (dbltest (lval2, lval)) {
        output_line("addd ,s");
        output_line("addd ,s");
    } else
        output_line ("addd ,s");
    gen_pop();
}

/**
 * subtract the primary register from the secondary
 */
void gen_sub(void) {
    gen_pop();
    gen_call("_sub");
}

/**
 * multiply the primary and secondary registers (result in primary)
 */
void gen_mult(void) {
    gen_pop();
    gen_call("_mul");
}

/**
 * divide the secondary register by the primary
 * (quotient in primary, remainder in secondary)
 */
void gen_div(void) {
    gen_pop();
    gen_call("_div");
}

/**
 * unsigned divide the secondary register by the primary
 * (quotient in primary, remainder in secondary)
 */
void gen_udiv(void) {
    gen_pop();
    gen_call("_udiv");
}

/**
 * compute the remainder (mod) of the secondary register
 * divided by the primary register
 * (remainder in primary, quotient in secondary)
 */
void gen_mod(void) {
    gen_pop();
    gen_call("_mod");
}

/**
 * compute the remainder (mod) of the secondary register
 * divided by the primary register
 * (remainder in primary, quotient in secondary)
 */
void gen_umod(void) {
    gen_pop();
    gen_call("_umod");
}

/**
 * inclusive 'or' the primary and secondary registers
 */
void gen_or(void) {
    output_line("orab ,s");
    output_line("oraa 1,s");
    gen_pop();
}

/**
 * exclusive 'or' the primary and secondary registers
 */
void gen_xor(void) {
    output_line("eorb ,s");
    output_line("eora 1,s");
    gen_pop();
}

/**
 * 'and' the primary and secondary registers
 */
void gen_and(void) {
    output_line("andb ,s");
    output_line("anda 1,s");
    gen_pop();
}

/**
 * arithmetic shift right the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_arithm_shift_right(void) {
    gen_pop();
    gen_call("_asr");
}

/**
 * logically shift right the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_logical_shift_right(void) {
    gen_pop();
    gen_call("_asl");
}

/**
 * arithmetic shift left the secondary register the number of
 * times in the primary register (results in primary register)
 */
void gen_arithm_shift_left(void) {
    gen_pop ();
    gen_call("_lsl");
}

/**
 * two's complement of primary register
 */
void gen_twos_complement(void) {
    gen_call("_neg");
}

/**
 * logical complement of primary register
 */
void gen_logical_negation(void) {
    gen_call("_lneg");
}

/**
 * one's complement of primary register
 */
void gen_complement(void) {
    output_line("coma");
    output_line("comb");
}

/**
 * Convert primary value into logical value (0 if 0, 1 otherwise)
 */
void gen_convert_primary_reg_value_to_bool(void) {
    gen_call("_bool");
}

/**
 * increment the primary register by 1 if char, INTSIZE if int
 */
void gen_increment_primary_reg(LVALUE *lval) {
    switch (lval->ptr_type) {
        case STRUCT:
            output_with_tab("addd ");
            output_number(lval->tagsym->size);
            newline();
            break ;
        case CINT:
        case UINT:
            output_line("addd #2");
            break;
        default:
            output_line("addd #1");
            break;
    }
}

/**
 * decrement the primary register by one if char, INTSIZE if int
 */
void gen_decrement_primary_reg(LVALUE *lval) {
    switch (lval->ptr_type) {
        case CINT:
        case UINT:
            output_line("subd #2");
            break;
        case STRUCT:
            output_with_tab("subd ");
            output_number(lval->tagsym->size - 1);
            newline();
            break ;
        default:
            output_line("subd #1");
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
    gen_call("_eq");
}

/**
 * not equal
 */
void gen_not_equal(void) {
    gen_pop();
    gen_call("_ne");
}

/**
 * less than (signed)
 */
void gen_less_than(void) {
    gen_pop();
    gen_call("_lt");
}

/**
 * less than or equal (signed)
 */
void gen_less_or_equal(void) {
    gen_pop();
    gen_call("_le");
}

/**
 * greater than (signed)
 */
void gen_greater_than(void) {
    gen_pop();
    gen_call("_gt");
}

/**
 * greater than or equal (signed)
 */
void gen_greater_or_equal(void) {
    gen_pop();
    gen_call("_ge");
}

/**
 * less than (unsigned)
 */
void gen_unsigned_less_than(void) {
    gen_pop();
    gen_call("_ult");
}

/**
 * less than or equal (unsigned)
 */
void gen_unsigned_less_or_equal(void) {
    gen_pop();
    gen_call("_ule");
}

/**
 * greater than (unsigned)
 */
void gen_usigned_greater_than(void) {
    gen_pop();
    gen_call("_ugt");
}

/**
 * greater than or equal (unsigned)
 */
void gen_unsigned_greater_or_equal(void) {
    gen_pop();
    gen_call("_uge");
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
    output_with_tab ("ldaa ");
    output_number(d);
    newline ();
}

/**
 * print partial instruction to get an immediate value into
 * the secondary register
 */
void gen_immediate2(void) {
    output_with_tab ("ldx ");
    xdirty = 1;
}

/**
 * add offset to primary register
 * @param val the value
 */
void add_offset(int val) {
    output_with_tab("addd ");
    output_number(val);
    newline();
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
            output_line("std _tmp");
            output_line("addd _tmp");
            break;
        case STRUCT:
            /* Might be smarter to pshx/pulx ? */
            output_with_tab("ldx ");
            output_number(size);
            newline();
            xdirty = 1;
            gen_call("_muld");
            break;
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

