/*
 *	65C816 backend for the Fuzix C Compiler
 *
 *	Although the 65C816 has a sane usable stack of sorts it actually
 *	doesn't help us hugely because that stack has to be in bank 0 so
 *	it may not be 1:1 mapped with our data. Using 24 bit pointers on
 *	the CPU is just ghastly so instead we keep our own C stack.
 *
 *	For simplicity we keep the CPU in 16bit mode except for some
 *	8bit specific ops. This does slightly reduce code density so might
 *	want looking at eventually, however it's not clear there is much of
 *	a win.
 *
 *	A is the accumulator, X is used as a working pointer, Y is our
 *	software stack.
 *
 *	Most of the work is done by pri() which knows how to perform an 8 or
 *	16bit operation between the working accumulator and any of the variable
 *	locations. As the CPU is fairly regular for the bits we need this is
 *	actually pretty clean.
 *
 *	Things we do not do yet
 *	- full register tracking - still very hackish
 *	- need to write 8/16bit accumulator switcher
 *
 *	- A lot of operations work versus offset,x which means that we want
 *	  to rewrite
 *			deref
 *			  |
 *			plus|minus
 *		       /    \
 *		    thing   constoffset
 *
 *	into a derefplus operation that uses lda offset,x
 *
 *	Secondly we can do most operations on n,x which means that
 *	we can rewrite any operation of the form
 *
 *			op
 *		       /  \
 *                        deref/derefplus(left or right side)
 *			      \
 *			     expr
 *
 *	to load the expr into ,x (easy for simple ones, for complex we
 *	probably have to do it into a and tax) then do the op on offset,x
 *
 *	For code size we have a few problems. Our deref for locals is
 *	n,y which is a 3 byte instruction, but is offset by the fact we
 *	don't have to load first. Our push/pop of arguments is expensive though
 *	as we have to lda, dey dey sta 0,y which is 8 bytes for a value (Z80 is
 *	4 for example)
 *
 *	The other nasty is that because we use the CPU stack for some things
 *	we have to do some shuffling on helpers. Thankfully it's a lot easier
 *	with 16bit pointers.
 *
 *	TODO size:
 *	- turn some ops into helpers in small mode
 *	- pusharg helpers for common small values
 *	- if optsize && dataseg is 0 then set DP to point to the stack
 *	  frame and use DP for the local refs (needs a space for hireg in
 *	  each frame and to pass upper word back in X for this format)
 *	- Use stz.
 *	- fold a + b + 1 to use sec adc, ditto a - b - 1 and clc
 *
 *	Need a ccvalid flag of 0 = no 1 = yes 2 = backwards so we can shorten
 *	stuff like  lda 2,y and #1 jsr __bool beq by spotting ccvalid when
 *	we bool or not and setting it for the code paths that keep eq/ne right
 *	(of which there are many)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned arg_len;	/* Number of bytes of argument frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned xlabel;		/* Internal backend generated branches */
static unsigned argbase;	/* Track shift between arguments and stack */
static unsigned livesize = 2;	/* 16bit mode generating for */
static unsigned cursize = 2;	/* 16bit mode currently set */
static unsigned ccvalid;	/* CC state */

#define CC_NONE		0	/* CC meaningless */
#define CC_VALID	1	/* CC valid for 0 / not zero on A */
#define CC_INV		2	/* Ditto but inverted */

/*
 *	Node types we create in rewriting rules
 */
#define T_NREF		(T_USER)	/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)	/* Function call by name */
#define T_NSTORE	(T_USER+2)	/* Store to a C global/static */
#define T_LREF		(T_USER+3)	/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)	/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)	/* *regptr */
#define T_REQ		(T_USER+10)	/* *regptr */
#define T_DEREFPLUS	(T_USER+11)	/* *(thing + offset) */


/*
 *	65C816 specifics. We need to track some register values to produce
 *	bearable code
 */

static void output(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	ccvalid = CC_NONE;
	if (cursize != livesize) {
		if (livesize == 1)
			printf("\trep #0x20\n");
		else
			printf("\tsep #0x20\n");
		cursize = livesize;
	}
	putchar('\t');
	vprintf(p, v);
	putchar('\n');
	va_end(v);
}

static void outputcc(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	ccvalid = CC_VALID;
	if (cursize != livesize) {
		if (livesize == 1)
			printf("\tsep #0x20\n");
		else
			printf("\trep #0x20\n");
		cursize = livesize;
	}
	putchar('\t');
	vprintf(p, v);
	putchar('\n');
	va_end(v);
}

static void outputnc(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	ccvalid = CC_NONE;
	if (cursize != livesize) {
		if (livesize == 1)
			printf("\trep #0x20\n");
		else
			printf("\tsep #0x20\n");
		cursize = livesize;
	}
	putchar('\t');
	vprintf(p, v);
	putchar('\n');
	va_end(v);
}


static void label(const char *p, ...)
{
	va_list v;
	va_start(v, p);
	vprintf(p, v);
	putchar(':');
	putchar('\n');
	va_end(v);
	ccvalid = CC_NONE;
}

#define R_A	0
#define R_X	1
#define R_Y	2

#define INVALID	0

struct regtrack {
	unsigned state;
	uint16_t value;
	unsigned snum;
	unsigned offset;
};

struct regtrack reg[3];

static void invalidate_regs(void)
{
	reg[R_A].state = INVALID;
	reg[R_X].state = INVALID;
	reg[R_Y].state = INVALID;
	printf(";invalregs\n");
}


static void invalidate_a(void)
{
	reg[R_A].state = INVALID;
}

static void invalidate_x(void)
{
	reg[R_X].state = INVALID;
}

static void const_a_set(unsigned val)
{
	if (reg[R_A].state == T_CONSTANT)
		reg[R_A].value = val;
	else
		reg[R_A].state = INVALID;
}

static void const_x_set(unsigned val)
{
	if (reg[R_X].state == T_CONSTANT)
		reg[R_X].value = val;
	else
		reg[R_X].state = INVALID;
}

/* Get a value into A, adjust and track */
static void load_a(uint16_t n)
{
	if (livesize == 1)
		n &= 0xFF;
	if (reg[R_A].state == T_CONSTANT) {
		if (reg[R_A].value == n)
			return;
	}
	if (reg[R_X].state == T_CONSTANT && reg[R_X].value == n)
		outputcc("txa");
	else if (reg[R_Y].state == T_CONSTANT && reg[R_Y].value == n)
		outputcc("tya");
	else if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n - 1) {
		reg[R_A].value++;
		outputcc("inc a");
	} else if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n + 1) {
		reg[R_A].value--;
		outputcc("dec a");
	} else
		outputcc("lda #%u", n);
	reg[R_A].state = T_CONSTANT;
	reg[R_A].value = n;
	reg[R_A].snum = 0;
}

/* Get a value into X, adjust and track */
/* We are missing some obscure tricks - like using tax dex if A is 1 off what
   we need, but it's not likely these happen often */
static void load_x(uint16_t n)
{
	if (reg[R_X].state == T_CONSTANT) {
		if (reg[R_X].value == n)
			return;
		if (reg[R_X].value == n - 1) {
			output("inx");
			reg[R_X].value++;
			return;
		}
		if (reg[R_X].value == n + 1) {
			output("dex");
			reg[R_X].value--;
			return;
		}
		if (reg[R_X].value == n - 2) {
			output("inx");
			output("inx");
			reg[R_X].value += 2;
			return;
		}
		if (reg[R_X].value == n + 2) {
			output("dex");
			output("dex");
			reg[R_X].value -= 2;
			return;
		}
	}
	if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n)
		outputcc("tax");
	else
		output("ldx #%u", n);
	reg[R_X].state = T_CONSTANT;
	reg[R_X].value = n;
	reg[R_X].snum = 0;
}

/*
 *	For now just try and eliminate the reloads. We shuld be able to
 *	eliminate some surplus stores with thought if we are careful
 *	how we defer them.
 */
static void set_x_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	/* Turn store forms into ref forms */
	switch (op) {
	case T_NSTORE:
		op = T_NREF;
		break;
	case T_LBSTORE:
		op = T_LBREF;
		break;
	case T_LSTORE:
		op = T_LREF;
		break;
	case T_NAME:
	case T_CONSTANT:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
	case T_LOCAL:
	case T_ARGUMENT:
		break;
	default:
		invalidate_x();
		printf("; invalidate X\n");
		return;
	}
	;
	printf("; set X %x, %d\n", op, value);
	reg[R_X].state = op;
	reg[R_X].value = value;
	reg[R_A].snum = n->snum;
	reg[R_X].snum = n->snum;
	return;
}

/* Can't deal with NREF until we propagate volatile info better */
/* Also need to deal with loading X from A and A from or via  X for nodes and
   node contents */
static unsigned x_contains(struct node *n)
{
//      printf(";x contains? %x %ld\n", n->op, n->value);
	if (n->op == T_NREF)
		return 0;
	if (reg[R_A].state != n->op || reg[R_X].state != n->op)
		return 0;
	if (reg[R_A].value != n->value || reg[R_X].value != n->value)
		return 0;
	if (reg[R_A].snum != n->snum || reg[R_X].snum != n->snum)
		return 0;
	/* Looks good */
	return 1;
}

static void set_a_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	switch (op) {
	case T_NAME:
	case T_CONSTANT:
	case T_NREF:
	case T_NSTORE:
	case T_LBREF:
	case T_LBSTORE:
	case T_LREF:
	case T_LSTORE:
	case T_LOCAL:
	case T_ARGUMENT:
		reg[R_A].state = op;
		reg[R_A].value = value;
		reg[R_A].snum = n->snum;
		return;
	default:
		invalidate_a();
	}
}

static unsigned a_contains(struct node *n)
{
	if (reg[R_A].state != n->op)
		return 0;
	if (reg[R_A].value != n->value)
		return 0;
	if (reg[R_A].snum != n->snum)
		return 0;
	/* Looks good */
	return 1;
}

static void move_a_x(void)
{
	if (reg[R_X].state == INVALID || reg[R_A].state != reg[R_X].state || reg[R_A].value != reg[R_X].value || reg[R_A].snum != reg[R_X].snum) {
		outputcc("tax");
		memcpy(reg + R_X, reg + R_A, sizeof(struct regtrack));
	}
}

static void move_x_a(void)
{
	if (reg[R_A].state == INVALID || reg[R_A].state != reg[R_X].state || reg[R_A].value != reg[R_X].value || reg[R_A].snum != reg[R_X].snum) {
		outputcc("txa");
		memcpy(reg + R_A, reg + R_X, sizeof(struct regtrack));
	}
}


static void setsize(unsigned size)
{
	livesize = size;
	if (size == 1)
		const_a_set(reg[R_A].value & 0xFF);
}

static void set16bit(void)
{
	livesize = 2;
}


/* Memory writes occured, invalidate according to what we know. Passing
   NULL indicates unknown memory changes */

static void invalidate_node(struct node *n)
{
	/* For now don't deal with the complex cases of whether we might
	   invalidate another object */
	if (reg[R_A].state != T_CONSTANT)
		reg[R_A].state = INVALID;
	if (reg[R_X].state != T_CONSTANT)
		reg[R_X].state = INVALID;
}

static void invalidate_mem(void)
{
	/* Should be able to keep more - eg address of objects is ok */
	if (reg[R_A].state != T_CONSTANT)
		reg[R_A].state = INVALID;
	if (reg[R_X].state != T_CONSTANT)
		reg[R_X].state = INVALID;
}

/*
 *	Example size handling. In this case for a system that always
 *	pushes words.
 */
static unsigned get_size(unsigned t)
{
	if (PTR(t))
		return 2;
	if (t == CSHORT || t == USHORT)
		return 2;
	if (t == CCHAR || t == UCHAR)
		return 1;
	if (t == CLONG || t == ULONG || t == FLOAT)
		return 4;
	if (t == CLONGLONG || t == ULONGLONG || t == DOUBLE)
		return 8;
	if (t == VOID)
		return 0;
	error("gs");
	return 0;
}

/*
 *	For 65C816 we keep byte objects word size for argument stack.
 */
static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

static void repeated_op(unsigned n, const char *op)
{
	while (n--)
		output("%s", op);
}

static void repeated_op_cc(unsigned n, const char *op)
{
	while (n--)
		outputcc("%s", op);
}

static unsigned can_pri(struct node *n)
{
	unsigned op = n->op;
	if (op == T_LABEL || op == T_NAME || op == T_CONSTANT)
		return 1;
	if (op == T_LREF || op == T_NREF || op == T_LBREF || op == T_LSTORE || op == T_NSTORE || op == T_LBSTORE)
		return 1;
	return 0;
}

static void pre_none(struct node *n)
{
}

/* Construct a direct operation if possible for the primary op. Must not
   trash either A or X unless via_x is set then X may be trashed but the
   op must support addr,X addressing. */

/* We use the nc version when loading things other than the working
   register - eg X, or when storing as it doesn't set the flag. For the X
   case the caller needs to invalidate the cc either way */

static int do_pri(struct node *n, const char *op, void (*pre)(struct node *n), unsigned via_x)
{
	/* TODO: consider size 4  internally - but if so remember to also
	   fix the assumptions in argstack_helper that expect 2/1 always */
	struct node *r = n->right;
	const char *name;
	unsigned s;
	switch (n->op) {
	case T_LABEL:
		pre(n);
		outputnc("%s #T%d+%d", op, n->val2, (unsigned) n->value);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		outputnc("%s #_%s+%d", op, name, (unsigned) n->value);
		return 1;
	case T_CONSTANT:
		/* These had the right squashed into them */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		/* These had the right squashed into them */
		r = n;
		break;
	}
	/* TODO: optimize ld case for 8bit by loading 16 if not NAME */
	s = get_size(r->type);
	switch (r->op) {
	case T_CONSTANT:
		pre(n);
		outputnc("%s #%d", op, r->value & 0xFFFF);
		return 1;
	case T_LREF:
	case T_LSTORE:
		pre(n);
		if (optsize && 0) {
			/* TODO: use zp hacks stack tracking */
		}
		setsize(s);
		outputnc("%s %d,y", op, r->value + sp);
		set16bit();
		return 1;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		setsize(s);
		name = namestr(r->snum);
		outputnc("%s _%s+%d", op, name, (unsigned) r->value);
		set16bit();
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		setsize(s);
		outputnc("%s T%d+%d", op, r->val2, (unsigned) r->value);
		set16bit();
		return 1;
		/* If we add registers
		   case T_RREF:
		   outputnc("%s @__reg%d", op, r->val2);
		   return 1; */
		/*
		 *      We may be able to dereference stuff
		 */
	case T_DEREF:
	case T_DEREFPLUS:
		if (can_pri(r->right) && via_x) {
			/* No stx n,x so can't ldx 0,x to chain derefs.
			   Could possibly
			   pha
			   do_pri lda
			   tax
			   pla ?? */
			do_pri(r->right, "ldx", pre_none, 0);
			ccvalid = CC_NONE;
			invalidate_x();
			/* X now holds our pointer */
			setsize(s);
			pre(r);
			outputnc("%s %d,x", op, r->value);
			set16bit();
			invalidate_a();
			return 1;
		}
	}
	return 0;
}


static int do_pri_cc(struct node *n, const char *op, void (*pre)(struct node *n), unsigned via_x)
{
	/* TODO: consider size 4  internally - but if so remember to also
	   fix the assumptions in argstack_helper that expect 2/1 always */
	struct node *r = n->right;
	const char *name;
	unsigned s;
	switch (n->op) {
	case T_LABEL:
		pre(n);
		outputcc("%s #T%d+%d", op, n->val2, (unsigned) n->value);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		outputcc("%s #_%s+%d", op, name, (unsigned) n->value);
		return 1;
	case T_CONSTANT:
		/* These had the right squashed into them */
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		/* These had the right squashed into them */
		r = n;
		break;
	}
	/* TODO: optimize ld case for 8bit by loading 16 if not NAME */
	s = get_size(r->type);
	switch (r->op) {
	case T_CONSTANT:
		pre(n);
		outputcc("%s #%d", op, r->value & 0xFFFF);
		return 1;
	case T_LREF:
	case T_LSTORE:
		pre(n);
		if (optsize && 0) {
			/* TODO: use zp hacks stack tracking */
		}
		setsize(s);
		outputcc("%s %d,y", op, r->value + sp);
		set16bit();
		return 1;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		setsize(s);
		name = namestr(r->snum);
		outputcc("%s _%s+%d", op, name, (unsigned) r->value);
		set16bit();
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		setsize(s);
		outputcc("%s T%d+%d", op, r->val2, (unsigned) r->value);
		set16bit();
		return 1;
		/* If we add registers
		   case T_RREF:
		   output("%s @__reg%d", op, r->val2);
		   return 1; */
		/*
		 *      We may be able to dereference stuff
		 */
	case T_DEREF:
	case T_DEREFPLUS:
		if (can_pri(r->right) && via_x) {
			/* No ldx n,x so can't ldx 0,x to chain derefs.
			   Could possibly
			   pha
			   do_pri lda
			   tax
			   pla ?? */
			do_pri(r->right, "ldx", pre_none, 0);
			ccvalid = CC_NONE;
			invalidate_x();
			/* X now holds our pointer */
			setsize(s);
			pre(r);
			outputcc("%s %d,x", op, r->value);
			set16bit();
			invalidate_a();
			return 1;
		}
	}
	return 0;
}


static void pre_taxldaclc(struct node *n)
{
	if (!a_contains(n)) {
		move_a_x();
		outputcc("lda 0,x");
	}
	invalidate_a();
	outputnc("clc");
}

static void pre_taxldasec(struct node *n)
{
	if (!a_contains(n)) {
		move_a_x();
		outputcc("lda 0,x");
	}
	invalidate_a();
	outputnc("sec");
}

static void pre_taxlda(struct node *n)
{
	if (!a_contains(n)) {
		move_a_x();
		outputcc("lda 0,x");
	}
	invalidate_a();
}

static void pre_tax(struct node *n)
{
	move_a_x();
}

static int pri(struct node *n, const char *op)
{
	return do_pri(n, op, pre_none, 1);
}

static int pri_cc(struct node *n, const char *op)
{
	return do_pri_cc(n, op, pre_none, 1);
}

/* Load the right side into X directly, then use the helper */
static int pri_help(struct node *n, char *helper)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	/* We can't do indirections this way because there is no ldx n,x */
	if (do_pri(n, "ldx", pre_none, 0)) {
		ccvalid = CC_NONE;
		invalidate_mem();
		helper_s(n, helper);
		return 1;
	}
	return 0;
}

static int pri_help_bool(struct node *n, char *helper)
{
	int r = pri_help(n, helper);
	if (r)
		n->flags |= ISBOOL;
	return r;
}

static void pre_clc(struct node *n)
{
	outputnc("clc");
}

static void pre_sec(struct node *n)
{
	outputnc("sec");
}

/*
 *	inc and dec are complicated but worth some effort as they
 *	are so commonly used for small constants. We could do with
 *	spotting and folding some stuff like *x++ perhaps to get a
 *	bit better codegen. 
 */

/* We can do most stuff with inc/dec on the 816 */
static int leftop_memc(struct node *n, const char *op)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned v;
	unsigned sz = get_size(n->type);
	char *name;
	unsigned count;
	unsigned nr = n->flags & NORETURN;

	if (sz > 2 || optsize)
		return 0;
	if (r->op != T_CONSTANT || r->value > 2)
		return 0;
	else
		count = r->value;

	v = l->value;

	switch (l->op) {
	case T_NAME:
		name = namestr(l->snum);
		setsize(sz);
		while (count--)
			outputcc("%s _%s+%d", op, name, v);
		if (!nr) {
			outputcc("lda _%s+%d", name, v);
			set_a_node(l);
		}
		set16bit();
		return 1;
	case T_LABEL:
		setsize(sz);
		while (count--)
			outputcc("%s T%d+%d", op, (unsigned) l->val2, v);
		if (!nr) {
			outputcc("lda T%d+%d", (unsigned) l->val2, v);
			set_a_node(l);
		}
		set16bit();
		return 1;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		/* We can do ,x but not ,y */
		invalidate_x();
		output("tyx");
		setsize(sz);
		while (count--) {
			outputcc("%s %d,x", op, v);
			invalidate_a();
		}
		if (!nr) {
			outputcc("lda %d,x", v);
			set_a_node(l);
		}
		set16bit();
		return 1;

	}
	return 0;
}

/* Pull the left side into X and call the same helper we use for
   the direct forms where we loaded X directly */
static unsigned pop_help(struct node *n, const char *helper, unsigned size)
{
	if (size > 2)
		return 0;
	output("plx");
	invalidate_mem();
	helper_s(n, helper);
	return 1;
}

static unsigned pop_help_bool(struct node *n, const char *helper, unsigned size)
{
	unsigned r = pop_help(n, helper, size);
	if (r)
		n->flags |= ISBOOL;
	return r;
}

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
	n->val2 = o->val2;
	n->snum = o->snum;
	n->flags |= o->flags & (SIDEEFFECT | IMPURE);
	free_node(o);
}

static void squash_left(struct node *n, unsigned op)
{
	struct node *l = n->left;
	n->op = op;
	squash_node(n, l);
	n->left = NULL;
}

static void squash_right(struct node *n, unsigned op)
{
	struct node *r = n->right;
	n->op = op;
	squash_node(n, r);
	n->right = NULL;
}

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* FIXME: review pri and adjust. Want to proritize 'via X' ops when
	   we add prix(); */
	/* We can use these directly with primary operators on A */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME || (op == T_LREF && n->value < 255))
		return 10;
	if (op == T_NREF || op == T_LBREF || op == T_LREF)
		return 1;
	/* Hard */
	return 0;
}

static unsigned cconly_down(unsigned op)
{
	if (op == T_BOOL || op == T_BANG)
		return 1;
	if (op == T_LT || op == T_LTEQ || op == T_GT || op == T_GTEQ || op == T_EQEQ || op == T_BANGEQ)
		return 2;
	return 0;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 *
 *	We need to look at rewriting deref and assign with plus offset
 *	as if we've stuffed the ptr into tmp we can use ,y for deref
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;

	/* TODO
	   - rewrite some reg ops
	 */

	/* *regptr */
	if (op == T_DEREF && r->op == T_RREF) {
		n->op = T_RDEREF;
		n->right = NULL;
		return n;
	}
	/* *regptr = */
	if (op == T_EQ && l->op == T_RREF) {
		n->op = T_REQ;
		n->left = NULL;
		return n;
	}

#if 1
	/* Moved to previous pass FIXME */
	/* Deal with x + y + z where it's not all constant but we can
	   combine the constant part. This gets generated by struct
	   references and the like so needs to be handled */
	if (op == T_PLUS && l->op == T_PLUS && l->right->op == T_CONSTANT && r->op == T_CONSTANT) {
		r->value += l->right->value;
		n->left = l->left;
		free_node(l->right);
		free_node(l);
		return n;
	}
#endif
	/* Cast of a load of a 32bit value to 16. Very common and
	   easily fixed. If we were big endian we'd need to adjust
	   the offset but we are little so it works out nice and easy. Check
	   the actual type - don't do this to floats ! */
	if (op == T_CAST && get_size(n->type) == 2 && (r->type | UNSIGNED) == ULONG && (r->op == T_LREF || r->op == T_NREF || r->op == T_LBREF || r->op == T_DEREF || r->op == T_DEREFPLUS)) {
		r->type = n->type;
	}
	/* Try and propogate CCONLY down as best we can */
	if (cconly_down(op) == 1 && cconly_down(r->op))
		r->flags |= n->flags & CCONLY;

	/* Turn a deref of an offset to an object into a single op so we can
	   generate a single lda offset,x in the code generator. This happens
	   in some array dereferencing and especially struct pointer access */
	if (op == T_DEREF || op == T_DEREFPLUS) {
		if (r->op == T_PLUS && r->right->op == T_CONSTANT) {
			n->op = T_DEREFPLUS;
			free_node(r->right);
			n->right = r->left;
			n->value = r->right->value;
			free_node(r);
			return n;
		}
		if (op == T_DEREF)
			n->value = 0;	/* So we can treat deref/derefplus together */
	}

	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || nt == CLONG || nt == ULONG || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += argbase + frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_REG) {
				squash_right(n, T_RREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
			if (r->op == T_LABEL) {
				squash_right(n, T_LBREF);
				return n;
			}
		}
		if (op == T_EQ) {
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
			if (l->op == T_LABEL) {
				squash_left(n, T_LBSTORE);
				return n;
			}
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += argbase + frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
			if (l->op == T_REG) {
				squash_left(n, T_RSTORE);
				return n;
			}
		}
	}
	/* Eliminate casts for sign, pointer conversion or same */
	if (op == T_CAST) {
		if (nt == r->type || (nt ^ r->type) == UNSIGNED || (PTR(nt) && PTR(r->type))) {
			free_node(n);
			return r;
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (op == T_FUNCCALL && r->op == T_NAME && PTR(r->type) == 1) {
		n->op = T_CALLNAME;
		n->snum = r->snum;
		n->value = r->value;
		n->val2 = r->val2;
		n->flags |= (r->flags & (SIDEEFFECT | IMPURE));
		free_node(r);
		n->right = NULL;
	}
	/* Commutive operations. We can swap the sides over on these */
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR || op == T_PLUS) {
/*		printf(";left %d right %d\n", is_simple(n->left), is_simple(n->right)); */
		if (is_simple(n->left) > is_simple(n->right)) {
			n->right = l;
			n->left = r;
		}
	}
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	outputnc(".export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch (s) {
	case A_CODE:
		outputnc(codeseg);
		break;
	case A_DATA:
		outputnc(".data");
		break;
	case A_LITERAL:
		outputnc(".literal");
		break;
	case A_BSS:
		outputnc(".bss");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	unreachable = 0;
	printf("_%s:\n", name);
	invalidate_regs();
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned argsize)
{
	frame_len = size;
	arg_len = argsize;

	if (size == 0)
		return;

	sp += size;
	/* Maybe shortcut some common values ? */

	if (size) {
		if (size <= 6)
			repeated_op(size, "dey");
		else {
			output("tya");
			outputnc("clc");
			output("adc #%d", -size);
			output("tay");
		}
	}
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	/* TODO: also clean up args for non vararg case */
	unsigned cost = 8;
	if (sp != size) {
		error("sp");
	}
	sp -= size;

	/* Skip epilogue if it has no users */

	if (unreachable)
		return;
	/* Called function cleans up arguments on 65c816 - except vararg */
	if (!(func_flags & F_VARARG))
		size += argsize;
	if (func_flags & F_VOIDRET)
		cost -= 2;
	/* Use the helper for small cases */
	if (optsize && size > 3 && size < 12) {
		outputnc("jmp __fnexit%d", size);
		unreachable = 1;
		return;
	}
	/* Ugly as we need to preserve A */
	if (size) {
		if (size <= cost)
			repeated_op(size, "iny");
		else {
			if (!(func_flags & F_VOIDRET))
				outputnc("pha");
			output("tya");
			outputnc("clc");
			output("adc #%d", size);
			output("tay");
			if (!(func_flags & F_VOIDRET))
				outputcc("pla");
		}
	}
	outputnc("rts");
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	label("L%d%s", n, tail);
	invalidate_regs();
}

unsigned gen_exit(const char *tail, unsigned n)
{
	if (frame_len + arg_len == 0) {
		outputnc("rts");
		unreachable = 1;
		return 1;
	} else if (frame_len + arg_len <= 9) {
		outputnc("jmp __fnexit%d", frame_len + arg_len);
		unreachable = 1;
		return 1;
	} else {
		outputnc("jmp L%d%s", n, tail);
		return 0;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	/* The assembler auto converts these if needed */
	outputnc("bra L%d%s", n, tail);
	unreachable = 1;
}

const char *jflags = "neeq";

/* Set the condition code info ready for the branch that will follow */
static void setjflags(struct node *n, const char *us, const char *s)
{
	if (n->type & UNSIGNED)
		jflags = us;
	else
		jflags = s;
}

void gen_jfalse(const char *tail, unsigned n)
{
	outputnc("b%c%c L%d%s", jflags[2], jflags[3], n, tail);
	jflags = "neeq";
}

void gen_jtrue(const char *tail, unsigned n)
{
	outputnc("b%c%c L%d%s", jflags[0], jflags[1], n, tail);
	jflags = "neeq";
}

void gen_switch(unsigned n, unsigned type)
{
	invalidate_x();
	invalidate_mem();
	output("ldx #Sw%d", n);
	/* FIXME: to work with output */
	printf("\tjmp __switch");
	helper_type(type, 0);
	putchar('\n');
	unreachable = 1;
}

void gen_switchdata(unsigned n, unsigned size)
{
	label("Sw%d", n);
	outputnc("\t.word %d", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
	label("Sw%d_%d", tag, entry);
	invalidate_regs();
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	invalidate_regs();
	ccvalid = CC_NONE;
	printf("\tjsr __");
}

void gen_helpclean(struct node *n)
{
}

void gen_data_label(const char *name, unsigned align)
{
	label("_%s", name);
}

void gen_space(unsigned value)
{
	outputnc(".ds %d", value);
}

void gen_text_data(unsigned n)
{
	outputnc(".word T%d", n);
}

void gen_literal(unsigned n)
{
	if (n)
		label("T%d", n);
}

void gen_name(struct node *n)
{
	outputnc(".word _%s+%d", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		outputnc(".word %u", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		outputnc(".byte %u", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		outputnc(".word %d", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		outputnc(".word %d", (unsigned) (value & 0xFFFF));
		outputnc(".word %d", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	outputnc(".65c816");
	outputnc(".a16");
	outputnc(".i16");
	outputnc(".code");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	label(";");
}


/*
 *	For the 65C816 we can push to the system stack or working stack
 *	for temporaries. The system stack results in
 *	pha | plx stx @tmp | op @tmp, whilst using the C stack results in
 *	dey dey sta 0,y | op 0,y iny iny which is much longer and slower so
 *	we use the CPU stack. This also means our frame offsets are constant
 *	and probably makes future DP hacks easier.
 */
unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
// Doesn't affect data stack    sp += s;
	/* These don't invalidate registers and set Y to 0, so handle them
	   directly */
	switch (s) {
	case 1:
	case 2:
		outputnc("pha");
		return 1;
	case 4:
		invalidate_x();
		/* TODO:  hireg as a track target ? */
		output("ldx @hireg");
		outputnc("phx");
		outputnc("pha");
		return 1;
	}
	return 0;
}

/*
 *	Without a reg-reg add the simple algorithm we use on Z80/8080
 *	doesn't work but we can do some cases
 *
 *	We can asl a any power of 2 and if we much about with @tmp we
 *	can do any right side with two bits set. TODO
 */

unsigned gen_mul(unsigned v)
{
	if (v == 0) {
		load_a(0);
		return 1;
	}
	/* For now just do the usual pointer ones */
	if (v == 1)
		return 1;
	if (v == 2) {
		outputcc("asl a");
		return 1;
	}
	if (v == 4) {
		outputcc("asl a");
		outputcc("asl a");
		return 1;
	}
	if (v == 8) {
		outputcc("asl a");
		outputcc("asl a");
		outputcc("asl a");
		return 1;
	}
	if (v == 16 || v == 32 || v == 64 || v == 128) {
		output("jsr __mul%d", v);
		return 1;
	}
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;
	unsigned val;

	switch (n->op) {
		/* Clean up is special and must be handled directly. It also has the
		   type of the function return so don't use that for the cleanup value
		   in n->right */
	case T_CLEANUP:
		if (n->val2) {
			if (!(func_flags & F_VOIDRET))
				move_a_x();
			output("tya");
			outputnc("clc");
			output("adc #%d", r->value);
			output("tay");
			if (!(func_flags & F_VOIDRET))
				move_x_a();
			else
				invalidate_a();
		}
		sp -= n->right->value;
		return 1;
	case T_EQ:
		if (s <= 2 && nr && r->op == T_CONSTANT && r->value == 0) {
			setsize(s);
			invalidate_mem();
			outputnc("stz 0,x");
			set16bit();
			return 1;
		}
		/* address in A. See if right simple */
		if (s <= 2 && do_pri_cc(n, "lda", pre_tax, 0)) {
			invalidate_a();
			setsize(s);
			invalidate_mem();
			outputnc("sta 0,x");
			set16bit();
			return 1;
		}
		/* Need a pri32 for simple cases TODO */
		/* Complex on both sides. Do these the hard way. Not as bad
		   as it seems as these are not common */
		return 0;
	case T_AND:
		/* There are some cases we can deal with */
		if (s <= 2 && pri_cc(n, "and")) {
			invalidate_a();
			return 1;
		}
		if (s == 4 && !optsize) {
			if (r->op == T_CONSTANT) {
				if ((r->value & 0xFFFF) != 0xFFFF) {
					outputcc("and #%d", r->value & 0xFFFF);
					const_a_set(reg[R_A].value & r->value);
				}
				if ((r->value & 0xFFFF0000) != 0xFFFF0000) {
					move_a_x();
					output("lda @hireg");
					output("and #%d", r->value >> 16);
					output("sta @hireg");
					invalidate_a();
					move_x_a();
				}
				return 1;
			}
		}
		return pri_help(n, "andx");
	case T_OR:
		if (s <= 2 && pri_cc(n, "ora")) {
			invalidate_a();
			return 1;
		}
		if (s == 4 && !optsize) {
			if (r->op == T_CONSTANT) {
				if (r->value & 0xFFFF) {
					outputcc("ora #%d", r->value & 0xFFFF);
					const_a_set(reg[R_A].value | r->value);
				}
				if ((r->value & 0xFFFF0000)) {
					move_a_x();
					output("lda @hireg");
					output("ora #%d", r->value >> 16);
					output("sta @hireg");
					invalidate_a();
					move_x_a();
				}
				return 1;
			}
		}
		return pri_help(n, "orax");
	case T_HAT:
		if (s <= 2 && pri_cc(n, "eor")) {
			invalidate_a();
			return 1;
		}
		if (s == 4 && !optsize) {
			if (r->op == T_CONSTANT) {
				if (r->value & 0xFFFF) {
					outputcc("eor #%d", r->value & 0xFFFF);
					const_a_set(reg[R_A].value ^ r->value);
				}
				if ((r->value & 0xFFFF0000)) {
					move_a_x();
					output("lda @hireg");
					output("eor #%d", r->value >> 16);
					output("sta @hireg");
					invalidate_a();
					move_x_a();
				}
				return 1;
			}
		}
		return pri_help(n, "xorx");
	case T_PLUS:
		if (s <= 2 && r->op == T_CONSTANT) {
			if (r->value == 0)
				return 1;
			if (r->value == 1) {
				outputcc("inc a");
				const_a_set(reg[R_A].value + 1);
				return 1;
			}
			if (r->value == 2) {
				outputcc("inc a");
				outputcc("inc a");
				const_a_set(reg[R_A].value + 2);
				return 1;
			}
		}
		if (s <= 2 && do_pri_cc(n, "adc", pre_clc, 1)) {
			invalidate_a();
			return 1;
		}
		if (s == 4 && !optsize) {
			if (r->op == T_CONSTANT && r->value < 65536) {
				if (r->value == 0)
					return 1;
				if (r->value == 1)
					outputcc("inc a");
				else {
					outputnc("clc");
					output("adc #%d", r->value);
					output("bcc X%d", ++xlabel);
					output("inc @hireg");
					label("X%d", xlabel);
				}
				invalidate_a();
				return 1;
			}
			if (r->op == T_CONSTANT) {
				/* 32bit add */
				outputnc("clc");
				output("adc #%d", r->value & 0xFFFF);
				invalidate_a();
				move_a_x();
				output("lda @hireg");
				output("adc #%d", r->value >> 16);
				output("sta @hireg");
				invalidate_a();
				move_x_a();
				return 1;
			}
		}
		return pri_help(n, "adcx");
	case T_MINUS:
		if (s > 2)
			return 0;
		if (s <= 2 && r->op == T_CONSTANT) {
			if (r->value == 0)
				return 1;
			if (r->value == 1) {
				outputcc("dec a");
				const_a_set(reg[R_A].value - 1);
				return 1;
			}
			if (r->value == 2) {
				outputcc("dec a");
				outputcc("dec a");
				const_a_set(reg[R_A].value - 1);
				return 1;
			}
			if (r->value <= 0xFFFF) {
				outputnc("sec");
				outputcc("sbc #%d", (unsigned) (r->value & 0xFFFF));
				const_a_set(reg[R_A].value - (r->value & 0xFFFF));
				return 1;
			}
		}
		if (s <= 2 && do_pri_cc(n, "sbc", pre_sec, 1)) {
			invalidate_a();
			return 1;
		}
		if (s == 4 && !optsize) {
			if (r->op == T_CONSTANT && r->value < 65536) {
				if (r->value == 0)
					return 1;
				if (r->value == 1)
					outputcc("dec a");
				else {
					outputnc("sec");
					output("sbc #%d", r->value);
					output("bcs X%d", ++xlabel);
					output("dec @hireg");
					label("X%d", xlabel);
				}
				invalidate_a();
				return 1;
			}
			/* 32bit sub  */
			if (r->op == T_CONSTANT) {
				outputnc("sec");
				output("sbc #%d", r->value & 0xFFFF);
				invalidate_a();
				move_a_x();
				output("lda @hireg");
				output("sbc #%d", r->value >> 16);
				output("sta @hireg");
				invalidate_a();
				move_x_a();
				return 1;
			}
		}
		return pri_help(n, "sbcx");
	case T_STAR:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT && gen_mul(r->value))
			return 1;
		/* TODO: power of 2 into add/shifts, short form helpers
		   for low consts 2,4 etc */
		return pri_help(n, "mulx");
	case T_SLASH:
		/* TODO - power of 2 const into >> */
		return pri_help(n, "divx");
	case T_PERCENT:
		return pri_help(n, "remx");
		/*
		 *      There are various < 0, 0, !0, > 0 optimizations to do here
		 *      Also add bits to cc check not eq cases - set the compare rule
		 *
		 *      Rewrite constant compares by 1 if possible to get the easy
		 *      forms.
		 */
	case T_EQEQ:
		if (n->flags & CCONLY) {
			/* Condition codes are valid so no work neede */
			if (r->op == T_CONSTANT) {
				if (r->value == 0) {
					if (ccvalid)
						return 1;
					invalidate_x();
					/* Force a tax to set the flags as it's
					   cheaper than a compare */
					move_a_x();
					return 1;
				}
				if (r->value == 1) {
					outputcc("dec a");
					return 1;
				}
				if (r->value == 2) {
					outputcc("dec a");
					outputcc("dec a");
					return 1;
				}
				if (r->value == 65534) {
					outputcc("inc a");
					outputcc("inc a");
					return 1;
				}
				if (r->value == 65535) {
					outputcc("inc a");
					return 1;
				}
			}
			if (pri_cc(n, "cmp"))
				return 1;
		}
		if (r->op == T_CONSTANT && r->value == 0) {
			helper(n, "not");
			n->flags |= ISBOOL;
			return 1;
		}
		return pri_help_bool(n, "eqeqx");
		/* Might make sense to only generate two ops and invert the
		   j flags ? */
	case T_GT:
		if (n->flags & CCONLY) {
			if (r->op == T_CONSTANT) {
				/* > 0 is != 0 */
				if ((n->type & UNSIGNED) && r->value == 0) {
					/* Check == 0 */
					invalidate_x();
					move_a_x();
					n->flags |= ISBOOL;
					jflags = "neeq";
					return 1;
				}
				/* GT has harder than GTEQ so adjust */
				output("cmp #%d", r->value - 1);
				setjflags(n, "cccs", "vcvs");
				return 1;
			}
			/* Maybe deca and cmp ? */
		}
		return pri_help_bool(n, "gtx");
	case T_GTEQ:
		if (r->op == T_CONSTANT && r->value == 0 && !(n->type & UNSIGNED)) {
			if (n->flags & CCONLY) {
				outputcc("asl a");
				jflags = "cccs";
				invalidate_a();
				return 1;
			}
		}
		/* True of cs or vs, false if vc or sc */
		if (n->flags & CCONLY) {
			if (pri_cc(n, "cmp")) {
				n->flags |= ISBOOL;
				setjflags(n, "cscc", "vsvc");
				return 1;
			}
		}
		return pri_help_bool(n, "gteqx");
	case T_LTEQ:
		if (r->op != T_CONSTANT || !(n->flags & CCONLY))
			return pri_help_bool(n, "lteqx");
		/* Fall through into a modified lt */

	case T_LT:
		if (r->op == T_CONSTANT && r->value == 0 && !(n->type & UNSIGNED)) {
			if (n->flags & CCONLY) {
				outputcc("asl a");
				jflags = "cscc";
				return 1;
			} else {
				outputcc("asl a");
				outputcc("lda #0");
				outputcc("rol a");
				return 1;
			}
		}
		/* True if cc or vc */
		if (n->flags & CCONLY) {
			if (pri_cc(n, "cmp")) {
				n->flags |= ISBOOL;
				setjflags(n, "cccs", "vcvs");
				return 1;
			}
		}
		return pri_help_bool(n, "ltx");
	case T_BANGEQ:
		if (n->flags & CCONLY) {
			if (r->op == T_CONSTANT) {
				if (r->value == 0) {
					jflags = "neeq";
					if (!ccvalid) {
						invalidate_x();
						move_a_x();
					}
					return 1;
				}
				if (r->value == 1) {
					outputcc("dec a");
					jflags = "neeq";
					return 1;
				}
				if (r->value == 2) {
					outputcc("dec a");
					outputcc("dec a");
					jflags = "neeq";
					return 1;
				}
				if (r->value == 65534) {
					outputcc("inc a");
					outputcc("inc a");
					jflags = "neeq";
					return 1;
				}
				if (r->value == 65535) {
					outputcc("inc a");
					jflags = "neeq";
					return 1;
				}
			}
			if (pri_cc(n, "cmp")) {
				n->flags |= ISBOOL;
				jflags = "neeq";
				return 1;
			}
		}
		if (r->op == T_CONSTANT && r->value == 0) {
			helper(n, "bool");
			return 1;
		}
		return pri_help_bool(n, "nex");
	case T_LTLT:
		/* Value to shift is now in A */
		val = r->value & 15;
		if (s <= 2 && r->op == T_CONSTANT) {
			if (val >= 8) {
				outputcc("swa");
				outputcc("and #0xff00");
				val -= 8;
			}
			repeated_op_cc(val, "asl a");
			invalidate_a();
			return 1;
		}
		if (s <= 2 && !optsize && do_pri(n, "ldx", pre_none, 0)) {
			ccvalid = CC_NONE;
			setsize(s);
			output("bra X%d", xlabel + 2);
			label("X%d", ++xlabel);
			output("asl a");
			label("X%d", ++xlabel);
			output("dex");
			output("bne X%d", xlabel - 1);
			set16bit();
			return 1;
		}
		return pri_help(n, "lsx");
	case T_GTGT:
		val = r->value & 15;
		if (s <= 2 && r->op == T_CONSTANT) {
			if (n->type & UNSIGNED) {
				if (val >= 8) {
					outputcc("swa");
					outputcc("and #0xff");
					val -= 8;
				}
				repeated_op_cc(val, "lsr a");
				invalidate_a();
				return 1;
			}
			/* No quick asr */
		}
		return pri_help(n, "rsx");
		/* The left was complex (or we'd have used the shortcut path. The
		   right will be a constant. */
	case T_PLUSPLUS:
		if (s <= 2) {
			move_a_x();
			setsize(s);
			if (nr && r->value <= 4) {
				repeated_op(r->value, "inc 0,x");
				set16bit();
				return 1;
			}
			/* Right is always constant for plusplus/minusminus */
			if (!nr)
				outputnc("pha");
			if (r->value < 4)
				repeated_op_cc(r->value, "inc a");
			else {
				outputnc("clc");
				outputcc("adc #%d", r->value);
			}
			invalidate_mem();
			outputnc("sta 0,x");
			if (!nr)
				outputcc("pla");
			set16bit();
			invalidate_a();
			return 1;
		}
		return pri_help(n, "plusplusx");
	case T_MINUSMINUS:
		if (s <= 2) {
			move_a_x();
			setsize(s);
			if (nr && r->value <= 4) {
				repeated_op(r->value, "dec 0,x");
				set16bit();
				return 1;
			}
			/* Right is always constant for plusplus/minusminus */
			if (!nr)
				outputnc("pha");
			if (r->value <= 4)
				repeated_op_cc(r->value, "dec a");
			else {
				outputnc("sec");
				outputcc("sbc #%d", r->value);
			}
			invalidate_mem();
			outputnc("sta 0,x");
			if (!nr)
				outputcc("pla");
			set16bit();
			invalidate_a();
			return 1;
		}
		return pri_help(n, "minusminusx");
		/* The right on these may be complex, but the result is what
		   should remain in A which is easier */
	case T_PLUSEQ:
		if (s <= 2) {
			setsize(s);
			if (r->op == T_CONSTANT && r->value <= 4) {
				move_a_x();
				if (nr) {
					repeated_op(r->value, "inc 0,x");
					set16bit();
					return 1;
				}
				outputcc("lda 0,x");
				repeated_op_cc(r->value, "inc a");
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			} else if (do_pri_cc(n, "adc", pre_taxldaclc, 0)) {
				/* If we could use an operator we've generated
				   tax lda 0,x clc, operator */
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			}
			set16bit();
		}
		return pri_help(n, "pluseqx");
	case T_MINUSEQ:
		if (s <= 2) {
			setsize(s);
			if (r->op == T_CONSTANT && r->value <= 4) {
				move_a_x();
				if (nr) {
					repeated_op(r->value, "inc 0,x");
					set16bit();
					return 1;
				}
				invalidate_mem();
				outputcc("lda 0,x");
				repeated_op_cc(r->value, "inc a");
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			} else if (do_pri_cc(n, "sbc", pre_taxldasec, 0)) {
				/* If we could use an operator we've generated
				   tax lda 0,x clc, operator */
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			}
			set16bit();
		}
		return pri_help(n, "minuseqx");
	case T_STAREQ:
		/* There are some weird cases we can do clever stuff
		   but not many - eg * 2 result not needed is ALS addr TODO */
		return pri_help(n, "muleqx");
	case T_SLASHEQ:
		return pri_help(n, "diveqx");
	case T_PERCENTEQ:
		return pri_help(n, "diveqx");
	case T_ANDEQ:
		if (s <= 2) {
			setsize(s);
			if (do_pri_cc(n, "and", pre_taxlda, 0)) {
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			}
			set16bit();
		}
		return pri_help(n, "andeqx");
	case T_OREQ:
		if (s <= 2) {
			setsize(s);
			if (do_pri_cc(n, "ora", pre_taxlda, 0)) {
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			}
			set16bit();
		}
		return pri_help(n, "oreqx");
	case T_HATEQ:
		if (s <= 2) {
			setsize(s);
			if (do_pri_cc(n, "eor", pre_taxlda, 0)) {
				invalidate_mem();
				outputnc("sta 0,x");
				set16bit();
				set_a_node(n->left);
				return 1;
			}
			set16bit();
		}
		return pri_help(n, "xoreqx");
	case T_SHLEQ:
		if (s <= 2) {
			setsize(s);
			if (s == 2)
				val = r->value & 15;
			else
				val = r->value & 7;
			if (val >= 8) {
				outputcc("swa");
				outputcc("and #0xFF00");
				val -= 8;
			}
			repeated_op_cc(val, "asl a");
			invalidate_mem();
			outputnc("sta 0,x");
			set16bit();
			invalidate_a();
			return 1;
		}
		return pri_help(n, "shleqx");
	case T_SHREQ:
		if (s <= 2 && (n->type & UNSIGNED)) {
			setsize(s);
			if (s == 2)
				val = r->value & 15;
			else
				val = r->value & 7;
			if (val >= 8) {
				outputcc("swa");
				outputcc("and #0x00FF");
				val -= 8;
			}
			repeated_op_cc(val, "lsr a");
			invalidate_mem();
			outputnc("sta 0,x");
			invalidate_a();
			set16bit();
			return 1;
		}
		return pri_help(n, "shreqx");
	}
	return 0;
}

/*
 *	Allow the code generator to shortcut the generation of the argument
 *	of a single argument operator (for example to shortcut constant cases
 *	or simple name loads that can be done better directly)
 */
unsigned gen_uni_direct(struct node *n)
{
	return 0;
}

static unsigned argstack_helper(struct node *n, unsigned sz)
{
	if (n->op == T_CONSTANT) {
		if (sz <= 2) {
			if (n->value <= 4) {
				output("jsr __push%d", n->value);
				return 1;
			}
			return 0;
		}
		if (n->value == 0) {
			output("jsr __pushl0");
			return 1;
		}
		if (!(n->value & 0xFFFF0000UL)) {
			load_a(n->value);
			output("jsr __pushl0a");
			return 1;
		}
	}
	/* The common lda n,y push values */
	if (n->op == T_LREF && n->value + sp <= 10 && !((sp + n->value) & 1)) {
		output("jsr __pushy%d", n->value + sp);
		return 1;
	}
#if 0
	/* FIXME: floats.. */
	/* Shortcut 32bit loads to 16bit arg as they seem to occur
	   often enough to be interesting */
	if (n->op == T_CAST && sz == 2 && get_size(n->right->type) == 4) {
		struct node *r = n->right;
		if (do_pri_cc(r, "lda", pre_none, 1)) {
			output("jsr __pushl0a");
			return 1;
		}
	}
#endif
	return 0;
}

/* Given a node see if we can generate a short form for it */

static void argstack(struct node *n)
{
	unsigned sz = get_stack_size(n->type);
	if (optsize) {
		/* See if we can be clever */
		if (argstack_helper(n, sz)) {
			sp += sz;
			return;
		}
		codegen_lr(n);
		if (sz == 4)
			output("jsr __pushal");
		else
			output("jsr __pusha");
		sp += sz;
		return;
	}
	/* Generate the node */
	codegen_lr(n);
	/* And stack it */
	if (sz == 2) {
		output("dey");
		output("dey");
		outputnc("sta 0,y");
		sp += 2;
	} else if (sz == 4) {
		output("dey");
		output("dey");
		output("dey");
		output("dey");
		outputnc("sta 0,y");
		output("lda @hireg");
		output("sta 2,y");
		sp += 4;
	} else
		error("astk");
}

/* We handle function arguments specially as we both push them to a different
   stack and optimize them. The tree looks like this
   
                 FUNCCALL|CALLNAME
                  /
	       ARGCOMMA
	        /    \
	      ...    EXPR
	      /
	    ARG
 */
static void gen_fcall(struct node *n)
{
	if (n == NULL)
		return;
	if (n->op == T_ARGCOMMA) {
		/* Recurse down arguments and stack then on the way up */
		gen_fcall(n->left);
		argstack(n->right);
	} else {
		argstack(n);
	}
	/* Final node done */
}

/*
 *	Some internal helpers work best if we use the data stack for long
 *	values
 */
const char *longfn(struct node *n)
{
	switch (n->op) {
	case T_SLASH:
		return "div";
	case T_PERCENT:
		return "rem";
	case T_STAR:
		return "mul";
	case T_LT:
		return "cclt";
	case T_GT:
		return "ccgt";
	case T_LTEQ:
		return "cclteq";
	case T_GTEQ:
		return "ccgteq";
	case T_BANGEQ:
		return "ccne";
	case T_EQEQ:		/* Maybe - need to decide */
		return "cceq";
		/* Shifts etc TBD - might make more sense to generate l/r backwards
		   and take the shift value via x */
	}
	return NULL;
}

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;
	const char *p;
	unsigned size = get_size(n->type);

	/* Unreachable code we can shortcut into nothing whee.be.. */
	if (unreachable)
		return 1;
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		r->flags |= nr;
		codegen_lr(r);
		return 1;
	}
	/* We should never meet an ARGCOMMA as they are handled by
	   gen_fcall */
	if (n->op == T_ARGCOMMA) {
		fprintf(stderr, "argcomma?\n");
	}
	if (n->op == T_FUNCCALL) {
		/* Generate and stack all the arguments */
		gen_fcall(l);
		/* Generate the address of the function */
		codegen_lr(r);
		move_a_x();
		invalidate_mem();
		/* TODO: check if this jumps in current bank or if we need
		   to do inx/rts hacks etc FIXME */
		/* Run it */
		output("jsr (0,x)");
		invalidate_regs();
		return 1;
	}
	if (n->op == T_CALLNAME) {
		gen_fcall(l);
		invalidate_regs();
		invalidate_mem();
		output("jsr _%s+%d", namestr(n->snum), n->value);
		return 1;
	}
	/*
	 *      For some internal functions it is easier to handle them
	 *      as if we C like function calls with values on the data
	 *      stack not the CPU one. We could even do fp in C this way
	 *      if we needed to.
	 */
	if (size == 4 && (p = longfn(n)) != NULL) {
		codegen_lr(l);
		argstack(l);
		codegen_lr(r);
		helper_s(n, p);
		sp -= 4;	/* The helper cleans up */
		return 1;
	}
	/* Shifts we evaluate the count last so we can go via x */
	if (size == 4 && (n->op == T_LTLT || n->op == T_GTGT)) {
		codegen_lr(r);
		outputnc("pha");
		codegen_lr(l);
		output("plx");
		if (n->op == T_LTLT)
			helper(n, "shlx");
		else
			helper_s(n, "shrx");
		return 1;
	}
	/* Flip any addition we can merge over */
	if (size == 2 && n->op == T_PLUS) {
		printf(";addflip\n");
		if (can_pri(l)) {
			n->left = r;
			n->right = l;
		}
		return 0;
	}
	/* The left may be a complex expression but also may be something
	   we can directly reference. The right is the amount */
	if (n->op == T_PLUSPLUS && leftop_memc(n, "inc"))
		return 1;
	if (n->op == T_MINUSMINUS && leftop_memc(n, "dec"))
		return 1;
	if (n->op == T_PLUSEQ && leftop_memc(n, "inc"))
		return 1;
	if (n->op == T_MINUSEQ && leftop_memc(n, "dec"))
		return 1;
	return 0;
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;
	unsigned rs;

	/* Casting from rt to lt */
	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);
	rs = get_size(rt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* extending a signed object */
	if (!(rt & UNSIGNED)) {
		/* Signed char to int or uint */
		if (rs == 1 && ls == 2) {
			invalidate_x();
			/* Cheapest way to ensure N flag is set right */
			move_a_x();
			output("bpl X%d", ++xlabel);
			output("ora #0xFF00");
			label("X%d", xlabel);
			invalidate_a();
			return 1;
		}
		if (rs == 1 && ls == 4) {
			outputnc("stz @hireg");
			invalidate_x();
			/* Cheapest way to ensure N flag is set right */
			move_a_x();
			output("bpl X%d", ++xlabel);
			output("ora #0xFF00");
			output("dec @hireg");
			label("X%d", xlabel);
			invalidate_a();
			return 1;
		}
		/* 2 to 4 */
		if (rs == 2 && ls == 4) {
			outputnc("stz @hireg");
			invalidate_x();
			/* Cheapest way to ensure N flag is set right */
			move_a_x();
			output("bpl X%d", ++xlabel);
			output("dec @hireg");
			label("X%d", xlabel);
			invalidate_a();
			return 1;
		}
		/* Can't happen ? */
		return 0;
	}
	/* Casting unsigned */
	if (ls == 4) {
		rs = get_size(rt);
		if (rs == 1)
			outputcc("and #0xff");
		outputnc("stz @hireg");
		return 1;
	}
	if (ls == 2) {
		outputcc("and #0xff");
		return 1;
	}
	return 0;
}

static unsigned op_eq(struct node *n, const char *op, const char *pre, unsigned size)
{
	if (size <= 2) {
		outputnc("plx");
		setsize(size);
		if (pre)
			output("%s", pre);
		outputcc("%s 0,x", op);
		invalidate_mem();
		outputnc("sta 0,x");
		set16bit();
		set_a_node(n->left);
		return 1;
	}
	if (!optsize) {
		output("plx");
		if (pre)
			output("%s", pre);
		outputcc("%s 0,x", op);
		invalidate_mem();
		outputnc("sta 0,x");
		if (!(n->flags & NORETURN))
			output("pha");
		output("lda @hireg");
		output("%s 2,x", op);
		invalidate_mem();
		output("sta @hireg");
		output("sta 2,x");
		if (!(n->flags & NORETURN)) {
			outputcc("pla");
			set_a_node(n->left);
		} else
			invalidate_a();
		return 1;
	}
	return 0;
}

static unsigned op(struct node *n, const char *op, const char *pre, unsigned size)
{
	if (size <= 2) {
		output("sta @tmp");
		output("pla");
		if (pre)
			output("%s", pre);
		outputcc("%s @tmp", op);
		invalidate_a();
		return 1;
	}
	if (!optsize) {
		output("sta @tmp");
		output("pla");
		if (pre)
			output("%s", pre);
		output("%s @tmp", op);
		invalidate_a();
		move_a_x();
		output("pla");
		output("%s @hireg", op);
		output("sta @hireg");
		invalidate_a();
		move_x_a();
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned nr = n->flags & NORETURN;
	unsigned v = n->value;
	struct node *r = n->right;

#if 0
	/* Temporaries are on the CPU stack */
	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);
#endif
	switch (n->op) {
		/* FIXME: need to do 4 byte forms */
	case T_NREF:
		/* We need to do bytes properly in case they have side
		   effects on hardware */
		/* Need to avoid optimising existing loads until volatile is sorted */
		if (size <= 2) {
			if (pri_cc(n, "lda")) {
				set_a_node(n);
				return 1;
			}
		}
	case T_LBREF:
		if (size <= 2) {
			if (a_contains(n))
				return 1;
			if (x_contains(n)) {
				move_x_a();
				return 1;
			}
			if (pri_cc(n, "lda")) {
				invalidate_mem();
				set_a_node(n);
				return 1;
			}
		}
		if (size == 4) {
			if (n->op == T_LBREF) {
				output("lda T%d+%u", n->val2, (unsigned)n->value + 2);
				outputnc("sta @hireg");
				outputcc("lda T%d+%u", n->val2, (unsigned)n->value);
			} else {
				const char *name = namestr(n->snum);
				output("lda _%s+%d", name, (unsigned)n->value + 2);
				outputnc("sta @hireg");
				output("lda _%s+%d", name, (unsigned)n->value);
			}
			invalidate_a();
			return 1;
		}
		return 0;
	case T_LREF:
		v += sp;
		if (size <= 2) {
			if (a_contains(n))
				return 1;
			if (x_contains(n)) {
				move_a_x();
				return 1;
			}
			outputcc("lda %d,y", v);
			set_a_node(n);
			return 1;
		}
		if (size == 4) {
			output("lda %d,y", v + 2);
			output("sta @hireg");
			outputcc("lda %d,y", v);
			return 1;
		}
		return 0;
	case T_NSTORE:
	case T_LBSTORE:
		if (size <= 2 && r->op == T_CONSTANT && r->value == 0) {
			pri(n, "stz");
			invalidate_mem();
			set_a_node(n);
			return 1;
		}
		if (size <= 2 && pri(n, "sta")) {
			invalidate_mem();
			set_a_node(n);
			return 1;
		}
		if (size == 4) {
			if (!nr)
				outputnc("pha");
			if (n->op == T_LBSTORE) {
				outputnc("sta T%d+%u", n->val2, (unsigned)n->value);
				outputnc("lda @hireg");
				output("sta T%d+%u", n->val2, (unsigned)n->value + 2);
			} else {
				const char *name = namestr(n->snum);
				output("sta _%s+%d", name, (unsigned)n->value);
				outputnc("lda @hireg");
				output("sta _%s+%d", name, (unsigned)n->value + 2);
			}
			if (!nr)
				outputnc("pla");
			invalidate_a();
			return 1;
		}
		return 0;
	case T_LSTORE:
		/* Can't stz n,y so no stz check */
		if (size <= 2 && pri(n, "sta")) {
			invalidate_mem();
			set_a_node(n);
			return 1;
		}
		if (size == 4) {
			if (!nr)
				outputnc("pha");
			outputnc("sta %d,y", n->value + sp);
			output("lda @hireg");
			outputnc("sta %d,y", n->value + sp + 2);
			if (!nr)
				outputnc("pla");
			invalidate_a();
			return 1;
		}
		return 0;
	case T_CALLNAME:
		invalidate_regs();
		invalidate_mem();
		output("jsr _%s+%d", namestr(n->snum), v);
		return 1;
	case T_BOOL:
		/* A cast to nowhere is no cast at all */
		if (nr)
			return 1;
		/* Already boolified ? */
		if (r->flags & ISBOOL) {
			n->flags |= ISBOOL;
			return 1;
		}
		if (n->flags & CCONLY) {
			/* Already happens to be correct */
			if (ccvalid == CC_VALID)
				return 1;
			if (size == 1) {
				outputcc("and #0xff");
				return 1;
			} else if (size == 2) {
				/* Cheapest 'is it 0 set flags' is to
				   just tax and throw it away */
				invalidate_x();
				move_a_x();
				return 1;
			} else {
				/* 32bit - check both halves */
				outputcc("ora @hireg");
				return 1;
			}
		}
		/* Non condition code cases via helpers */
		return 0;
	case T_BANG:
		if (n->flags & CCONLY) {
			if (ccvalid == CC_VALID);
			else if (size == 1)
				outputcc("and #0xff");
			else if (size == 2) {
				/* Cheapest 'is it 0 set flags' is to
				   just tax and throw it away */
				invalidate_x();
				move_a_x();
			} else {
				outputcc("ora @hireg");
			}
			/* Basically eq with the flags reversed */
			setjflags(n, "eqne", "eqne");
			return 1;
		}
		/* Non condition code cases via helpers */
		return 0;
	case T_EQ:
		invalidate_mem();
		if (size > 2) {
			if (nr) {
				output("plx");
				outputnc("sta 0,x");
				output("lda @hireg");
				output("sta 2,x");
			} else {
				output("plx");
				output("sta 0,x");
				outputnc("pha");
				output("lda @hireg");
				output("sta 2,x");
				outputcc("pla");
			}
			invalidate_x();
			invalidate_a();
			return 1;
		}
		output("plx");
		setsize(size);
		outputnc("sta 0,x");
		set16bit();
		invalidate_a();
		invalidate_x();
		invalidate_mem();
		return 1;
	case T_FUNCCALL:
		move_a_x();
		invalidate_mem();
		output("jsr (0,x)");
		invalidate_regs();
		return 1;
	case T_DEREF:
	case T_DEREFPLUS:
		/* We could optimize the tracing a bit here. A deref
		   of memory where we know A is a name, local etc is
		   one where we can update the contents info TODO */
		move_a_x();
		if (size > 2) {
			output("lda %d,x", v + 2);
			output("sta @hireg");
			outputcc("lda %d,x", v);
			invalidate_a();
			return 1;
		}
		/* TODO: need to look at volatile propogation and volatile
		   plus hardware I/O for optimizing opportunities */
		setsize(size);
		outputcc("lda %d,x", v);
		set16bit();
		invalidate_a();
		return 1;
	case T_CONSTANT:
		if (size > 2) {
			if (v >> 16) {
				load_a(v >> 16);
				outputnc("sta @hireg");
			} else
				outputnc("stz @hireg");
		}
		load_a(v);
		return 1;
	case T_NAME:
		/* For correctness we need to byte op the loads, annoyingly
		   so need to look at optimizations for now hw addresses */
		if (size == 1 && pri_cc(n, "lda")) {
			set_a_node(n);
			return 1;
		}
		/* A label is an internal object so we don't care if we pull
		   an extra byte: TODO optimize this and local cases */
	case T_LABEL:
		if (pri_cc(n, "lda")) {
			if (size == 4)
				outputnc("stz @hireg");
			set_a_node(n);
			return 1;
		}
		return 0;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		output("tya");
		output("clc");
		outputcc("adc #%d", v);
		if (size == 4)
			outputnc("stz @hireg");
		return 1;
	case T_CAST:
		return gen_cast(n);
		/* Negate A */
	case T_TILDE:
		if (size <= 2) {
			outputcc("eor #0xFFFF; cpl");
			const_a_set(~reg[R_A].value);
			return 1;
		}
		if (size == 4 && !optsize) {
			outputcc("eor #0xFFFF");
			const_a_set(~reg[R_A].value);
			move_a_x();
			output("lda @hireg");
			output("eor #0xFFFF");
			output("sta @hireg");
			move_x_a();
			return 1;
		}
		return 0;
	case T_NEGATE:
		if (size <= 2) {
			outputcc("eor #0xFFFF");
			outputcc("inc a");
			const_a_set(-reg[R_A].value);
			return 1;
		}
		return 0;
	case T_AND:
		return op(n, "and", NULL, size);
	case T_OR:
		return op(n, "ora", NULL, size);
	case T_HAT:
		return op(n, "eor", NULL, size);
	case T_PLUS:
		return op(n, "adc", "clc", size);
	case T_MINUS:
		return op(n, "sbc", "sec", size);
	case T_STAR:
		return pop_help(n, "mulx", size);
	case T_SLASH:
		return pop_help(n, "divx", size);
	case T_PERCENT:
		return pop_help(n, "remx", size);
	case T_EQEQ:
		return pop_help_bool(n, "eqeqx", size);
	case T_GTEQ:
		return pop_help_bool(n, "gtx", size);
	case T_GT:
		return pop_help_bool(n, "gtx", size);
	case T_LTEQ:
		return pop_help_bool(n, "lteqx", size);
	case T_LT:
		return pop_help_bool(n, "ltx", size);
	case T_BANGEQ:
		return pop_help_bool(n, "nex", size);
		/*
		 *      Need to look at these. Left is 4 byte right is only
		 *      really 2 so we might be able to do a better job TODO
		 */
	case T_LTLT:
		return pop_help(n, "lsx", size);
	case T_GTGT:
		return pop_help(n, "rsx", size);
		/*
		 *      xxEQ ops have a pointer sized left so can go via X thus
		 *      we pass a size of 2 to pop_help
		 */
	case T_PLUSPLUS:
		if (nr == 0)
			return pop_help(n, "postincx", 2);
		/* Fall through */
	case T_PLUSEQ:
		if (op_eq(n, "adc", "clc", size))
			return 1;
		return pop_help(n, "pluseqx", 2);
	case T_MINUSMINUS:
		if (nr == 0)
			return pop_help(n, "postdecx", 2);
		/* Fall through */
	case T_MINUSEQ:
		/* This one is a bit different because order matters */
		if (size <= 2) {
			output("plx");
			setsize(size);
			outputnc("sta @tmp");
			outputcc("lda 0,x");
			outputnc("sec");
			outputcc("sbc @tmp");
			invalidate_mem();
			outputnc("sta 0,x");
			set16bit();
			set_a_node(n->left);
			invalidate_x();
			return 1;
		}
		if (!optsize) {
			output("plx");
			output("sta @tmp");
			outputcc("lda 0,x");
			outputnc("sec");
			outputcc("sbc @tmp");
			outputnc("sta 0,x");
			if (!nr)
				outputcc("pha");
			output("lda 2,x");
			output("sbc @hireg");
			output("sta @hireg");
			output("sta 2,x");
			if (!nr) {
				outputcc("pla");
				set_a_node(n->left);
			} else
				invalidate_a();
			return 1;
		}
		return pop_help(n, "minuseqx", 2);
	case T_STAREQ:
		return pop_help(n, "muleqx", 2);
	case T_SLASHEQ:
		return pop_help(n, "diveqx", 2);
	case T_PERCENTEQ:
		return pop_help(n, "remeqx", 2);
	case T_ANDEQ:
		return op_eq(n, "and", NULL, 2);
	case T_OREQ:
		return op_eq(n, "ora", NULL, 2);
	case T_HATEQ:
		return op_eq(n, "eor", NULL, 2);
	case T_SHLEQ:
		return pop_help(n, "shleqx", 2);
	case T_SHREQ:
		return pop_help(n, "shreqx", 2);
	}
	return 0;
}
