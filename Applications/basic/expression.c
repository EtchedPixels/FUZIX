/*
 *	The core expression parser.
 *
 *	Everything is a struct value. We provide implementation of the various
 *	included functions as well
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "basic.h"
#include "tokens_def.h"

static void do_expression(value *r , int level);

/*
 *	Trap out if the value is zero with a division by zero error
 */
static int divzero(int v)
{
    if (v == 0)
        error(ERROR_DIVZERO);
    return v;
}

/*
 *	Turn a value into an integer. Fail if not possible - BASIC does
 *	not do magic string to integer autoconversions
 */
static int intof(value *v)
{
    if (v->type != T_INT)
        error(ERROR_TYPE);
    return v->d.intval;
}

/*
 *	Return an integer as a result of a subexpression.
 */    
static int do_intexpression(int level)
{
    value v;
    do_expression(&v, level);
    return intof(&v);
}

/*
 *	Set the passed value object to the string resulting from
 *	evaluating this subexpression.
 */
static value *strexpression(value *v, int level)
{
    do_expression(v, level);
    if (v->type != T_STRING)
        error(ERROR_TYPE);
    return v;
}

/* Set a value to a boolean */
static void vsetbool(value *v, int n)
{
    v->type = T_INT;
    v->d.intval = !!n;
}

/* Set a value to an integer */
static void vsetint(value *v, int n)
{
    v->type = T_INT;
    v->d.intval = !!n;
}

/* Precedence for operators. Using the same table as the old Sinclar BASIC */
static uint8_t precedence(uint8_t c)
{
    if (c == TOK_OR)
        return 2;
    if (c == TOK_AND)
        return 3;
    if (c == TOK_NOT)
        return 4;
    if (c == '=' || c == '<' || c == '>' || c == TOK_NE || c == TOK_GE || c == TOK_LE)
        return 5;
    if (c == '+' || c == '-')
        return 6;
    if (c == '*' || c == '/')
        return 8;
    if (c == TOK_ORD)
        return 10;
    syntax();
}

/* Create a temporary string object. These are grabbed off the base
   of the heap and then recycled. This works because we never change
   the call/return stack during an expression evaluation */
static void tempstr(value *v, const uint8_t *d, int len)
{
    uint8_t *c = stralloc(len + 1);
    c[len] = 0;
    memcpy(c, d, len);
    v->d.string.data = c;
    v->d.string.len = len;
    v->d.string.temp = 1;
    v->type = T_STRING;
}

/* Chop up a string. When we are working with existing temporaries try to
   reuse them to reduce memory and speed up */
static void midstring(value *v, int s, int d)
{
    uint8_t *c;
    if (s < 1 || d < 0)
        error(ERROR_BADINDEX);
    /* Turn from 1 based to 0 based */
    s--;
    /* Work out what actual size we get */
    if (s > v->d.string.len) {
        s = v->d.string.len;
        d = 0;
    }
    else if (d > v->d.string.len - s)
        d = v->d.string.len - s;
    /* If this is a temporary then we can just mash it */
    if (v->d.string.temp) {
        v->d.string.data += s;
        v->d.string.len = d;
        v->d.string.data[d] = 0;
        return;
    }
    /* It's a real variable we must make a copy */
    tempstr(v, v->d.string.data + s, d);
}

static void rightstring(value *v, int n)
{
    uint8_t *c;
    if (n < 0)
        error(ERROR_BADINDEX);
    /* Already shorter */
    if (v->d.string.len <= n)
        return;
    /* If this is a temporary then we can just mash it */
    if (v->d.string.temp) {
        v->d.string.data += v->d.string.len - n;
        v->d.string.len = n;
        return;
    }
    /* It's a real variable we must make a copy */
    tempstr(v, v->d.string.data + v->d.string.len - n, n);
}

static void inkeystr(value *v)
{
    /* FIXME */
    tempstr(v, "", 0);
}

/* Create a new string from two others */
static void concat(value *v, value *p)
{
    /* FIXME: overflow case ? */
    if (p->d.string.len == 0)
        return;
    tempstr(v, v->d.string.data, v->d.string.len + p->d.string.len + 1);
    memcpy(v->d.string.data + v->d.string.len, p->d.string.data,
        p->d.string.len + 1);
}

/* Create a single byte string from a character */
static void char_func(value *v, int n)
{
    uint8_t c;
    if (n < -128 || n > 255)
        error(ERROR_RANGE);
    c = n;
    tempstr(v, &c, 1);
}

/* The BASIC 'SGN()' function */
static int sgn(int n)
{
    if (n == 0)
        return n;
    if (n < 0)
        return -1;
    return 1;
}

uint8_t *do_makenumber(value *r, uint8_t *p)
{
    /* Keep it simple for now */
    int n = 0;
    while(isdigit(*p)) {
        /* FIXME : overflow */
        n *= 10;
        n += *p++ - '0';
    }
    r->type = T_INT;
    r->d.intval = n;
    return p;
}


void makenumber(value *r)
{
    execp = do_makenumber(r, execp);
}

static int val(char *p)
{
    value v;
    if (*p == '-') {
        do_makenumber(&v, p);
        return -v.d.intval;
    } else if (isdigit(*p)) {
        do_makenumber(&v, p);
        return v.d.intval;
    }
    error(ERROR_TYPE);
}

static void powerof(value *v, int p)
{
    /* TODO */
}

static void makestring(value *v)
{
    /* Our end of line is cunning \0 so this works */
    uint8_t *e = strchr(++execp, '"');
    if (e == NULL)
        require('"');
    tempstr(v, execp, e - execp);
    execp = e + 1;
}
    
/* Evaluate an expression from left to right until we hit a lower priority
   operator */
 
static void do_expression(value *r, int level)
{
    value v;
    value tmpstr;

    /* We don't expect an end mid expression */    
    if (*execp == TOK_END)
        syntax();

    /* Begin by working out what we have on the left side. This might be
       - A value of some form
       - An operation taking a single argument (eg unary minus)
       - A bracketed expression
       - A function
    */
    /* Number */
    if (isdigit(*execp))
        makenumber(&v);
    /* String */
    else if (*execp == '"')
        makestring(&v);
    /* Variable */
    else if (isalpha(*execp))
        /* Identifier (functions are tokenized */
        get_variable(&v);
    /* NOT we handle as an oddity */
    else if (*execp == TOK_NOT)
        vsetbool(&v, !do_intexpression(LEVEL_4));
    /* Bracketed expressions get fully evaluated */
    else if (*execp == '(') {
        execp++;
        do_expression(&v, LEVEL_0);
        require(')');
    /* Function calls */
    } else if (*execp >= TOK_INT && *execp <= TOK_INKEY) {
        /* Function */
        uint8_t method = *execp++;
        /* FIXME: replace this with a uint8_t per var we scan giving
           2 bit codes for what is needed * ? */
        /* Catch any argumentless functions first */
        if (method == TOK_INKEY) {
            inkeystr(&v);
        } else {
            require('(');
            switch(method) {
            case TOK_INT:
                /* Meaningless whilst we are still integer basic */
                vsetint(&v, (int)intexpression());
                break;
            case TOK_ABS:
                vsetint(&v, abs(intexpression()));
                break;
            case TOK_SGN:
                vsetint(&v, sgn(intexpression()));
                break;
            case TOK_LEN:
                vsetint(&v, strexpression(&v, LEVEL_0)->d.string.len);
                break;
            case TOK_CODE:
                vsetint(&v, strexpression(&v, LEVEL_0)->d.string.data[0]);
                break;
            case TOK_VAL:
                vsetint(&v, val(strexpression(&v, LEVEL_0)->d.string.data));
                break;
            case TOK_MOD:
            {
                int left = intexpression();
                int right;
                require(',');
                right = intexpression();
                vsetint(&v, left % divzero(right));
                break;
            }
            case TOK_LEFT:
                strexpression(&v, LEVEL_0);
                require(',');
                midstring(&v, 0, intexpression());
                break;
            case TOK_RIGHT:
                strexpression(&v, LEVEL_0);
                require(',');
                rightstring(&v, intexpression());
                break;
            case TOK_MID:
            {
                int l1, l2;
                strexpression(&v, LEVEL_0);
                require(',');
                l1 = intexpression();
                require(',');
                l2 = intexpression();
                midstring(&v, l1, l2);
                break;
            }
            case TOK_CHR:
                char_func(&v, intexpression());
                break;
            }
            require(')');
        }
    } else if (*execp == '-') {
        /* Unary minus */
        execp++;
        /* Priority check.... FIXME */
        vsetint(&v, -intexpression());
    } else
        syntax();

    /* Now deal with binary and postfix operators. Keep evaluating
       until we find something of lower priority, then return so that
       the caller can evaluate their expression first */
    for(;;) {
        uint8_t c = *execp;
        int prec;

        /* We have completed evaluation */
        if (c == 0)
            break;

        /* If the precedence is lower then we should return the expression
           so far as this must be evaluted before our caller continues the
           work */
        prec = precedence(c);
        if (prec < level)
            break;

        /* Move past the operator */
        execp++;

        /* Logical operators */
        if (c == TOK_AND) {
            vsetbool(&v, intof(&v) && do_intexpression(LEVEL_3));
        } else if (c == TOK_OR) {
            vsetbool(&v, intof(&v) || do_intexpression(LEVEL_2));
        /* Power - in BASIC this is postfix */
        } else if (c == TOK_ORD) {
            powerof(&v, do_intexpression(LEVEL_10));
        /* Maths */
        } else if (c == '-') {
            vsetint(&v, intof(&v) - do_intexpression(LEVEL_6));
        } else if (c == '*') {
            vsetint(&v, intof(&v) * do_intexpression(LEVEL_8));
        } else if (c == '/') {
            vsetint(&v, intof(&v) / divzero(do_intexpression(LEVEL_8)));
        } else if (c == '+') {
            if (v.type == T_STRING) {
                strexpression(&tmpstr, LEVEL_6);
                concat(&v, &tmpstr);
            } else
                vsetint(&v, v.d.intval + do_intexpression(LEVEL_6));
        /* The comparison operators work on strings and numbers */
        } else if (v.type == T_INT) {
            int l = intof(&v);
            int r = do_intexpression(LEVEL_5);
            switch(c) {
            case TOK_LE:
                r = l <= r;
                break;
            case TOK_GE:
                r = l >= r;
                break;
            case TOK_NE:
                r = l != r;
                break;
            case '<':
                r = l < r;
                break;
            case '>':
                r = l > r;
                break;
            case '=':
                r = l == r;
                break;
            case '+':
                r = l + r;
            default:
                syntax();
                break;
            }
        } else {
            strexpression(&tmpstr, LEVEL_5);
            /* FIXME: strictly speaking this is wrong as you can have
               chr(0) in a BASIC string. We will swap strcmp for our own
               helper eventually */
            int r = strcmp(v.d.string.data, tmpstr.d.string.data);
            switch(c) {
            case TOK_LE:
                r = r != 1;
                break;
            case TOK_GE:
                r = r != -1;
                break;
            case TOK_NE:
                r = !!r;
                break;
            case '<':
                r = r == -1;
                break;
            case '>':
                r = r == 1;
                break;
            case '=':
                r = !r;
                break;
            default:
                syntax();
                break;
            }
        }
    }
    /* We are done */
    *r = v;
}

/*
 *	Calculate an expression and move the execution pointer to the point
 *	beyond.
 */
void expression(value *r)
{
    do_expression(r, LEVEL_0);
}

int intexpression(void)
{
    return do_intexpression(LEVEL_0);
}
