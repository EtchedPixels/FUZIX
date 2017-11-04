#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>


static const char *ctypes[9] = {
    "eof",
    "constant",
    "leftassoc",
    "leftparen",
    "rightparen",
    "function",
    "unary",
    "symbol",
    "postfix",
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
    uint16_t data;
};

/* For now before we pack it all as an integer value */
#define CLASS(x)	((x)->class)
#define PRECEDENCE(x)	((x)->data)

#define OPMAX 32

char *input;

static const char ops[] = {"+-*/%"};

static struct token eoftok = {
    0,
    EOF_TOKEN,
    0
};

static struct token eoftok2 = {
    ';',
    EOF_TOKEN,
    0
};

static struct token peek;
static int peeked;

struct token *token(void)
{
    static struct token n;
    char *x;

    if (peeked) {
        peeked = 0;
        return &peek;
    }

    while(isspace(*input))
        input++;
    if (*input == 0)
        return &eoftok;

    if (*input == ';')
        return &eoftok2;

    if ((x = strchr(ops, *input)) != NULL) {
        n.tok = *input;		/* Op code */
        n.class = LEFT_ASSOC;
        n.data = 1 + x - ops;	/* Priority */
        input++;
        return &n;
    }
    if (*input == '(') {
        n.tok = *input;
        n.class = LEFTPAREN;
        input++;
        return &n;
    }
    if (*input == ')') {
        n.tok = *input;
        n.class = RIGHTPAREN;
        input++;
        return &n;
    }
    if (*input == '!') {
        n.tok = '!';
        n.class = UNARY;
        n.data = 0;
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
            push(pop() / tmp);
            break;
        case '%':
            tmp = pop();
            push(pop() % tmp);
            break;
        case '-':
            tmp = pop();
            if(t.class == LEFT_ASSOC)
                push(pop() - tmp);
            else
                push(-tmp);
            break;
        case '!':
            push(!pop());
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
    0
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

const struct token *eval(int in_decl)
{
    struct token *t;
    next = OPERAND;

    while((t = token())->class != EOF_TOKEN) {
#if DEBUG
        printf("|Token %d Class %s Data %d\n",
            t->tok, ctypes[t->class], t->data);
#endif            
        switch(CLASS(t)) {
            case CONSTANT:
                neednext(OPERAND, OPERATOR);
                push(t->data);
                break;
            case SYMBOL:
                /* symbols might be functions - tidy this up */
                neednext(OPERAND, OPERATOR|LPAREN);
                push(t->data);
                break;
            case UNARY:
                neednext(OPERAND, OPERAND);
                pushop(t);
                break;
            case LEFT_ASSOC:
                /* Umary minus is special */
                if (t->tok == '-' && next == OPERAND) {
                    neednext(OPERAND, OPERAND);
                    t->class = UNARY;
                    pushop(t);
                    break;
                }
            case FUNCTION:
                neednext(OPERATOR, OPERAND);
                while (optop > opstack && optop->class == LEFT_ASSOC &&
                       PRECEDENCE(t) <= PRECEDENCE(optop))
                    popop();
                pushop(t);
                if (optop > opstack && optop->class == FUNCTION)
                    popop();
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
    return t;
}

int main(int argc, char *argv[])
{
    char buf[512];
    fgets(buf, 512, stdin);
    input = buf;
    eval(0);
}
