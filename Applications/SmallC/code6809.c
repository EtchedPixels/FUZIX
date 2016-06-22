/*      File codeas09.c: 2.2 (84/08/31,10:05:13) */
/*% cc -O -c %
 *
 *	THIS IS FROM AN OLDER VERSION OF THE COMPILER: WILL NEED PORTING
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

#ifdef M6809

/*      Define ASNM and LDNM to the names of the assembler and linker
        respectively */

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
#define INTSIZE 2
#define BYTEOFF 1

void tab(void)
{
        output_byte('\t');
}

void ot(char ptr[])
{
        tab ();
        output_string (ptr);

}

void ol(char ptr[])
{
        ot (ptr);
        newline();
}



/*
 *      print all assembler info before any code is generated
 *
 */
void header(void)
{
        output_string("|\tSmall C MC6809\n|\tCoder (2.4,84/11/27)\n|");
        frontend_version();
        newline();
        ol (".globl\tsmul,sdiv,smod,asr,asl,neg,lneg,case");
        ol (".globl\teq,ne,lt,le,gt,ge,ult,ule,ugt,uge,bool");

}


void initmac(void) {
        defmac("mc6809\t1");
        defmac("mitas09\t1");
        defmac("smallc\t1");

}

int galign(int t)
{
        return (t);

}

/*
 *      return size of an integer
 */
int intsize(void) {
        return(INTSIZE);

}

/*
 *      return offset of ls byte within word
 *      (ie: 8080 & pdp11 is 0, 6809 is 1, 360 is 3)
 */
int byteoff(void) {
        return(BYTEOFF);
}

/*
 *      Output internal generated label prefix
 */
void olprfix(void) {
        output_string("LL");

}

/*
 *      Output a label definition terminator
 */
void col(void)
{
        output_string ("=.\n");

}

/*
 *      begin a comment line for the assembler
 *
 */
void comment(void)
{
        output_byte ('|');

}

/*
 *      Output a prefix in front of user labels
 */
void prefix(void) {
        output_byte('_');

}

/*
 *      print any assembler stuff needed after all code
 *
 */
void trailer(void)
{
        ol (".end");

}

/*
 *      function prologue
 */
void prologue(void)
{

}

/*
 *      text (code) segment
 */

void gtext(void)
{
        ol (".text");

}

/*
 *      data segment
 */
void gdata(void)
{
        ol (".data");

}

/*
 *  Output the variable symbol at scptr as an extrn or a public
 */
void ppubext(char *scptr)
{
        if (scptr[STORAGE] == STATIC)
                return;
        ot (".globl\t");
        prefix ();
        output_string (scptr);
        nl();

}

/*
 * Output the function symbol at scptr as an extrn or a public
 */
void fpubext(char *scptr) {
        ppubext(scptr);

}

/*
 *  Output a decimal number to the assembler file
 */
void onum(int num) {
        output_decimal(num);    /* pdp11 needs a "." here */
        output_byte('.');
}

/*
 *      fetch a static memory cell into the primary register
 */

void getmem(char *sym)
{
        if ((sym[IDENT] != POINTER) & (sym[TYPE] == CCHAR)) {
                ot ("ldb\t");
                prefix ();
                output_string (sym + NAME);
                newline();
                ot ("sex");
                newline();
        } else {
                ot ("ldd\t");
                prefix ();
                output_string (sym + NAME);
                newline();
        }
}

/*
 *      fetch the address of the specified symbol into the primary register
 *
 */
void getloc(char *sym)
{
        if (sym[STORAGE] == LSTATIC) {
                immed();
                printlabel(glint(sym));
                nl();
        } else {
                ot ("leay\t");
                onum (glint(sym) - stkp);
                output_string ("(s)\n\ttfr\ty,d\n");
        }

}

/*
 *      store the primary register into the specified static memory cell
 *
 */
void putmem (char *sym)
{
        if ((sym[IDENT] != POINTER) & (sym[TYPE] == CCHAR)) {
                ot ("stb\t");
        } else
                ot ("std\t");
        prefix ();
        output_string (sym + NAME);
        newline();

}

/*
 *      store the specified object type in the primary register
 *      at the address on the top of the stack
 *
 */
void putstk(char typeobj)
{
        if (typeobj == CCHAR)
                ol ("stb\t@(s)++");
        else
                ol ("std\t@(s)++");
        stkp = stkp + INTSIZE;

}

/*
 *      fetch the specified object type indirect through the primary
 *      register into the primary register
 *
 */
void indirect (char typeobj)
{
        ol("tfr\td,y");
        if (typeobj == CCHAR)
                ol ("ldb\t(y)\n\tsex");
        else
                ol ("ldd\t(y)");

}

/*
 *      swap the primary and secondary registers
 *
 */
void swap(void)
{
        ol ("exg\td,x");

}

/*
 *      print partial instruction to get an immediate value into
 *      the primary register
 *
 */
void immed(void)
{
        ot ("ldd\t#");

}

/*
 *      push the primary register onto the stack
 *
 */
void gpush(void)
{
        ol ("pshs\td");
        stkp = stkp - INTSIZE;

}

/*
 *      pop the top of the stack into the secondary register
 *
 */
void gpop(void)
{
        ol ("puls\td");
        stkp = stkp + INTSIZE;

}

/*
 *      swap the primary register and the top of the stack
 *
 */
void swapstk(void)
{
        ol ("ldy\t(s)\nstd\t(s)\n\ttfr\ty,d");

}

/*
 *      call the specified subroutine name
 *
 */
void gcall(char *sname)
{
        ot ("jsr\t");
        if (*sname == '^')
                output_string (++sname);
        else {
                prefix ();
                output_string (sname);
        }
        newline();

}

/*
 *      return from subroutine
 *
 */
void gret(void)
{
        ol ("rts");

}

/*
 *      perform subroutine call to value on top of stack
 *
 */
void callstk(void)
{
        gpop();
        ol("jsr\t(x)");

}

/*
 *      jump to specified internal label number
 *
 */
void jump(int label)
{
        ot ("lbra\t");
        printlabel(label);
        nl();

}

/*
 *      test the primary register and jump if false to label
 *
 */
void testjump (int label, int ft)
{
        ol ("cmpd\t#0");
        if (ft)
                ot ("lbne\t");
        else
                ot ("lbeq\t");
        printlabel (label);
        newline();

}

/*
 *      print pseudo-op  to define a byte
 *
 */
void defbyte(void)
{
        ot (".byte\t");

}

/*
 *      print pseudo-op to define storage
 *
 */
void defstorage(void)
{
        ot (".blkb\t");

}

/*
 *      print pseudo-op to define a word
 *
 */

void defword(void)
{
        ot (".word\t");

}

/*
 *      modify the stack pointer to the new value indicated
 *
 */
void modstk(int newstkp)
{
        int     k;

        k = galign(newstkp - stkp);
        if (k == 0)
                return (newstkp);
        ot ("leas\t");
        onum (k);
        output_string ("(s)\n");
        return (newstkp);

}

/*
 *      multiply the primary register by INTSIZE
 */
void gaslint(void)
{
        ol ("aslb\n\trola");

}

/*
 *      divide the primary register by INTSIZE
 */
void gasrint(void)
{
        ol ("asra\n\trorb");

}

/*
 *      Case jump instruction
 */
void gjcase(void) {
        ot ("jmp\tcase");
        newline();

}

/*
 *      add the primary and secondary registers
 *      if lval2 is int pointer and lval is int, scale lval
 */
void gadd (LVALUE *lval, LVALUE *lval2)
{
        if (dbltest (lval2, lval)) {
                ol ("asl\t1(s)\n\trol\t(s)");
        }
        ol ("addd\t(s)++");
        stkp = stkp + INTSIZE;

}

/*
 *      subtract the primary register from the secondary
 *
 */
void gsub(void)
{
        ol ("subd\t(s)++\n\tcoma\n\tcomb\n\taddd\t#1");
        stkp = stkp + INTSIZE;

}

/*
 *      multiply the primary and secondary registers
 *      (result in primary)
 *
 */
void gmult(void)
{
        gcall ("^smul");
        stkp = stkp + INTSIZE;

}

/*
 *      divide the secondary register by the primary
 *      (quotient in primary, remainder in secondary)
 *
 */
void gdiv(void)
{
        gcall ("^sdiv");
        stkp = stkp + INTSIZE;

}

/*
 *      compute the remainder (mod) of the secondary register
 *      divided by the primary register
 *      (remainder in primary, quotient in secondary)
 *
 */
void gmod(void)
{
        gcall ("^smod");
        stkp = stkp + INTSIZE;

}

/*
 *      inclusive 'or' the primary and secondary registers
 *
 */
void gor(void)
{
        ol ("ora\t(s)+\n\torb\t(s)+");
        stkp = stkp + INTSIZE;

}

/*
 *      exclusive 'or' the primary and secondary registers
 *
 */
void gxor(void)
{
        ol ("eora\t(s)+\n\teorb\t(s)+");
        stkp = stkp + INTSIZE;

}

/*
 *      'and' the primary and secondary registers
 *
 */
void gand(void)
{
        ol ("anda\t(s)+\n\tandb\t(s)+");
        stkp = stkp + INTSIZE;

}

/*
 *      arithmetic shift right the secondary register the number of
 *      times in the primary register
 *      (results in primary register)
 *
 */
void gasr(void)
{
        gcall ("^asr");
        stkp = stkp + INTSIZE;

}

/*
 *      arithmetic shift left the secondary register the number of
 *      times in the primary register
 *      (results in primary register)
 *
 */
void gasl(void)
{
        gcall ("^asl");
        stkp = stkp + INTSIZE;

}

/*
 *      two's complement of primary register
 *
 */
void gneg(void)
{
        gcall ("^neg");

}

/*
 *      logical complement of primary register
 *
 */
void glneg(void)
{
        gcall ("^lneg");

}

/*
 *      one's complement of primary register
 *
 */
void gcom(void)
{
        ol ("coma\n\tcomb");

}

/*
 *      convert primary register into logical value
 *
 */
void gbool(void)
{
        gcall ("^bool");
}

/*
 *      increment the primary register by 1 if char, INTSIZE if
 *      int
 */
void ginc (int lval[])
{
        if (lval[2] == CINT)
                ol ("addd\t#2");
        else
                ol ("addd\t#1");

}

/*
 *      decrement the primary register by one if char, INTSIZE if
 *      int
 */
void gdec(int lval[])
{
        if (lval[2] == CINT)
                ol ("subd\t#2");
        else
                ol ("subd\t#1");

}

/*
 *      following are the conditional operators.
 *      they compare the secondary register against the primary register
 *      and put a literl 1 in the primary if the condition is true,
 *      otherwise they clear the primary register
 *
 */

/*
 *      equal
 *
 */
void geq(void)
{
        gcall ("^eq");
        stkp = stkp + INTSIZE;

}

/*
 *      not equal
 *
 */
void gne(void)
{
        gcall ("^ne");
        stkp = stkp + INTSIZE;

}

/*
 *      less than (signed)
 *
 */
void glt(void)
{
        gcall ("^lt");
        stkp = stkp + INTSIZE;
}

/*
 *      less than or equal (signed)
 *
 */
void gle(void)
{
        gcall ("^le");
        stkp = stkp + INTSIZE;

}

/*
 *      greater than (signed)
 *
 */
void ggt(void)
{
        gcall ("^gt");
        stkp = stkp + INTSIZE;

}

/*
 *      greater than or equal (signed)
 *
 */
void gge(void)
{
        gcall ("^ge");
        stkp = stkp + INTSIZE;

}

/*
 *      less than (unsigned)
 *
 */
void gult(void)
{
        gcall ("^ult");
        stkp = stkp + INTSIZE;

}

/*
 *      less than or equal (unsigned)
 *
 */
void gule(void)
{
        gcall ("^ule");
        stkp = stkp + INTSIZE;

}

/*
 *      greater than (unsigned)
 *
 */
void gugt(void)
{
        gcall ("^ugt");
        stkp = stkp + INTSIZE;

}

/*
 *      greater than or equal (unsigned)
 *
 */
void guge(void)
{
        gcall ("^uge");
        stkp = stkp + INTSIZE;

}

char *inclib(void) {
#ifdef  flex
        return("B.");
#endif
#ifdef  unix
        return(INCDIR);
#endif
#ifdef  cpm
        return("B:");
#endif

}

/*      Squirrel away argument count in a register that modstk/getloc/stloc
        doesn't touch.
*/

void gnargs(int d)
{
        ot ("ldu\t#");
        onum(d);
        newline();

}

#endif
