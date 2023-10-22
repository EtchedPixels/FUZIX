/*
 *	6502 backend for the Fuzix C Compiler
 *
 *	The big challenge here is that the C stack is a software
 *	construct and so quite slow to adjust. As the compiler thinks
 *	mostly in terms of call frames we can avoid a chunk of the cost
 *	but not all of it.
 *
 *	We try and reduce the cost by
 *	1. Generating direct references whenever we can
 *	2. When we need a helper and we can directly access we stuff the
 *	   one side of the operation into @tmp
 *	3. For certain operations we generate the left/right ourselves and
 *	   go via the CPU stack. This is a win in some common cases like
 *	   assignment, particularly on the 65C02
 *
 *	For the rest we have to go via the C stack which whilst painfulf in
 *	places is helped by the relatively low clocks per instruction.
 *
 *	Elements of this design like the separate stack with ZP pointer are
 *	heavily influenced by CC65 and one goal is to use many of the same
 *	support functions. Our pproach to code generation is however quite
 *	different and constrained by wanting to run on an 8bit micro.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "compiler.h"
#include "backend.h"

#define NMOS_6502	0
#define CMOS_6502	1

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned xlabel;		/* Internal backend generated branches */
static unsigned argbase;	/* Track shift between arguments and stack */

/*
 *	Node types we create in rewriting rules
 */
#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)		/* *regptr */
#define T_REQ		(T_USER+10)		/* *regptr */


/*
 *	6502 specifics. We need to track some register values to produce
 *	bearable code
 */

static void output(const char *p, ...)
{
	va_list v;
	va_start(v, p);
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
}


#define R_A	0
#define R_X	1
#define R_Y	2

#define INVALID	0

struct regtrack {
	unsigned state;
	uint8_t value;
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

static void invalidate_y(void)
{
	reg[R_Y].state = INVALID;
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

static void const_y_set(unsigned val)
{
	if (reg[R_Y].state == T_CONSTANT)
		reg[R_Y].value = val;
	else
		reg[R_Y].state = INVALID;
}

/* Get a value into A, adjust and track */
static void load_a(uint8_t n)
{
	if (reg[R_A].state == T_CONSTANT) {
		if (reg[R_A].value == n)
			return;
	}
	/* No inca deca */
	if (reg[R_X].state == T_CONSTANT && reg[R_X].value == n)
		output("txa");
	else if (reg[R_Y].state == T_CONSTANT && reg[R_Y].value == n)
		output("tya");
	else
	output("lda #%u", n);
	reg[R_A].state = T_CONSTANT;
	reg[R_A].value = n;
}

/* Get a value into X, adjust and track */
static void load_x(uint8_t n)
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
	}
	/* TODO: tyx for later CPU ? */
	if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n)
		output("tax");
	else
		output("ldx #%u", n);
	reg[R_X].state = T_CONSTANT;
	reg[R_X].value = n;
}

/* Get a value into Y, adjust and track */
static void load_y(uint8_t n)
{
	if (reg[R_Y].state == T_CONSTANT) {
		if (reg[R_Y].value == n)
			return;
		if (reg[R_Y].value == n - 1) {
			output("iny");
			reg[R_Y].value++;
			return;
		}
		if (reg[R_Y].value == n + 1) {
			output("dey");
			reg[R_Y].value--;
			return;
		}
	}
	/* TODO: tyx for later CPU ? */
	if (reg[R_A].state == T_CONSTANT && reg[R_A].value == n)
		output("tay");
	else
		output("ldy #%u", n);
	reg[R_Y].state = T_CONSTANT;
	reg[R_Y].value = n;
}

/*
 *	For now just try and eliminate the reloads. We shuld be able to
 *	eliminate some surplus stores with thought if we are careful
 *	how we defer them.
 */
static void set_xa_node(struct node *n)
{
	unsigned op = n->op;
	unsigned value = n->value;

	/* Turn store forms into ref forms */
	switch(op) {
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
		invalidate_a();
		invalidate_x();
		printf("; invalidate xa\n");
		return;
	}
;	printf("; set XA %x, %d\n", op, value);
	reg[R_X].state = op;
	reg[R_A].state = op;
	reg[R_A].value = value;
	reg[R_X].value = value;
	reg[R_A].snum = n->snum;
	reg[R_X].snum = n->snum;
	return;
}

/* Can't deal with NREF until we propagate volatile info better */

static unsigned xa_contains(struct node *n)
{
//	printf(";xa contains? %x %ld\n", n->op, n->value);
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

	switch(op) {
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
	if (reg[R_A].state != T_CONSTANT)
		reg[R_A].state = INVALID;
	if (reg[R_X].state != T_CONSTANT)
		reg[R_X].state = INVALID;
}

static void set_reg(unsigned r, unsigned v)
{
	reg[r].state = T_CONSTANT;
	reg[r].value = (uint8_t)v;
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
 *	For 6502 we keep byte objects byte size
 */
static unsigned get_stack_size(unsigned t)
{
	return get_size(t);
}


/* Generate a call to an internal helper */
static void gen_internal(const char *p)
{
	invalidate_regs();
	output("jsr __%s", p);
}


/* Construct a direct operation if possible for the primary op */
static int do_pri8(struct node *n, const char *op, void (*pre)(struct node * n))
{
	struct node *r = n->right;
	const char *name;
	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%s #<T%d+%d", op,  n->val2, (unsigned)n->value);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%s #<_%s+%d", op,  name, (unsigned)n->value);
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
	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		output("%s #%d", op, r->value & 0xFF);
		return 1;
	case T_LREF:
	case T_LSTORE:
		/* 255 is a fringe case we can do for 8bit but not split
		   8 and 16, so for now just skip it */
		if (r->value < 254) {
			pre(n);
			load_y(r->value);
			output("%s (@sp),y", op);
			return 1;
		}
		/* For now punt */
		return 0;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		name = namestr(r->snum);
		output("%s _%s+%d", op,  name, (unsigned)r->value);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		output("%s T%d+%d", op,  r->val2, (unsigned)r->value);
		return 1;
	/* If we add registers
	case T_RREF:
		output("%s __reg%d", op, r->val2);
		return 1;*/ 
	}
	return 0;
}

/* Construct a direct operation if possible for the primary op */
static int do_pri8hi(struct node *n, const char *op, void (*pre)(struct node * n))
{
	struct node *r = n->right;
	const char *name;
	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%s #>T%d+%d", op,  n->val2, (unsigned)n->value);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%s #_%s+%d", op,  name, (unsigned)n->value);
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
	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		output("%s #%d", op, (unsigned)(r->value >> 8));
		return 1;
	case T_LREF:
	case T_LSTORE:
		if (r->value < 254) {
			pre(n);
			load_y(r->value + 1) ;
			output("%s (@sp),y", op);
			return 1;
		}
		/* For now punt */
		return 0;
	case T_NREF:
	case T_NSTORE:
		pre(n);
		name = namestr(r->snum);
		output("%s _%s+%d", op,  name, (unsigned)r->value + 1);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		output("%s T%d+%d", op,  r->val2, (unsigned)r->value + 1);
		return 1;
	/* If we add registers
	case T_RREF:
		output("%s __reg%d+1", op, r->val2);
		return 1;*/ 
	}
	return 0;
}

/* 16bit/ We are rather limited here because we only have a few ops with x */
static int do_pri16(struct node *n, const char *op, void (*pre)(struct node *n))
{
	struct node *r = n->right;
	const char *name;
	switch(n->op) {
	case T_LABEL:
		pre(n);
		output("%sa #<T%d+%d", op,  n->val2, (unsigned)n->value);
		output("%sx #>T%d+%d", op,  n->val2, ((unsigned)n->value) >> 8);
		return 1;
	case T_NAME:
		pre(n);
		name = namestr(n->snum);
		output("%sa #<_%s+%d", op,  n, (unsigned)n->value);
		output("%sx #>_%s+%d", op,  n, ((unsigned)n->value) >> 8);
		return 1;
	case T_LREF:
	case T_NREF:
	case T_LBREF:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
	case T_CONSTANT:
		/* These had the right squashed into them */
		r = n;
	}
	switch(r->op) {
	case T_CONSTANT:
		pre(n);
		output("%sa #%d", op, r->value & 0xFF);
		output("%sx #%d", op, r->value >> 8);
		return 1;
	case T_LSTORE:
	case T_LREF:
		if (r->value < 255) {
			pre(n);
			load_y(r->value + 1);
			output("%sa (@sp),y", op);
			output("tax");
			load_y(r->value);
			output("%sa (@sp),y", op);
			return 1;
		}
		/* For now punt */
		return 0;
	case T_NSTORE:
	case T_NREF:
		name = namestr(r->snum);
		pre(n);
		output("%sa _%s+%d", op,  name, (unsigned)r->value);
		output("%sx _%s+%d", op,  name, ((unsigned)r->value) + 1);
		return 1;
	case T_LBSTORE:
	case T_LBREF:
		pre(n);
		output("%sa T%d+%d", op,  r->val2, (unsigned)r->value);
		output("%sx T%d+%d", op,  r->val2, ((unsigned)r->value) + 1);
		return 1;
	/* If we add registers
	case T_RREF:
		pre(n);
		output("%sa __reg%dd", op, r->val2);
		output("%sx __reg%d + 1", op,  r->val2);
		return 1;*/ 
	}
	return 0;
}

static void pre_none(struct node *n)
{
}

static void pre_store8(struct node *n)
{
	output("sta @tmp");
}

static void pre_store16(struct node *n)
{
	output("sta @tmp");
	output("stx @tmp+1");
}

static int pri8(struct node *n, const char *op)
{
	return do_pri8(n, op, pre_none);
}

static int pri16(struct node *n, const char *op)
{
	return do_pri16(n, op, pre_none);
}

static int pri8_help(struct node *n, char *helper)
{
	if (do_pri8(n, "lda", pre_store8)) {
		/* Helper invalidates A itself */
		helper_s(n, helper);
		set_reg(R_Y, 0);
		return 1;
	}
	return 0;
}

static int pri16_help(struct node *n, char *helper)
{
	if (do_pri16(n, "ld", pre_store16)) {
		/* Helper invalidates XA itself */
		helper_s(n, helper);
		set_reg(R_Y, 1);
		return 1;
	}
	return 0;
}

static int pri_help(struct node *n, char *helper)
{
	unsigned s = get_size(n->type);

	if (s == 1 && pri8_help(n, helper))
		return 1;
	else if (s == 2 && pri16_help(n, helper))
		return 1;
	return 0;
}

static void pre_clc(struct node *n)
{
	output("clc");
}

static void pre_sec(struct node *n)
{
	output("sec");
}

static void pre_stash(struct node *n)
{
	output("sta @tmp");
	output("stx @tmp+1");
}

/*
 *	inc and dec are complicated but worth some effort as they
 *	are so commonly used for small constants. We could o with
 *	spotting and folding some stuff like *x++ perhaps to get a
 *	bit better codegen. 
 */

/* Try to write inline inc and dec for simple forms */
/* TODO: sort out flag tests for dec */
static int leftop_memc(struct node *n, const char *op)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned v;
	unsigned sz = get_size(n->type);
	char *name;
	unsigned count;
	unsigned nr = n->flags & NORETURN;

	if (sz > 2)
		return 0;
	if (r->op != T_CONSTANT || r->value > 2)
		return 0;
	else
		count = r->value;

	/* Being super clever doesn't help if we need the value anyway */
	if (!nr && (n->op == T_PLUSPLUS || n->op == T_MINUSMINUS))
		return 0;

	v = l->value;

	switch(l->op) {
	case T_NAME:
		name = namestr(l->snum);
		while(count--) {
			output("%s _%s+%d", op, name, v);
			if (sz == 2) {
				output("beq X%d", ++xlabel);
				output("%s _%s+%d", op, name, v + 1);
				label("X%d", xlabel);
			}
		}
		if (!nr) {
			output("lda _%s+%d", name, v);
			if (sz == 2)
				output("ldx _%s+%d", name, v + 1);
		}
		return 1;
	case T_LABEL:
		while(count--) {
			output("%s T%d+%d", op, (unsigned)l->val2, v);
			if (sz == 2) {
				output("beq X%d", ++xlabel);
				output("%s T%d+%d", op, (unsigned)l->val2, v + 1);
				label("X%d", xlabel);
			}
		}
		if (nr == 1) {
			output("lda T%d+%d", (unsigned)l->val2, v);
			if (sz == 2)
				output("ldx T%d+%d", (unsigned)l->val2, v + 1);
		}
		return 1;
	case T_ARGUMENT:
		v += argbase + frame_len;
	case T_LOCAL:
		return 0;
		/* Don't seem to have a suitable addressing mode */
	}
	return 0;
}


/* Do a 16bit operation upper half by switching X into A */
static unsigned try_via_x(struct node *n, const char *op, void (*pre)(struct node *))
{
	if (optsize)  {
		struct node *r = n->right;
		unsigned rop = r->op;
		if (rop == T_LREF) {
			if (r->value == 0) { 
				output("jsr __%ssp0", op);
				set_reg(R_Y, 1);
				invalidate_x();
				invalidate_a();
				return 1;
			} else if (r->value < 255) {
				output("ldy #%d", r->value);
				output("jsr __%spy", op);
				const_y_set(reg[R_Y].value + 1);
				invalidate_x();
				invalidate_a();
				return 1;
			}
		}
		if (rop == T_CONSTANT && r->value < 256) {
			load_y(r->value);
			output("jsr __%s8y", op);
			invalidate_x();
			invalidate_a();
			return 1;
		}
	}
	/* Name and lbref are progably not worth it as have to go via tmp */
		
	if (do_pri8(n, op, pre) == 0)
		return 0;
	output("pha");
	output("txa");
	memcpy(&reg[R_A], &reg[R_X], sizeof(struct regtrack));
	do_pri8hi(n, op, pre_none);
	output("tax");
	output("pla");
	invalidate_a();
	invalidate_x();
}

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
	n->val2 = o->val2;
	n->snum = o->snum;
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

	/* We can use these directly with primary operators on A */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME || (op == T_LREF && n->value < 255))
		return 10;
	/* Can go via @tmp */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	/* Hard */
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
	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
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
		if (nt == r->type || (nt ^ r->type) == UNSIGNED ||
		 (PTR(nt) && PTR(r->type))) {
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
	output(".export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		output(".code");
		break;
	case A_DATA:
		output(".data");
		break;
	case A_LITERAL:
		output(".literal");
		break;
	case A_BSS:
		output(".bss");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
	invalidate_regs();
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	if (size == 0)
		return;

	sp += size;
	/* Maybe shortcut some common values ? */

	if (size < 256) {
		load_a(size);
		gen_internal("enter8");
		return;
	}
	load_a(size & 0xFF);
	load_y(size >> 8);
	gen_internal("enter16");
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != size) {
		error("sp");
	}
	sp -= size;
	/* TODO arg removal */
	if (size > 256) {
		/* Ugly as we need to preserve AX */
		output("pha");
		load_a(size & 0xFF);
		load_y(size >> 8);
		gen_internal("return16");
		output("pla");
	}
	else if (size) {
		load_y(size);
		gen_internal("return8");
	}
	else
		output("rts");
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	label("L%d%s", n, tail);
	invalidate_regs();
}

unsigned gen_exit(const char *tail, unsigned n)
{
	/* Want to use BRA/BRL if we have it */
	if (frame_len == 0) {
		output("rts");
		return 1;
	} else {
		output("jmp L%d%s", n, tail);
		return 0;
	}
}

void gen_jump(const char *tail, unsigned n)
{
	/* Want to use BRA/BRL if we have it */
	output("jmp L%d%s", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	output("jeq L%d%s", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	output("jne L%d%s", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	/* TODO: fix to work with output */
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n");
	output(".word Sw%d\n", n);
}

void gen_switchdata(unsigned n, unsigned size)
{
	label("Sw%d", n);
	output("\t.word %d", size);
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
	output(".ds %d", value);
}

void gen_text_data(unsigned n)
{
	output(".word T%d", n);
}

void gen_literal(unsigned n)
{
	if (n)
		label("T%d", n);
}

void gen_name(struct node *n)
{
	output(".word _%s+%d", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		output(".word %u", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		output(".byte %u", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		output(".word %d", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		output(".word %d", (unsigned) (value & 0xFFFF));
		output(".word %d", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	output(".code");
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
 *	The million dollar question - where do working temporaries go
 *	- user stack
 *		- plenty of room
 *		- can access indirectly
 *	- system stack
 *		- less room
 *		- faster
 *		- must pull via A to access (or via X/Y on C02)
 *
 *	The system stack case is alas hard to generalize so we don't. We
 *	can however use the system stack via gen_shortcut if we find a
 *	must have case.
 *
 *	TODO; optimize push constant low, push constant 8bit value as 16bit
 *	in func args
 */
unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
	sp += s;
	/* These don't invalidate registers and set Y to 0, so handle them
	   directly */
	switch(s) {
		case 1:
			output("jsr __pushc");
			set_reg(R_Y, 0);
			return 1;
		case 2:
			output("jsr __push");
			set_reg(R_Y, 0);
			return 1;
		case 4:
			output("jsr __pushl");
			set_reg(R_Y, 0);
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

	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		if (n->val2) {
			/* Only clean up vararg. stdarg is cleaned up by
			   the called function */
			gen_internal("cleanup");
			gen_value(UINT, n->right->value);
		}
		sp -= n->right->value;
		return 1;
	case T_EQ:	/* address in XA, can we build right ? */
		/* We already rewrite simple left hand sides into LSTORE
		   NSTORE etc. Here we try and handle the other common
		   case of  complexexpression = simple. This is often the
		   case with things like  *x++ = 0; */
		if (s > 2)
			return 0;
		if (s == 1 && do_pri8(n, "lda", pre_stash)) {
			invalidate_a();
			if (cpu != NMOS_6502) {
				output("sta (@tmp)");
				return 1;
			}
			load_y(0);
			output("sta (@tmp),y");
			return 1;
		}
		else if (s == 2 && do_pri16(n, "ld", pre_stash)) {
			invalidate_x();
			invalidate_a();
			load_y(0);
			output("sta (@tmp),y");
			load_y(1);
			output("stx (@tmp),y");
			return 1;
		}
		/* Complex on both sides. Do these the hard way. Not as bad
		   as it seems as these are not common */
		return 0;
	case T_AND:
		/* There are some cases we can deal with */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((r->value & 0xFF00) == 0x0000)
					load_x(0);
				else if ((r->value & 0xFF00) != 0xFF00)
					return 0;
			}
			if ((r->value & 0xFF) == 0x00)
				load_a(0);
			else if ((r->value & 0xFF) != 0xFF) {
				output("and #%d", ((unsigned)r->value) & 0xFF);
				const_a_set(reg[R_A].value & r->value);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "and"))
			return 1;
		if (s == 2 && try_via_x(n, "and", pre_none))
			return 1;
		return pri_help(n, "andtmp");
	case T_OR:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((r->value & 0xFF00) == 0xFF00)
					load_x(0xFF);
				else if ((r->value & 0xFF00) != 0x0000)
					return 0;
			}
			if ((r->value & 0xFF) == 0xFF)
				load_a(0xFF);
			else if ((r->value & 0xFF) != 0x00) { 
				output("ora #%d", ((unsigned)r->value) & 0xFF);
				const_a_set(reg[R_A].value | r->value);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "ora"))
			return 1;
		if (s == 2 && try_via_x(n, "ora", pre_none))
			return 1;
		return pri_help(n, "oratmp");
	case T_HAT:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (s == 2) {
				if ((r->value & 0xFF00) != 0x0000)
					return 0;
			}
			if ((r->value & 0xFF) != 0x00) {
				output("eor #%d", ((unsigned)r->value) & 0xFF);
				const_a_set(reg[R_A].value ^ r->value);
			}
			return 1;
		}
		if (s == 1 && pri8(n, "eor"))
			return 1;
		if (s == 2 && try_via_x(n, "eor", pre_none))
			return 1;
		return pri_help(n, "xortmp");
	case T_PLUS:
		if (s > 2)
			return 0;
		if (s == 1 && do_pri8(n, "adc", pre_clc)) {
			if (r->op == T_CONSTANT)
				const_a_set(reg[R_A].value + r->value);
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			if (r->value <= 0xFF) {
				output("clc");
				output("adc #%d", (unsigned)(r->value & 0xFF));
				output("bcc X%d", ++xlabel);
				output("inx");
				label("X%d", xlabel);
				const_a_set(reg[R_X].value + (r->value & 0xFF));
				/* TODO: set up X properly if known */
				invalidate_x();
				return 1;
			}
			if (r->value == 256) {
				output("inx");
				const_x_set(reg[R_X].value + 1);
				return 1;
			}
			if (r->value == 512) {
				output("inx");
				output("inx");
				const_x_set(reg[R_X].value + 2);
				return 1;
			}
		}
		if (s == 1 && do_pri8(n, "adc", pre_clc)) {
			invalidate_a();
			return 1;
		}
		if (s == 2 && try_via_x(n, "adc", pre_clc))
			return 1;
		return pri_help(n, "adctmp");
	case T_MINUS:
		if (s > 2)
			return 0;
		if (s == 1 && do_pri8(n, "sbc", pre_sec)) {
			if (r->op == T_CONSTANT)
				const_a_set(reg[R_A].value - r->value);
			return 1;
		}
		if (s == 2 && r->op == T_CONSTANT) {
			if (r->value <= 0xFF) {
				output("sec");
				output("sbc #%d", (unsigned)(r->value & 0xFF));
				output("bcs X%d", ++xlabel);
				output("dex");
				label("X%d", xlabel);
				const_a_set(reg[R_A].value - (r->value & 0xFF));
				/* TODO: we shguld probably set this up */
				invalidate_x();
				return 1;
			}
			if (r->value == 256) {
				output("dex");
				return 1;
			}
			if (r->value == 512) {
				output("dex");
				output("dex");
				return 1;
			}
		}
		if (s == 1 && do_pri8(n, "sbc", pre_sec)) {
			invalidate_a();
			return 1;
		}
		if (s == 2 && try_via_x(n, "sbc", pre_sec))
			return 1;
		return pri_help(n, "sbctmp");
	case T_STAR:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT && r->value == 256 && (n->type & UNSIGNED)) {
			output("tax");
			memcpy(&reg[R_X], &reg[R_A], sizeof(struct regtrack));
			load_a(0);
			return 1;
		}
		/* TODO: power of 2 into add/shifts, short form helpers
		   for low consts 2,4 etc */
		return pri_help(n, "multmp");
	case T_SLASH:
		if (r->op == T_CONSTANT && r->value == 256 && (n->type & UNSIGNED)) {
			output("txa");
			memcpy(&reg[R_A], &reg[R_X], sizeof(struct regtrack));
			load_x(0);
			return 1;
		}
		/* TODO - power of 2 const into >> */
		return pri_help(n, "divtmp");
	case T_PERCENT:
		if (r->op == T_CONSTANT && r->value == 256 && (n->type & UNSIGNED)) {
			load_x(0);
			return 1;
		}
		return pri_help(n, "remtmp");
	/*
	 *	There are various < 0, 0, !0, > 0 optimizations to do here
	 *	TODO
	 */
	case T_EQEQ:
		if (r->op == T_CONSTANT && r->value == 0) {
			helper(n, "not");
			return 1;
		}
		return pri_help(n, "eqeqtmp");
	case T_GTEQ:
		return pri_help(n, "gteqtmp");
	case T_GT:
		return pri_help(n, "gttmp");
	case T_LTEQ:
		return pri_help(n, "lteqtmp");
	case T_LT:
		return pri_help(n, "lttmp");
	case T_BANGEQ:
		if (r->op == T_CONSTANT && r->value == 0) {
			helper(n, "bool");
			return 1;
		}
		return pri_help(n, "netmp");
	case T_LTLT:
		if (s == 2 && r->op == T_CONSTANT && r->value == 8) {
			output("tax");
			load_a(0);
			return 1;
		}
		return pri_help(n, "lstmp");
	case T_GTGT:
		if (s == 2 && r->op == T_CONSTANT && r->value == 8) {
			if (n->type & UNSIGNED) {
				output("txa");
				load_x(0);
				return 1;
			}
		}
		return pri_help(n, "gttmp");
	/* TODO: special case by 1,2,4, maybe inline byte cases ? */
	/* We want to spot trees where the object on the left is directly
	   addressible and fold them so we can generate inc _reg, bcc, inc _reg+1 etc */
	/* TODO: look at push/pop for nr in leftop_tmp as option when need result - esp on C02 */
	case T_PLUSPLUS:
		if (r->op == T_CONSTANT) {
			if (r->value == 1) {
				gen_internal("plusplus1");
				return 1;
			}
			if (r->value == 2) {
				gen_internal("plusplus2");
				return 2;
			}
			if (r->value == 4) {
				gen_internal("plusplus4");
				return 4;
			}
		}
		return pri_help(n, "plusplustmp");
	case T_MINUSMINUS:
		if (r->op == T_CONSTANT) {
			if (r->value == 1) {
				gen_internal("minusminus1");
				return 1;
			}
			if (r->value == 2) {
				gen_internal("minusminus2");
				return 1;
			}
			if (r->value == 4) {
				gen_internal("minusminus4");
				return 1;
			}
		}
		return pri_help(n, "minusminustmp");
	case T_PLUSEQ:
		if (r->op == T_CONSTANT) {
			if (r->value == 1) {
				gen_internal("pluseq1");
				return 1;
			}
			if (r->value == 2) {
				gen_internal("pluseq2");
				return 1;
			}
			if (r->value == 4) {
				gen_internal("pluseq4");
				return 1;
			}
			if (r->value < 256) {
				load_y(r->value);
				gen_internal("pluseqy");
				return 1;
			}
		}
		return pri_help(n, "pluseqtmp");
	case T_MINUSEQ:
		if (r->op == T_CONSTANT) {
			if (r->value == 1) {
				gen_internal("minuseq1");
				return 1;
			}
			if (r->value == 2) {
				gen_internal("minuseq2");
				return 1;
			}
			if (r->value == 4) {
				gen_internal("minuseq4");
				return 1;
			}
		}
		return pri_help(n, "minuseqtmp");
	case T_ANDEQ:
		return pri_help(n, "andeqtmp");
	case T_OREQ:
		return pri_help(n, "oreqtmp");
	case T_HATEQ:
		return pri_help(n, "oreqtmp");
	case T_ARGCOMMA:
		/* We generate these directly when we can to optimize the
		   call return overhead a bit but it has to be done by
		   the peepholer */
		if (s == 1)
			output("jsr __pushc");
		else if (s == 2)
			output("jsr __push");
		else
			output("jsr __pushl");
		set_reg(R_Y, 0);
		sp += s;
		return 1;
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

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned nr = n->flags & NORETURN;

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
	/* The left nay be a complex expression but also may be soemthing
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

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	if (!(rt & UNSIGNED)) {
		/* Signed */
		if (ls == 2) {
			load_x(0);
			output("ora #0");
			output("bmi X%d", ++xlabel);
			output("dex");
			label("X%d", xlabel);
			invalidate_x();
			return 1;
		}
		return 0;
	}
	if (ls == 4) {
		rs = get_size(rt);
		if (rs == 1) {
			load_x(0);
			output("stx @hireg");
			output("stx @hireg+1");
			return 1;
		} else {
			if (cpu == NMOS_6502) {
				load_y(0);
				output("sty @hireg");
				output("sty @hireg+1");
			} else {
				output("stz @hireg");
				output("stz @hireg+1");
			}
			return 1;
		}
	}
	if (ls == 2) {
		load_x(0);
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	char *name;
	unsigned nr = n->flags & NORETURN;

	v = n->value;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);
	switch(n->op) {
	/* FIXME: need to do 4 byte forms */
	case T_LREF:
		if (size == 1 && n->value == 0) {
			if (a_contains(n))
				return 1;
			/* Same length as simple load via Y but
			   sets X to 0 so avoids the casting cost */
			load_x(0);
			output("lda (@sp,x)");
			return 1;
		}
		/* Fall through */
	case T_NREF:
	case T_LBREF:
		if (size == 1) {
			if (a_contains(n))
				return 1;
			if (pri8(n, "lda")) {
				set_a_node(n);
				return 1;
			}
		} else if (size == 2) {
			if (xa_contains(n))
				return 1;
			if (pri16(n, "ld")) {
				set_xa_node(n);
				return 1;
			}
		}
		/* FIXME: need to do 4 byte forms ?? */
		return 0;
	case T_NSTORE:
	case T_LBSTORE:
	case T_LSTORE:
		if (size == 1 && pri8(n, "sta")) {
			set_a_node(n); 
			return 1;
		} else if (size == 2 && pri16(n, "st")) {
			set_xa_node(n);
			return 1;
		}
		/* FIXME: need to do 4byte forms **/
		return 0;
	case T_CALLNAME:
		invalidate_regs();
		output("jsr _%s+%d", namestr(n->snum), n->value);
		return 1;
	case T_EQ:
		/* store XA in top of stack addr  .. ugly */
		if (size > 2) 
			return 0;
		/* Maybe make this whole lot a pair of helpers ? */
		gen_internal("poptmpy0");
		set_reg(R_Y, 0);
		output("sta (@tmp),y");
		if (size == 2) {
			load_y(1);
			output("stx (@tmp),y");
		}
		invalidate_mem();
		return 1;	
	case T_FUNCCALL:
		/* For now just helper it */
		return 0;
	case T_DEREF:
		/* We could optimize the tracing a bit here. A deref
		   of memory where we know XA is a name, local etc is
		   one where we can udate the contents info TODO */
		if (size > 2)
			return 0;
		output("sta @tmp");
		output("stx @tmp+1");
		if (size == 1) {
			load_x(0);
			output ("lda (@tmp,x)");
			invalidate_a();
		} else {
			load_y(0);
			invalidate_a();
			output("lda (@tmp),y");
			load_y(1);
			invalidate_x();
			output("ldx (@tmp),y");
		}
		return 1;
	case T_CONSTANT:
		if (size > 2) {
			load_a(n->value >> 24);
			output("sta @hireg+1");
			load_a(n->value >> 16);
			output("sta @hireg");
		}
		/* We have to special case this to get the value setting right */
		if (size == 2)
			load_x(n->value >> 8);
		load_a(n->value & 0xFF);
		return 1;
	case T_NAME:
	case T_LABEL:
		if (size == 1 && pri8(n, "lda")) {
			invalidate_a();
			return 1;
		}
		if (size == 2 && pri16(n, "ld")) {
			invalidate_x(); 
			invalidate_a();
			return 1;
		}
		return 0;
	/* Local and argument are more complex so helper them */
	case T_CAST:
		return gen_cast(n);
	}
	return 0;
}
