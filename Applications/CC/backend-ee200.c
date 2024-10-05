/*
 *	EE200 / CD200 / Warrex CPU4
 *
 *	A 16bit minicomputer architecture from the 1970s.
 *
 *	Registers: A B X Y Z S P C
 *
 *	A and B are accumulators and work much as expected for 8 and 16bit maths. SUB is weird as it's
 *	a reverse subtract. As we can't work with constants for most ops we use A and B together to do
 *	things.
 *
 *	X Y and Z are pointer registers. X is more flexible and can be stored/loaded direct from memory
 *	but Y and Z are only loadable via other registers. X is also used in the system call/return sequence
 *	which is a hybrid branch-link and stack model. X is stacked, then X is set to the return addrerss. On
 *	a return P is loaded with X and the old X restored. This is used (although not for C code) so that on
 *	a call X points to the following arguments
 *
 *	A	16bit working value
 *	B	scratch
 *	X	working pointer (maybe make a reg var or tracked index helper)
 *	Y	register
 *	Z	scratch/working pointer
 *
 *	TODO: should we make namestr() take a node and handle LB forms itself or wrap it to do so ?
 *
 *	Need a way to tell codegen that we want to target a pointer register directly
 *	Probably this target wants the calle function to cleanup for non vararg
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	4	/* Bytes between arguments and locals if no reg saves */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */

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
	fprintf(stderr, "type %x\n", t);
	error("gs");
	return 0;
}

static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

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

/*
 *	Heuristic for guessing what to put on the right. This is very
 *	processor dependent. For 8080 we are quite limited especially
 *	with locals. In theory we could extend some things to 8bit
 *	locals on 8085 (ldsi, ldax d, mov e,a)
 */

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME)
		return 10;
	/* We can load this directly into a register but may need xchg pairs */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	return 0;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
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
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned segment)
{
	switch(segment) {
	case A_CODE:
		printf("\t.%s\n", codeseg);
		break;
	case A_DATA:
		printf("\t.data\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	case A_LITERAL:
		printf("\t.literal\n");
		break;
	default:
		error("gseg");
	}
}

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name, unsigned aframe)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size)
{
	frame_len = size;
	sp = 0;

	printf("\tstx (-s)\n");	/* Stack the return address */
	argbase = ARGBASE;
	if (func_flags & F_REG(1)) {
		printf("\txfr y,x\n");
		printf("\stx (-s)\n");
		argbase += 2;
	}
	/* TODO: Work out size for break */
	if (size > 10) {
		printf("\tldb %d\n", -size);
		printf("\tadd b,z\n");
		printf("\txfr b,s\n");
		return;
	}
	if (size & 1) {
		printf("\tstab (-s)\n");
		size--;
	}
	while(size) {
		printf("\tsta (-s)\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");
	/* Return in A, does need care on stack */
	sp -= size;
	if (size > 3) {
		printf("\tldb 0x%x\n", (uint16_t)size);
		printf("\tadd b,s\n");
		printf("\txfr b,s\n");
	} else {
		while (size) {
			printf("\tinr s\n");
			size--;
		}
	}
	if (func_flags & F_REG(1))
		printf("\ldx (s+)\nxfr x,y\n");
	printf("\tldx (s+)\n");
	printf("\trtr\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjnz L%d%s\n", n, tail);
}

static void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 3) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		printf("\tldb %d\n", v);
		printf("\tadd b,s\n");
		printf("\txfr b,s\n");
	} else {
		while(v) {
			printf("\tinr s\n");
			v--;
		}
	}
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */
void gen_helpcall(struct node *n)
{
	if (n->type == FLOAT)
		gen_push(n->right);
	printf("\tjsr __");
}

void gen_helpclean(struct node *n)
{
	unsigned s;

	if (n->type != FLOAT)
		return;

	s = 0;
	if (n->left) {
		s += get_size(n->left->type);
		/* gen_node already accounted for removing this thinking
		   the helper did the work, adjust it back as we didn't */
		sp += s;
	}
	s += get_size(n->right->type);
	gen_cleanup(s);
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tldx Sw%d\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_data(unsigned n)
{
	printf("\t.word T%d\n", n);
}

/* The label for a literal (currently only strings)
   TODO: if we add other literals we may need alignment here */

void gen_literal(unsigned n)
{
	if (n)
		printf("T%d:\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%d\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	unsigned w = WORD(value);
	if (PTR(type)) {
		printf("\t.word %u\n", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %d\n", w);
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.setcpu %d\n", cpu);
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
/*	printf(";SP=%d\n", sp); */
}

/*
 *	Try and generate shorter code for stuff we can directly access
 */

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 *
 *	TODO: add local direct access for stuff within range of S
 */
static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	/* TODO: we can access direct locals within range */
	/* TODO group the user ones together for a range check ? */
	if (op != T_CONSTANT && op != T_NAME && op != T_LABEL &&
		 op != T_NREF && op != T_LBREF && op != T_RREF)
		 return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into A B or X
 *	we merge this with the similar hl one in the main table ?
 */

static unsigned load_r_with(const char r, struct node *n)
{
	unsigned v = WORD(n->value);
	const char *name;

	switch(n->op) {
	case T_NAME:
		printf("\tld%c _%s+%d\n", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\tld%c T%d+%d\n", r, n->val2, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		if (r == 'a') {
			if (v == 0) {
				printf("\tcla\n");
				return 1;
			}
			if (v == 1) {
				printf("\tcla\n\tina\n");
				return 1;
			}
			if (v == 0xFFFF) {
				printf("\tcla\n\tiva\n");
				return 1;
			}
		}
		else if (v == 0) {
			printf("clr %c\n");
			return 1;
		}
		printf("\tld%c %d\n", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		printf("\tld%c (_%s+%d)\n", r, name, v);
		return 1;
	case T_LBREF:
		printf("\tld%c (T%d+%d)\n", r, n->val2, v);
		return 1;
	case T_RREF:
		printf("xfr y,%c\n");
		return 1;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_rb_with(struct node *n)
{
	switch(n->op) {
	case T_NAME:
		printf("\tld%cb _%s+%d\n", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\tld%cb T%d+%d\n", r, n->val2, v);
		return 1;
	case T_CONSTANT:
		printf("\tld%cb %d\n", r, BYTE(v));
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		printf("\tld%cb (_%s+%d)\n", r, name, v);
		return 1;
	case T_LBREF:
		printf("\tld%cb (T%d+%d)\n", r, n->val2, v);
		return 1;
	case T_RREF:
		printf("xfrb y,%c\n");
		return 1;
	default:
		return 0;
	}
	return 1;
}

static void repeated_op(const char *o, unsigned n)
{
	while(n--)
		printf("\t%s\n", o);
}

static void load_reg(struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	printf("\txfr y,a\n");
}

static void save_reg(unsigned s)
{
	printf("\txfr a,y\n");
}

/* Generate faster helper ops using B to pass second arg */
static unsigned gen_bop(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2) {
		if (load_r_with('b', r) == 0)
			return 0;
	} else {
		if (load_rb_with(r) == 0)
			return 0;
	}
	if (sign)
		helper_s(n, op);
	else
		helper(n, op);
	return 1;
}

static unsigned gen_compc(const char *op, struct node *n, struct node *r, unsigned sign)
{
	if (r->op == T_CONSTANT && r->value == 0) {
		char buf[10];
		strcpy(buf, op);
		strcat(buf, "0");
		if (sign)
			helper_s(n, buf);
		else
			helper(n, buf);
		n->flags |= ISBOOL;
		return 1;
	}
	if (gen_bop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

static int count_mul_cost(unsigned n)
{
	int cost = 0;
	while(n > 1) {
		if (n & 1)
			cost += 6;	/* push pop add */
		n >>= 1;
		cost++;			/* sla */
	}
	return cost;
}

/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
	unsigned pops = 0;
	while(n > 1) {
		if (n & 1) {
			pops++;
			printf("\tsta (-s)\n");
		}
		printf("\tsla\n");
		n >>= 1;
	}
	while(pops--) {
		printf("\tldb (s+)\n\tadd b,a\n");
	}
}

static unsigned can_fast_mul(unsigned s, unsigned n)
{
	/* Pulled out of my hat 8) */
	unsigned cost = 15 + 3 * opt;
	/* The base cost of the helper is 6 lxi de, n; call, but this may be too aggressive
	   given the cost of mulde TODO */
	if (optsize)
		cost = 10;
	if (s > 2)
		return 0;
	if (n == 0 || count_mul_cost(n) <= cost)
		return 1;
	return 0;
}

static void gen_fast_mul(unsigned s, unsigned n)
{

	if (n == 0)
		printf("\tcla\n");
	else
		write_mul(n);
}

static unsigned gen_fast_div(unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n & (n - 1))
		return 0;

	while(n > 1) {
		printf("\tsra\n");
		n >>= 1;
	}
	return 1;
}

/* TODO : we could in theory optimize xor 255 with cpl ? */
static unsigned gen_logicc(struct node *n, unsigned s, const char *op, unsigned v, unsigned code)
{
	if (s > 2 || (n && n->op != T_CONSTANT))
		return 0;

	if (v == 0) {
		if (code == 1)
			printf("\tcla\n");
	/* TODO: spot byte form for size 1 */
	} else if (v == 65535 && code != 3) {
		if (code == 2)
			printf("\tcla\n\tdca\n");
	} else {
		if (s == 2) {
			printf("\tldb %d\n", v);
			printf("\t%s b,a\n", op);
		} else {
			printf("\tldbb %d\n", v);
			printf("\t%sb b,a\n", op);
		}
	}
	return 1;
}

/* TODO: check the 0 case here and in 8080/Z80 */
static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		printf("\cla\n");
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "and", mask, 1);
		return 1;
	}
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	The 8080 is pretty basic so there isn't a lot we turn around here. As
 *	proof of concept we deal with the add case. Other processors may be
 *	able to handle a lot more.
 *
 *	If your processor is good at subtracts you may also want to rewrite
 *	constant on the left subtracts in the rewrite rules into some kind of
 *	rsub operator.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;

	/* We only deal with simple cases for now */
	if (r) {
		if (!access_direct(n->right))
			return 0;
		v = r->value;
	}

	switch (n->op) {
	case T_CLEANUP:
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
		if (s > 2)
			return 0;
		if (s == 1)
			printf("\tstab ");
		else
			printf("\tsta ");
		printf("(_%s+%d)\n", namestr(n->snum), WORD(n->value));
			return 1;
		/* TODO 4/8 for long etc */
		return 0;
	case T_LBSTORE:
		if (s > 2)
			return 0;
		if (s == 1)
			printf("\tstab");
		else
			printf("\tsta");
		printf(" (T%d+%d)\n", n->val2, v);
		return 1;
	case T_RSTORE:
		save_reg(s);
		return 1;
	case T_EQ:
		/* The address is in A at this point */
		if (s == 2) {
			if (load_r_with('b', r) == 0)
				error("teq");
			printf("\txax\n");
			printf("\tsta (x)\n");
			return 1;
		} else if (s == 1) {
			if (load_rb_with('b', r) == 0)
				error("teq");
			printf("\txax\n");
			printf("\tstab (x)\n");
			return 1;
		}
		return 0;
	case T_PLUS:
		if (r->op == T_CONSTANT) {
			if (v < 4 && s <= 2) {
				if (s == 1)
					repeated_op("inab", v);
				else
					repeated_op("ina", v);
				return 1;
			}
		}
		if (s <= 2) {
			/* LHS is in A at the moment, end up with the result in A */
			if (s == 1) {
				if (load_rb_with('b', r) == 0)
					return 0;
				printf("\tadd b,a\n");
				return 1;
			}
			/* Short cut register case */
			if (r->op == T_REG) {
				printf("\tadd y,a\n");
				return 1;
			}
			if (s > 2 || load_r_with('b', r) == 0)
				return 0;
			printf("\tadd b,a\n");
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (v < 4 && s <= 2) {
				if (s == 1)
					repeated_op("dcab", v);
				else
					repeated_op("dca", v);
				return 1;
			}
		}
		if (s <= 2) {
			/* LHS is in A at the moment, end up with the result in A */
			if (s == 1) {
				if (load_rb_with('b', r) == 0)
					return 0;
				printf("\tadd b,a\n");
				return 1;
			}
			/* Short cut register case */
			if (r->op == T_REG) {
				printf("\tadd y,a\n");
				return 1;
			}
			if (s > 2 || load_r_with('b', r) == 0)
				return 0;
			printf("\tadd b,a\n");
			return 1;
		}
		return 0;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && can_fast_mul(s, r->value)) {
				 gen_fast_mul(s, r->value);
				return 1;
			}
		}
		return gen_bop("mulb", n, r, 0);
	case T_SLASH:
		/* FIXME: shift is signed so this is wrong for large numbers ? */
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_div(s, r->value))
				return 1;
		}
		return gen_bop("divb", n, r, 1);
	case T_PERCENT:
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_remainder(s, r->value))
				return 1;
		}
		return gen_bop("remde", n, r, 1);
	case T_AND:
		if (gen_logicc(r, s, "and", r->value, 1))
			return 1;
		return gen_bop("bandb", n, r, 0);
	case T_OR:
		if (gen_logicc(r, s, "or", r->value, 2))
			return 1;
		return gen_bop("borb", n, r, 0);
	case T_HAT:
		if (gen_logicc(r, s, "xor", r->value, 3))
			return 1;
		return gen_bop("bxorb", n, r, 0);
	case T_EQEQ:
		return gen_compc("cmpeq", n, r, 0);
	case T_GTEQ:
		return gen_compc("cmpgteq", n, r, 1);
	case T_GT:
		return gen_compc("cmpgt", n, r, 1);
	case T_LTEQ:
		return gen_compc("cmplteq", n, r, 1);
	case T_LT:
		return gen_compc("cmplt", n, r, 1);
	case T_BANGEQ:
		return gen_compc("cmpne", n, r, 0);
	case T_LTLT:
		if (s <= 2 && r->op == T_CONSTANT && r->value <= 8) {
			repeated_op("sla", r->value);
			return 1;
		}
		return gen_bop("shlb", n, r, 0);
	case T_GTGT:
		/* >> by 8 unsigned */
		/* Sugned right shift 16bit */
		if (!(n->type & UNSIGNED)) {
			if (r->op == T_CONSTANT && r->value < 8) {
				if (s == 2) {
					repeated_op("sra", r->value);
					return 1;
				}
				if (s == 1) {
					repeated_op("srab", r->value);
					return 1;
				}
			}
		}
		return gen_bop("shrb", n, r, 1);
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_PLUSEQ:
		if (s == 1) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tldab (x)\n\taddb b,a\n\tstab (x)\n");
			return 1;
		}
		if (s == 2) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tlda (x)\n\tadd b,a\n\tsta (x)\n");
			return 1;
		}
		return gen_bop("pluseqb", n, r, 0);
	case T_MINUSMINUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		if (s == 1) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tldab (x)\n\tsubb b,a\n\tstbb (x)\n");
			if (!(n->flags & NORETURN))
				printf("\txfr b,a\n");
			return 1;
		}
		if (s == 2) {
			if (load_r_with('b', r) == 0)
				return 0;
			printf("\txax\n\tlda (x)\n\tsub b,a\n\tstb (x)\n");
			if (!(n->flags & NORETURN))
				printf("\txfr b,a\n");
			return 1;
		}
		return gen_bop("minuseqb", n, r, 0);
	case T_ANDEQ:
		if (s == 1) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tldab (x)\nandb b,a\n\tstab (x)\n");
			return 1;
		}
		if (s == 2) {
			if (load_r_with('b', r) == 0)
				return 0;
			printf("\txax\n\tlda (x)\nand b,a\n\tsta (x)\n");
			return 1;
		}
		return gen_bop("andeqb", n, r, 0);
	case T_OREQ:
		if (s == 1) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tldab (x)\nor b,a\n\tstab (x)\n");
			return 1;
		}
		if (s == 2) {
			if (load_r_with('b', r) == 0)
				return 0;
			printf("\txax\n\tlda (x)\nor b,a\n\tsta (x)\n");
			return 1;
		}
		return gen_bop("oreqb", n, r, 0);
	case T_HATEQ:
		if (s == 1) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tldab (x)\nxor b,a\n\tstab (x)\n");
			return 1;
		}
		if (s == 2) {
			if (load_rb_with('b', r) == 0)
				return 0;
			printf("\txax\n\tlda (x)\nxor b,a\n\tsta (x)\n");
			return 1;
		}
		return gen_bop("xoreqb", n, r, 0);
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

static unsigned reg_canincdec(struct node *n, unsigned s, int v)
{
	/* We only deal with short and char for register */
	/* Is the shortcut worth it ? */
	if (n->op != T_CONSTANT || v > 8 || v < -8)
		return 0;
	return 1;
}

static unsigned reg_incdec(unsigned s, int v)
{
	if (s == 1) {
		if (v < 0)
			repeated_op("dcr c", -v);
		else
			repeated_op("inr c", v);
	} else {
		if (v < 0)
			repeated_op("dcx b", -v);
		else
			repeated_op("inx b", v);
	}
	return 1;
}

/*
 *	Allow the code generator to short cut any subtrees it can directly
 *	generate.
 */
unsigned gen_shortcut(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned v;
	unsigned nr = n->flags & NORETURN;

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
	/* Re-order assignments we can do the simple way */
	if (n->op == T_NSTORE && s <= 2) {
		codegen_lr(r);
		/* Expression result is now in HL */
		if (s == 2)
			printf("\tsta");
		else
			printf("\tstab");
		printf(" _%s+%d\n", namestr(n->snum), WORD(n->value));
		return 1;
	}
	/* We can do locals within range */
	if (n->op == T_LSTORE && s <= 2) {
		if (n->value + sp < 128) {
			codegen_lr(r);
			printf("\tldsi %d\n", WORD(n->value + sp));
			if (s == 2)
				printf("\tsta %d(s)\n", WORD(n->value + sp));
			else
				printf("\tstab %d(s)\n", WORD(n->value + sp));
			return 1;
		}
	}
	/* Shortcut any initialization of Y we can do directly */
	if (n->op == T_RSTORE) {
		if (load_r_with('y', r))
			return 1;
		return 0;
	}
	/* Assignment to *Y, byte or word pointer always */
	if (n->op == T_REQ) {
		/* Try and get the value into B */
		if (s == 1) {
			if (!load_rb_with('a',r))
				codegen_lr(r);
			printf("\tstab (y)\n");
		} else {
			if (!load_r_with('a',r))
				codegen_lr(r);
			printf("\tsta (y)\n");
		}
		return 1;
	}
	/* ?? LBSTORE */
	/* Register targetted ops. These are totally different to the normal EQ ops because
	   we don't have an address we can push and then do a memory op */
	if (l && l->op == T_REG) {
		v = r->value;
		switch(n->op) {
		case T_PLUSPLUS:
			if (reg_canincdec(r, s, v)) {
				load_reg(n, s);
				reg_incdec(s, v);
				return 1;
			}
			if (!nr) {
				printf("\tsta (-s)\n");
				sp += 2;
			}
			/* Fall through */
		case T_PLUSEQ:
			if (reg_canincdec(r, s, v)) {
				reg_incdec(s, v);
				if (nr)
					return 1;
				if (n->op == T_PLUSEQ) {
					load_reg(n, s);
				}
			} else {
				/* Amount to add into A */
				codegen_lr(r);
				printf("\tadd y,a\n");
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				printf("\tlda (s+)\n");
				sp -= 2;
			}
			return 1;
		case T_MINUSMINUS:
			if (!(n->flags & NORETURN)) {
				if (reg_canincdec(r, s, -v)) {
					load_reg(n, s);
					reg_incdec(s, -v);
					return 1;
				}
				codegen_lr(r);
				helper(n, "ysub");
				return 1;
			}
			/* If we don't care about the return they look the same so fall
			   through */
		case T_MINUSEQ:
			if (reg_canincdec(r, s, -v)) {
				reg_incdec(s, -v);
				load_reg(n, s);
				return 1;
			}
			/* Get the subtraction value into A */
			codegen_lr(r);
			helper(n, "ysub");
			/* Result is only left in Y reload if needed */
			load_reg(n, s);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					load_reg(NULL, s);
					gen_fast_mul(s, v);
					save_reg(s);
					return 1;
				}
			}
			codegen_lr(r);
			helper(n, "ymul");
			return 1;
		case T_SLASHEQ:
			/* TODO: power of 2 constant divide maybe ? */
			codegen_lr(r);
			helper_s(n, "ydiv");
			return 1;
		case T_PERCENTEQ:
			/* TODO: spot % 256 case */
			codegen_lr(r);
			helper(n, "yrem");
			return 1;
		case T_SHLEQ:
			if (r->op == T_CONSTANT) {
				if (s == 1) {
					repeated_op("addb y,y", v);
					load_reg(n, s);
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					printf("\tclr y\n");
					load_reg(n, s);
					return 1;
				}
				repeated_op("add y,y", v);
				load_reg(n, s);
				return 1;
			}
			codegen_lr(r);
			helper(n, "yshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT) {
				if (v >= 8 && s == 1) {
					printf("\clrb y\n");
					load_reg(n, s);
					return 1;
				}
				if (v >= 16) {
					printf("\tclr y\n");
					load_reg(n, s);
					return 1;
				}
				if (!(n->type & UNSIGNED) && v < 2 + 4 * opt) {
					repeated_op("sra y", v);
					load_reg(n, s);
					return 1;
				}
			}
			codegen_lr(r);
			helper_s(n, "yshr");
			return 1;
		case T_ANDEQ:
			codegen_lr(r);
			printf("\tand y,a\n\txay\n");
			return 1;
		case T_OREQ:
			codegen_lr(r);
			printf("\tor y,a\n\txay\n");
			return 1;
		case T_HATEQ:
			codegen_lr(r);
			printf("\txor y,a\n\txay\n");
			return 1;
		}
	}
	return 0;
}

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;

	switch(size) {
	case 2:
		printf("\sta (-s)h\n");
		return 1;
	case 4:
		if (optsize)
			printf("\tjsr __pushl\n");
		else
			printf("\tldb (__hireg\n\tstb (-s)\n\tsta (-s)\n");
		return 1;
	default:
		return 0;
	}
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;

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
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	printf("\tldbb 0xFF\n\tand b,a\n");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	char *name;
	unsigned nr = n->flags & NORETURN;
	/* We adjust sp so track the pre-adjustment one too when we need it */

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NREF:
		if (size == 1) {
			printf("\tldab (_%s+%d)\n", namestr(n->snum), v);
		} else if (size == 2) {
			printf("\tlda (_%s+%d)\n", namestr(n->snum), v);
			return 1;
		} else if (size == 4) {
			printf("\tlda (_%s+%d)\n", namestr(n->snum), v + 2);
			printf("\tsta (__hireg)\n");
			printf("\tlda (_%s+%d)\n", namestr(n->snum), v);
		} else
			error("nrb");
		return 1;
	case T_LBREF:
		if (size == 1) {
			printf("\tldab (T%d+%d)\n", n->val2, v);
		} else if (size == 2) {
			printf("\tlda (T%d+%d)\n", n->val2, v);
		} else if (size == 4) {
			printf("\tlda (T%d+%d)\n", n->val2, v + 2);
			printf("\tsta (__hireg)\n");
			printf("\tlda (T%d+%d)\n", n->val2, v);
		} else
			error("lbrb");
		return 1;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		printf(";L sp %d %s(%ld)\n", sp, namestr(n->snum), n->value);
		if (nr)
			return 1;
		v += sp;
		if (v < 126 && size == 4) {
			printf("\tlda %d(s)\n", WORD(n->value) + 2);
			printf("\tsta (__hireg)\n");
			printf("\tlda %d(s)\n", WORD(n->value));
			return 1;
		}
		if (v < 128) {
			if (size == 2)
				printf("\tlda %d(s)\n", WORD(n->value));
			else
				printf("\tldab %d(s)\n", WORD(n->value));
			return 1;
		}
		/* Via helper magic for compactness */
		if (size == 1)
			name = "ldbyte";
		else
			name = "ldword";
		printf("\tldb %d\n\tjsr __%sw\n", v + 2, name);
		if (size == 4)
			printf("\tldb 2(z)\n\tstb (__hireg)\n");
		return 1;
	case T_RREF:
		if (nr)
			return 1;
		printf("\txfr y,a\n");
		return 1;
	case T_NSTORE:
		if (size == 4) {
			printf("\tsta (%s+%d)\n", namestr(n->snum), v);
			printf("\tldb (__hireg)\nstb (%s+%d)\n",
				namestr(n->snum), v + 2);
			return 1;
		}
		if (size == 1)
			printf("\tstab");
		else
			printf("\tsta");
		printf(" (_%s+%d)\n", namestr(n->snum), v);
		return 1;
	case T_LBSTORE:
		if (size == 4) {
			printf("\tsta (T%d+%d)\n", n->val2, v);
			printf("ldb (__hireg)\n\tstb (T%d+%d)\n",
				n->val2, v + 2);
			return 1;
		}
		if (size == 1)
			printf("\tstab");
		else
			printf("\tsta");
		printf(" (T%d+%d)\n", n->val2, v);
		return 1;
	case T_LSTORE:
/*		printf(";L sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		v += sp;
		if (size == 4 && v < 126) {
			printf("\tsta %d(s)\n\tldb (__hireg)\n\tstb %d(s)\n",
				v, v + 2);
			return 1;
		}
		if (v < 128) {
			if (size == 1)
				printf("\tstab %d(s)\n", v);
			else
				printf("\tsta %d(s)\n", v);
			return 1;
		}
		/* For -O3 they asked for it so inline the lot */
		/* We dealt with size one above */
		if (opt > 2) {
			printf("\tldb %d\n\tadd s,b\n\txfr b,z\n", v);
			if (size == 1) {
				printf("\tstab (z)\n");
				return 1;
			}
			printf("\tsta (z)\n");
			if (size == 4)
				printf("\tldb (__hireg)\n\tstb 2(z)\n");
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		/* Can rewrite some of them into rst if need be */
		if (size == 1)
			name = "stbyte";
		else
			name = "stword";
		printf("\tldb %d\n", v + 2);
		printf("\tjsr __%s\n", name);
		/* The helper is guaranteed to leave Z pointing to the value */
		if (size == 4)
			printf("\tldb (__hireg)\n\tstb 2(z)\n");
		return 1;
	case T_RSTORE:
		save_reg(size);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tjsr _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		printf("\txaz\n");
		if (size == 1) {
			printf("\tstab (z)\n");
			return 1;
		}
		printf("\tsta (z)\n");
		if (size == 4) {
			printf("\tldb (__hireg)\n");
			printf("\tstb 2(z)\n");
		}
		break;
	case T_RDEREF:
		if (size == 1) {
			printf("\tldab (y)\n");
			return 1;
		}
		printf("\tlda (y)\n");
		if (size == 4)
			printf("\tldb 2(y)\n\tstb (__hireg)\n");
		return 1;
	case T_DEREF:
		printf("\txaz\n");
		/* This pattern wants pulling into a helper passed reg/size ? */
		if (size == 1) {
			printf("\tldab (z)\n");
			return 1;
		}
		printf("\tlda (z)\n");
		if (size == 4)
			printf("\tldb 2(z)\n\tstb (__hireg)\n");
		break;
	case T_FUNCCALL:
		printf("\tjsr __jsra\n");
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		/* Used for const strings and local static */
		printf("\tlda T%d+%d\n", n->val2, v);
		return 1;
	case T_CONSTANT:
		if (nr)
			return 1;
		switch(size) {
		case 4:
			printf("\tlda %u\n", ((v >> 16) & 0xFFFF));
			printf("\tsta (__hireg)\n");
		case 2:
			printf("\tlda %d\n", (v & 0xFFFF));
			return 1;
		case 1:
			printf("\tldab %d\n", (v & 0xFF));
			return 1;
		}
		break;
	case T_NAME:
		if (nr)
			return 1;
		printf("\tlda _%s+%d\n", namestr(n->snum), v);
		return 1;
	/* FIXME: LBNAME ?? */
	case T_LOCAL:
		if (nr)
			return 1;
		v += sp;
/*		printf(";LO sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		printf("\tlda %d\n\tadd s,an", v);
		return 1;
	case T_ARGUMENT:
		if (nr)
			return 1;
		v += frame_len + argbase + sp;
		printf("\tlda %d\n\tadd s,an", v);
		return 1;
	case T_REG:
		if (nr)
			return 1;
		/* A register has no address.. we need to sort this out */
		error("rega");
		return 1;
	case T_CAST:
		if (nr)
			return 1;
		return gen_cast(n);
	case T_PLUS:
		if (size <= 2) {
			printf("\tldb (s+)\n\tadd b,a\n");
			return 1;
		}
		break;
	}
	return 0;
}
