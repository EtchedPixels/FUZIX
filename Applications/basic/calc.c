#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "tokens_def.h"

#define DEBUG

static const char *ctypes[11] = {
    "eof",
    "constant",
    "leftassoc",
    "leftparen",
    "rightparen",
    "function",
    "unary",
    "symbol",
    "postfix",
    "function arg",
    "rightassoc"
};

enum {
    LOGIC_OR,
    LOGIC_AND,
    LOGIC_NOT,
    LOGIC_COMPARE,
    ARITH_AS,		/* add sub */
    ARITH_DM,		/* div mul */
    ARITH_UM,		/* umary minus */
    ARITH_O,		/* orders */
    ARITH_B,		/* brackets */
    ARITH_FUNC,		/* functions */
    ARITH_ARRAY,	/* array dereferencing */
    FN_COMMA,		/* , in functions */
    FN_CALL,		/* function call */
};

struct token {
    uint8_t tok;
#define EOF_TOKEN		128
#define INTEGER			129
#define SIZEOF			130
    uint8_t class;
#define CONSTANT		1
#define	LEFT_ASSOC		2
#define LEFTPAREN		3
#define RIGHTPAREN		4
#define FUNCTION		5
#define UNARY			6
#define SYMBOL			7
#define POSTFIX			8
#define FN_ARG			9
#define	RIGHT_ASSOC		10
    uint16_t data;
};

/* For now before we pack it all as an integer value */
#define CLASS(x)	((x)->class)
#define PRECEDENCE(x)	((x)->data)

#define OPMAX 32

char *input;

static const uint8_t ops[] = {
                        '!',		/* FIXME: for testing only */
                        '+', '-',
                        '*', '/',
                        '(',')',
                        TOK_ORD, '^',
                        TOK_NOT, TOK_AND, TOK_OR,
                        '=', '<', '>',
                        TOK_NE, TOK_LE, TOK_GE,
                        ',',
                        0
};

static uint8_t oppri[] = {
                            FN_CALL,
                            ARITH_AS, ARITH_AS,
                            ARITH_DM, ARITH_DM,
                            ARITH_B, 0,
                            ARITH_O, ARITH_O,
                            LOGIC_NOT, LOGIC_AND, LOGIC_OR,
                            LOGIC_COMPARE, LOGIC_COMPARE, LOGIC_COMPARE,
                            LOGIC_COMPARE, LOGIC_COMPARE, LOGIC_COMPARE,
                            FN_COMMA,
                          };
static uint8_t opclass[] = { FUNCTION,
                             LEFT_ASSOC, LEFT_ASSOC,	/* +- */
                             LEFT_ASSOC, LEFT_ASSOC, /* */
                             LEFTPAREN, RIGHTPAREN,
                             RIGHT_ASSOC, RIGHT_ASSOC,
                             UNARY, LEFT_ASSOC, LEFT_ASSOC,
                             LEFT_ASSOC, LEFT_ASSOC, LEFT_ASSOC,
                             LEFT_ASSOC, LEFT_ASSOC, LEFT_ASSOC,
                             FN_ARG,
                          };

static struct token eoftok = {
    0,
    EOF_TOKEN,
    0
};

static struct token starttok = {
    '(',
    LEFTPAREN,
    ARITH_B
};

static struct token peek;
static int peeked;

struct token *token(void)
{
    static struct token n;
    uint8_t *x;

    if (peeked) {
        peeked = 0;
        return &peek;
    }

    while(isspace(*input))
        input++;
    if (*input == 0)
        return &eoftok;

    n.tok = *input;

    if ((x = strchr(ops, *input)) != NULL) {
        uint8_t o = (uint8_t)(x - ops);
        n.class = opclass[o];
        n.data = oppri[o];
        input++;
        return &n;
    }
    /* symbol or numeric value */
    if (isdigit(*input)) {
        errno = 0;
        n.data = strtol(input, &input, 0);
        if (errno) {
            fprintf(stderr, "invalid integer.\n");
            exit(1);
        }
        n.class = CONSTANT;
        return &n;
    }
    if (isalpha(*input) || *input == '_') {
        char *p = input++;
        while(*input && (isalnum(*input) || *input == '_'))
            input++;
        printf("Will look for symbol\n");
        n.class = SYMBOL;
        n.data = 1;	/* Will be symbol index */
        return &n;
    }
    /* To add for testing function and comma */
    fprintf(stderr, "?? %s\n", input);
    exit(1);
}

const struct token *token_peek(void)
{
    if (peeked)
        return &peek;
    memcpy(&peek, token(), sizeof(peek));
    peeked = 1;
    return &peek;
}

struct token opstack[OPMAX];
struct token *optop = opstack;
uint16_t datastack[OPMAX];
uint16_t *datatop = datastack;

void push(uint16_t data)
{
    if (datatop == &datastack[OPMAX]) {
        fprintf(stderr, "push: too complex\n");
        exit(1);
    }
    *datatop++ = data;
}

uint16_t pop(void)
{
    if (datatop == datastack) {
        fprintf(stderr, "pop: underflow\n");
        exit(1);
    }
    return *--datatop;
}

void popop(void)
{
    struct token t;
    uint16_t tmp;
    if (optop == opstack) {
        fprintf(stderr, "popop: underflow\n");
        exit(1);
    }
    t = *--optop;
#ifdef DEBUG
    printf("popop %c\n", t.tok);
#endif    
    switch(t.tok) {
        case '(':
            break;
        case '+':
            push(pop() + pop());
            break;
        case '*':
            push(pop() * pop());
            break;
        case '/':
            tmp = pop();
//            if (tmp === 0) ...
            push(pop() / tmp);
            break;
        case '-':
            tmp = pop();
            if(t.class == LEFT_ASSOC)
                push(pop() - tmp);
            else
                push(-tmp);
            break;
        case '!':
        case TOK_NOT:
            push(!pop());
            break;
        case TOK_AND:
            push(pop() && pop());
            break;
        case TOK_OR:
            push(pop() || pop());
            break;
        case '=':
            push(pop() == pop());
            break;
        case '<':
            push(pop() >= pop());
            break;
        case '>':
            push(pop() <= pop());
            break;
        case TOK_NE:
            push(pop() != pop());
            break;
        case TOK_LE:
            push(pop() > pop());
            break;
        case TOK_GE:
            push(pop() < pop());
            break;
#ifdef FLOAT            
        case TOK_ORD:
            tmp = pop();
            push powf(pop(), tnp);
            break;
#endif
        case TOK_INT:
            push((int)pop());
            break;
        case TOK_ABS:
            push(abs(pop()));
            break;
        case TOK_SGN:
            tmp = pop();
            if (tmp < 0)
                push(-1);
            else if (tmp == 0)
                push(0);
            else
                push(1);
            break;
        case TOK_MOD:
            tmp = pop();
//            if (tmp == 0)...
            push(pop() % tmp);
            break;
        default:
            if (t.class == FUNCTION) {
                /* ??? */
            }
            fprintf(stderr,"popop: bad op '%c'\n", t.tok);
            exit(1);
    }
}

void pushop(const struct token *t)
{
    if (optop == &opstack[OPMAX]) {
        fprintf(stderr, "pushop: too complex\n");
        exit(1);
    }
#ifdef DEBUG    
    printf("pushop %d %c\n",t->class,t->tok);
#endif    
    *optop++ = *t;
}
      
void do_popout(uint8_t stopclass)
{
    while(optop != opstack) {
        if (optop[-1].class == stopclass)
            return;
        popop();
    }
}

void popout(void) {
    do_popout(LEFTPAREN);
    if (optop == opstack)
        fprintf(stderr, "Unbalanced brackets in expression.\n");
}

/* Do we need to be smarter here and spot ?'s left onthe opstack too ? */
void popout_final(void)
{
    do_popout(LEFTPAREN);
    popop();
    if (optop != opstack)
        fprintf(stderr, "Unbalanced brackets expression end.\n");
    printf("Answer = %u\n", pop());
    if (datatop != datastack)
        fprintf(stderr, "Unbalanced data end.\n");
}


static uint8_t next;
#define LPAREN		1		/* A left bracket - eg func( */
#define OPERAND		2		/* Any operand - symbol/constnat */
#define OPERATOR	4		/* An operator */
#define RPAREN		8		/* A right bracket - closing typedef */


/* We use bitmasks. The need mask must overlap the types passed. Usually
   it's a single bit but functions require a left-paren only, while a
   a left-paren is also valid for an operand */

void neednext(uint8_t h, uint8_t n)
{
    if (next & h) {
        next = n;
        return;
    }
    fprintf(stderr, "Syntax error: Want %x got %x.\n",
        (int)h, (int)next);
}

static const struct token fncall = {
    FUNCTION,
    FUNCTION,
    FN_CALL
};

/* Write out an expression tree as we linearly parse the code. We arrange
   things so we can write out all constants and in particular strings as
   we go so we don't buffer anything we don't need to in this pass
   
   TODO: ?: finish postfix operators (x++, x--, []) and typecasts (type)x
   Postfix is generalising the function call stuff. We may well want
   to defer knowing about function calls to tree parsing and just do
   generic markers and logic, however we do need to handle [expr] at
   this layer. sizeof may force our hands a bit
   
   Note that at this level we don't care about types. We will do type
   checking and type conversion insertions when we parse the resulting
   trees
   
   Would it be better to recurse , and do ?: at a higher level ?
   
   */

void eval(void)
{
    struct token *t;
    next = OPERAND;

    while((t = token())->class != EOF_TOKEN) {
#ifdef DEBUG
        printf("|Token %d Class %s Data %d\n",
            t->tok, ctypes[t->class], t->data);
#endif            
        switch(CLASS(t)) {
            case CONSTANT:
                neednext(OPERAND, OPERATOR);
                push(t->data);
                break;
            case SYMBOL:
                neednext(OPERAND, OPERATOR|LPAREN);
                push(t->data);
                break;
            case UNARY:
                neednext(OPERAND|LPAREN, OPERAND);
                pushop(t);
                break;
            case LEFT_ASSOC:
                /* Umary minus is special */
                if (t->tok == '-' && next == OPERAND) {
                    neednext(OPERAND|LPAREN, OPERAND);
                    t->class = UNARY;
                    pushop(t);
                    break;
                }
            case FUNCTION:
            case RIGHT_ASSOC:
                if (CLASS(t) == FUNCTION)
                    neednext(OPERAND, OPERAND);
                else
                    neednext(OPERATOR, OPERAND);
                do {
                    if (optop == opstack || CLASS(optop-1) == LEFTPAREN) {
                        pushop(t);
                        break;
                    }
                    if (PRECEDENCE(t) > PRECEDENCE(optop-1)) {
                        pushop(t);
                        break;
                    }
                    if (CLASS(t) == RIGHT_ASSOC && PRECEDENCE(t) == PRECEDENCE(optop-1)) {
                        pushop(t);
                        break;
                    }
                    popop();
                } while (optop > opstack);
                if (optop == opstack)
                    fprintf(stderr, "internal -> bracket lost\n");
                break;
                
            case LEFTPAREN:
                if (next & OPERATOR) {
                    /* Function call */
                    neednext(LPAREN,OPERAND);
                    pushop(&fncall);
                } else
                    /* Else precedence bracketing */
                    neednext(OPERAND|LPAREN,OPERAND);
                pushop(t);
                break;
            case FN_ARG:
                /* Calculate the argument before the comma if any and put
                   it on the data stack */
                do_popout(FUNCTION);
                neednext(OPERATOR,OPERAND|LPAREN);
                break;
            case RIGHTPAREN:
                neednext(OPERATOR|RPAREN, OPERATOR);
                popout();
                popop();	/* drop the left paren */
                break;
            default:		/* Assume we've hit the expression end */
                goto done;
        }
    }
done:
    neednext(OPERATOR,0);
    popout_final();
}

int main(int argc, char *argv[])
{
    char buf[512];
    fgets(buf, 512, stdin);
    input = buf;
    pushop(&starttok);
    eval();
}
