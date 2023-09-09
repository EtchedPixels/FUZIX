/*
 *	The 680x/630x series processors are closely related except for the
 *	6809/6309 which is totally different and to us unrelated. All of them
 *	have a very small number of registers and an instruction set that is
 *	built around register/memory operations.
 *
 *	The original 6800 cannot push/pop the index register and has no 16bit
 *	maths operations. It is not considerded in this target at this point
 *
 *	The 6803 has a single index register and index relative addressing.
 *	There is almost no maths on the index register except an 8bit add.
 *	The processor also lacks the ability to do use the stack as an index
 *	but can copy the stack pointer to x in one instruction. Thus we have
 *	to play tracking games on the contents of X.
 *
 *	The 6303 is slightly pipelined and adds some bit operations that are
 *	mostly not relative to the compiler as well as a the ability to exchange
 *	X and D, which makes maths on the index much easier.
 *
 *	The 68HC11 adds an additional Y index register, which has a small
 *	performance penalty. For now we ignore Y and treat it as a 6803.
 *
 *	All conditional branches are short range, the assemnbler supports
 *	jxx which turns into the correct instruction combination for a longer
 *	conditional branch as needed.
 *
 *	Almsot every instruction changes the flags. This helps us hugely in
 *	most areas and is a total nightmare in a couple.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define T_NREF		(T_USER)	/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)	/* Function call by name */
#define T_NSTORE	(T_USER+2)	/* Store to a C global/static */
#define T_LREF		(T_USER+3)	/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)	/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)


/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

/*
 *	All our sizes are fairly predictable. Our stack is byte oriented
 *	so we push bytes as bytes.
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
 *	Register state tracking
 */

static unsigned value_d;
static unsigned value_x;
static unsigned state_d;
static unsigned state_x;

#define INVALID		0
#define	CONSTANT	1
#define	BCONSTANT	2	/* Only used with D tracker */
#define STACKREL	3	/* Only used with X tracker */

static void invalidate_d(void)
{
	state_d = INVALID;
}

static void load_d(unsigned v, unsigned keepc)
{
	unsigned char vl = v;
	unsigned char vh = v >> 8;
	unsigned char vdl = value_d;
	unsigned char vdh = value_d >> 8;

	/* Complete match */
	if (state_d == CONSTANT && value_d == v)
		return;

	/* We know the low byte and it matches */
	if ((state_d == CONSTANT || state_d == BCONSTANT) && vdl == vl) {
		/* Find the best way to get the byte set up */
		if (vh == vdl)
			puts("tba");
		else if (keepc)
			printf("\tldaa #%d\n", vh);
		else if (vh == vdh - 1)
			puts("deca");
		else if (vh == vdh + 1)
			puts("inca");
		else if (vh)
			printf("\tldaa #%d\n", vh);
		else
			puts("\tclra");
		state_d = CONSTANT;
		value_d = v;
		return;
	}
	/* See if the high byte matches */
	if (state_d == CONSTANT && vh == vdh) {
		/* Find the best way to get the byte set up */
		if (vl == vdh)
			puts("tab");
		else if (keepc)
			printf("\tldab #%d\n", vh);
		else if (vh == vdh - 1)
			puts("decb");
		else if (vh == vdh + 1)
			puts("incb");
		else if (vh)
			printf("\tldaa #%d\n", vh);
		else
			puts("\tclra");
		state_d = CONSTANT;
		value_d = v;
		return;
	}
	if (keepc || v)
		printf("\tldd #%d\n", v);
	else
		puts("\tclra\n\tclrb");
	state_d = CONSTANT;
	value_d = v;
}

/* TODO: optimize incb decb cases */
static void load_b(unsigned v, unsigned keepc)
{
	if ((state_d == BCONSTANT || state_d == CONSTANT)
	    && (value_d & 0xFF) == (v & 0xFF))
		return;
	if (state_d == CONSTANT) {
		value_d &= 0xFF00;
		value_d |= v & 0xFF;
	} else {
		state_d = BCONSTANT;
		value_d = v;
	}
	if (v == 0 && !keepc)
		puts("\tclrb");
	else
		printf("\tldab #%d\n", v);
}

static void modify_d(unsigned v)
{
	if (v == 0)
		return;
	if (state_d != CONSTANT) {
		printf("\taddd #%d\n", v);
		return;
	};
	v += value_d;
	/* Let the load routine figure out how to load this most efficiently */
	load_d(v, 0);
}

/* For now just track stack relative X. We might want to track symbol relative
   as well, but probably not constant. Due to the way X works we don't generate
   code in these helpers but instead in the code that uses them. We do want
   to track X as constant for function call loads and the like via X */

static void invalidate_x(void)
{
	state_x = INVALID;
}

static void load_x_s(void)
{
	state_x = STACKREL;
	value_x = sp;
}

static void modify_x(unsigned v)
{
	if (state_x != STACKREL)
		return;
	value_x += v;
}

static void repeated_instruction(const char *x, unsigned n)
{
	while (n--)
		puts(x);
}


/*
 *	Helpers
 */

/*
 *	If this returns 1 then a load must succeed. If it returns 0 a load
 *	may succeed but you'll only know by trying it.
 *
 *	True if we can reference this without damanging D, X may change
 */
static unsigned can_reference(struct node *n)
{
	unsigned op = n->op;
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	/* Simple constant loads */
	if (op == T_CONSTANT || op == T_NAME || op == T_LABEL)
		return 1;
	/* Simple memory loads */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	/* Complicated loads. Because we use X to generate the indexing
	   and save D in the process we can always load X with a local, it
	   might not be a simple loads but it will do the job */
	if (op == T_LREF)
		return 1;
	return 0;
}

/* Generate an X that can access offset spoff from the stack. Most of the
   time we are able to just load X as needed and keeping using n,X. O a bigger
   stack we have to play around with X a bit */

static unsigned gen_offset(int spoff, int spmax, unsigned save_d)
{
	if (state_x != STACKREL) {
		load_x_s();
		puts("\ttsx");
	}
	/* X is now some offset relative to what we need, but may be too big */
	spoff += value_x;
	if (spoff <= spmax)
		return spoff;
	/* The value is out of range of addressing to be used. Modify X */
	if (cpu == 6303 || cpu == 6811) {
		if (spmax - spoff < 5) {
			repeated_instruction("\tinx", spmax - spoff);
			modify_x(spmax - spoff);
		} else {
			puts("\txchg");
			printf("\taddd #%d\n", spoff);
			puts("\txchg");
			modify_x(spoff);
		}
		return 0;
	}
	if (spoff < 0)
		error("badgo");
	/* We only have ABX and we may have to save B */
	if (spmax - spoff < 10 - 2 * save_d)
		repeated_instruction("\tinx", spmax - spoff);
	else {
		unsigned n = spmax - spoff;
		/* Could in theory optimize if state of d is known and happens
		   to contain right byte - unlikely! */
		if (save_d)
			puts("\tstab @tmp");
		if (n >= 255) {
			puts("\tldab #255");
			while (n >= 255) {
				puts("\tabx");
				n -= 255;
			}
		}
		if (n)
			printf("\tldab #%d\n\tabx\n", n);
		if (save_d)
			puts("\tldab @tmp");
	}
	modify_x(spmax - spoff);
	return spmax;
}

/*
 *	The 8bit form is simple enough. For 16bits we need to generate
 *	either + 1 on the offset for the low byte, or >> 8 on the value
 *	for the first byte
 */
static void reference_op(struct node *n, const char *ins, unsigned s,
			 unsigned byte)
{
	unsigned offs;
	unsigned op = n->op;
	unsigned v = WORD(n->value);

	/* Big endian */
	if (s == 2 && byte) {
		/* Right half of word */
		if (op == T_CONSTANT) {
			if (byte == 2)
				v = BYTE(v);
			else
				v >>= 8;
		} else if (byte == 2)
			v++;
	}

	if (op == T_LREF) {
		/* This may generate additional code */
		offs = gen_offset(v, 255, 1);
		printf("\t%s %d,x\n", ins, offs);
		return;
	}
	printf("\t%s ", ins);
	switch (op) {
	case T_CONSTANT:
		printf("#%d\n", v);
		break;
	case T_NAME:
		printf("#%s+%d\n", namestr(n->snum), v);
		break;
	case T_LABEL:
		printf("T%d+%d\n", n->val2, v);
		break;
	case T_NREF:
		printf("%s+%d\n", namestr(n->snum), v);
		break;
	case T_LBREF:
		printf("\tT%d+%d\n", n->val2, v);
		break;
	default:
		error("rop");
	}
}

/*
 *	Load X with a node if possible, return 0 if not
 */
static unsigned load_x_with(struct node *n)
{
	unsigned op = n->op;
	unsigned s = get_size(n->type);

	if (s > 2)
		return 0;
	switch (op) {
	case T_CONSTANT:
	case T_NAME:
	case T_LABEL:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
		reference_op(n, "ldx", 2, 0);
		return 1;
	}
	return 0;
}

/*
 *	Load D with a node if possible, return 0 if not
 */
static unsigned load_d_with(struct node *n)
{
	unsigned op = n->op;
	unsigned s = get_size(n->type);
	unsigned v = WORD(n->value);
	if (s > 2)
		return 0;
	switch (op) {
	case T_CONSTANT:
		if (s == 2) {
			load_d(n->value, 0);
			reference_op(n, "ldd", 2, 0);
			return 1;
		}
		/* s == 1 */
		load_b(BYTE(v), 0);
		reference_op(n, "ldab", 1, 0);
		break;
	case T_NAME:
	case T_LABEL:
	case T_NREF:
	case T_LBREF:
	case T_LREF:
		invalidate_d();
		if (s == 2)
			reference_op(n, "ldd", 2, 0);
		else
			reference_op(n, "ldab", 1, 0);
		return 1;
	}
	return 0;
}

/*
 *	The left hand side is in D, the right hand side is directly
 *	referenceable. We generate the pair of ops
 *	xxxb ref / xxxa ref + 1 (or constant halves)
 */
static unsigned gen_logic8(struct node *n, const char *p, unsigned form)
{
	unsigned s = get_size(n->type);
	static char x[6];
	if (s > 2 || !can_reference(n->right))
		return 0;
	invalidate_d();
	strcpy(x, p);
	x[4] = 'b';
	/* TODO: optimize constant logic ops - eg and 0 bytes */
	reference_op(n->right, x, 1, 0);
	if (s == 2) {
		x[4] = 'a';
		reference_op(n->right, x, 2, 1);
	}
	return 1;
}

/*
 *	Same but the other argument is top of stack
 */
static unsigned gen_pair_pop(struct node *n, const char *p)
{
	unsigned s = get_size(n->type);
	unsigned offs;
	if (s > 2)
		return 0;

	/* Get the X offset to the top of stack variable */
	offs = gen_offset(sp, 255, 1);
	if (s == 2)
		printf("\t%sb 1,x\n\t%sa ,x\n\tpulx\n", p, p);
	else
		printf("\t%sb ,x\n\tins\n", p);
	invalidate_d();
	return 1;
}

/*
 *	Ditto but 16bit op forms exist
 */
static unsigned gen_op16(struct node *n, const char *op)
{
	unsigned s = get_size(n->type);
	if (s > 2 || !can_reference(n->right))
		return 0;
	/* TODO - we don't wany sub/add const to do invalidate */
	invalidate_d();
	reference_op(n->right, op, 2, 0);
	return 1;
}

/*
 *	D holds the one side, the other side is simple
 */
static unsigned gen_compare(struct node *n, const char *h)
{
	/* Do the subtract */
	if (gen_op16(n, "sub") == 0)
		return 0;
	if (n->op == T_CONSTANT)
		modify_d(n->value);
	else
		invalidate_d();
	/* Call the flags and bool helper */
	helper(n, h);
	n->flags |= ISBOOL;
	return 0;
}

/* Try and simplify compares */
static unsigned gen_comp_reversed(struct node *n, const char *rcond)
{
	/* If the right is simple gen_direct handles this for us */
	if (can_reference(n->left)) {
		codegen_lr(n->right);
		gen_compare(n->left, rcond);
		return 1;
	}
	return 0;
}


static unsigned gen_cast(struct node *n, struct node *r)
{
	unsigned lt = r->type;
	unsigned rt = n->type;
	unsigned ls;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;
	ls = get_size(lt);
	/* Shrink is free */
	if ((lt & ~UNSIGNED) < (rt & ~UNSIGNED))
		return 1;
	/* TODO should inline uint->ulong and uchar->ulong */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	puts("\tclra");
	return 1;
}

/*
 *	Turn commutative ?= operations into instructions via X
 */
static unsigned gen_xeqop(struct node *n, const char *cop, unsigned form)
{
	struct node *l = n->left;
	unsigned s = get_size(n->type);

	if (s > 2 || !can_reference(l))
		return 0;
	/* Get the modifcation value */
	codegen_lr(n->right);
	load_x_with(l);
	invalidate_d();
	/* TODO: optimize logic ops with constants */
	if (form == 0)
		printf("\t%sd ,x\n", cop);
	else {
		printf("\t%sa ,x\n", cop);
		printf("\t%sb 1,x\n", cop);
	}
	if (s == 2)
		printf("\tstd ,x\n");
	else
		printf("\tstab ,x\n");
	return 1;
}


static void shrink_stack(unsigned v)
{
	/* TODO: set properly */
	invalidate_x();
	if (v < 12) {
		while (v > 1) {
			puts("\tpulx");
			v -= 2;
		}
		if (v)
			puts("\tins");
	} else {
		if (cpu == 6303 || cpu == 6811) {
			puts("\ttsx\n\txchg");	/* SP into D */
			printf("\taddd #%d\n", v);
			puts("\txchg\n\ttxs");
		} else {
			puts("\tstab @tmp");
			if (v >= 255) {
				puts("\tldab #255");
				while (v >= 255) {
					puts("\tabx");
					v -= 255;
				}
			}
			switch (v) {
			case 1:
				puts("\tins");
				break;
			case 2:
				puts("\tpulx");
				break;
			default:
				printf("\tldab #%d\n\tabx\n", v);
				break;
			}
			puts("\tldab @tmp\n");
		}
	}
}

/* We assume this can always kill d */
static void grow_stack(unsigned v)
{
	unsigned cost = 12;
	if (cpu == 6803)
		cost = 22;
	if (v < cost) {
		while (v > 1) {
			puts("\tpshx");
			v -= 2;
		}
		if (v)
			puts("\tdes");
	} else {
		if (cpu == 6303 || cpu == 6811) {
			invalidate_x();
			puts("\ttsx\n\txchg");	/* SP into D */
			printf("\taddd #%d\n", -v);
			puts("\txchg\n\ttxs");
		} else {
			invalidate_d();
			puts("\tsts @tmp");
			printf("\tldd #%d\n", -v);
			puts("\taddd @tmp");
			puts("\tstd @tmp");
			puts("\tlds @tmp");
		}
	}
}



/*
 *	Tree rewriting. We turn variable references into single node along
 *	with calls to a function by name. Also re-order some operations
 *	for better code generation later
 */
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
 *	Heuristic for guessing what to put on the right. We are pretty
 *	flexible given our register/memory architecture
 */

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can load these directly into a register and we want the
	   constant on the right */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME)
		return 10;
	/* We can reference global objects cheaply too but we prefer
	   constants on the right */
	if (op == T_NREF || op == T_LBREF)
		return 9;
	/* Locals are usually fairly cheap to reference but can blow up
	   into complicated stuff on big stack ranges so for now punt
	   on that */
	return 0;
}

#define ARGBASE		2

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

	/* TODO: we can probably sanely inline 4 byte versions */
	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT
	    || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += ARGBASE + frame_len;
				squash_right(n, T_LREF);
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
					l->value += ARGBASE + frame_len;
				squash_left(n, T_LSTORE);
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
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR
	    || op == T_PLUS) {
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

void gen_segment(unsigned s)
{
	switch (s) {
	case A_CODE:
		printf("\t.text\n");
		break;
	case A_DATA:
		printf("\t.data\n");
		break;
	case A_LITERAL:
		printf("\t.literal\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
	invalidate_d();
}

/* Generate the stack frame */
void gen_frame(unsigned size)
{
	frame_len = size;
	grow_stack(size);
}

void gen_epilogue(unsigned size)
{
	if (sp != size) {
		error("sp");
	}
	sp -= size;
	shrink_stack(size);
	puts("\trts");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
	invalidate_d();
	invalidate_x();
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

void gen_switch(unsigned n, unsigned type)
{
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf("\n\t.word Sw%d\n", n);
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case(unsigned tag, unsigned entry)
{
	invalidate_d();
	invalidate_x();
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	invalidate_d();
	invalidate_x();
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	invalidate_d();
	invalidate_x();
	printf("\thjsr __");
}

void gen_helpclean(struct node *n)
{
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
	if (PTR(type)) {
		printf("\t.word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are big endian */
		printf("\t.word %d\n",
		       (unsigned) ((value >> 16) & 0xFFFF));
		printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.code\n");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
}

unsigned gen_push(struct node *n)
{
	/* Our push will put the object on the stack, so account for it */
	unsigned v = get_size(n->type);
	sp += v;
	if (v > 4)
		return 0;
	puts("\tpshb");
	if (v > 1)
		puts("\tpsha");
	if (v > 2)
		printf("\tldx @sreg\n\tpshx\n");
	return 1;
}

/*
 *	Handle two argument operations that can be performed without going
 *	via the stack. We've already tried to arrange that any simple
 *	commutative operations have the bits we need on the right hand side.
 */
unsigned gen_direct(struct node *n)
{
	unsigned op = n->op;
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	if (n->op == T_CLEANUP) {
		shrink_stack(n->right->value);
		return 1;
	}
	/* Operations we can do direct forms of in some cases */
	switch (n->op) {
		/* Maths and logic we can do inline */
	case T_PLUS:
		return gen_op16(n, "add");
	case T_MINUS:
		return gen_op16(n, "sub");
	case T_AND:
		return gen_logic8(n, "and", 1);
	case T_OR:
		return gen_logic8(n, "ora", 2);
	case T_HAT:
		return gen_logic8(n, "eor", 3);
		/* Comparisons we can generate a simpler helper. Need to look at
		   inline forms using subd next */
	case T_EQEQ:
		return gen_compare(n, "booleq");
	case T_BANGEQ:
		return gen_compare(n, "boolne");
	case T_GT:
		return gen_compare(n, "boolgt");
	case T_GTEQ:
		return gen_compare(n, "boolgteq");
	case T_LT:
		return gen_compare(n, "boollt");
	case T_LTEQ:
		return gen_compare(n, "boollteq");
#if 0
		/* Spot the easy cases and fix */
		/* Also do simple constant shifts here maybe ? */
	case T_MUL:
	case T_DIV:
	case T_PERCENT:
#endif
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
	unsigned op = n->op;

	if (op == T_COMMA) {
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
	}
	/* TODO: re-order assignments via X etc */
	switch (op) {
//      case T_MINUSEQ:  different as order of op matters
//              return gen_xeqop(n, "minus");
	case T_PLUSEQ:
		return gen_xeqop(n, "add", 0);
	case T_ANDEQ:
		return gen_xeqop(n, "and", 1);
	case T_OREQ:
		return gen_xeqop(n, "ora", 2);
	case T_HATEQ:
		return gen_xeqop(n, "eor", 3);
	case T_CAST:
		codegen_lr(n->right);
		return gen_cast(n, n->right);
		/* Handle comparisons with a simple left hand side. We generate
		   the right hand side, then do an inverse comparison */
	case T_EQEQ:
		return gen_comp_reversed(n, "eq");
	case T_BANGEQ:
		return gen_comp_reversed(n, "ne");
	case T_LT:
		return gen_comp_reversed(n, "ge");
	case T_LTEQ:
		return gen_comp_reversed(n, "gt");
	case T_GT:
		return gen_comp_reversed(n, "le");
	case T_GTEQ:
		return gen_comp_reversed(n, "lt");
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v = WORD(n->value);
	unsigned h = WORD(n->value >> 16);
	unsigned offs;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL)
		sp -= get_size(n->left->type);
	switch (n->op) {
	case T_CONSTANT:
		switch (s) {
		case 4:
			load_d(h, 0);
			printf("\tstd @hireg\n");
		case 2:
			load_d(v, 0);
			return 1;
		case 1:
			load_b(v, 0);
			return 1;
		}
		break;
	case T_NAME:
		printf("\tldd #%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LABEL:
		invalidate_d();
		printf("\tldd #T%d+%d\n", n->val2, v);
		return 1;
	case T_CALLNAME:
		printf("\tjsr _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		invalidate_x();
		printf("\tpulx\n");
		if (s == 1)
			printf("\tstab ,x\n");
		else if (s == 2)
			printf("\tstd ,x\n");
		else
			return 0;
		return 1;
	case T_AND:
		return gen_pair_pop(n, "and");
	case T_OR:
		return gen_pair_pop(n, "ora");
	case T_HAT:
		return gen_pair_pop(n, "eor");
	case T_CAST:
		return gen_cast(n, n->right);
	case T_NEGATE:
		if (s > 2)
			return 0;
		if (s == 2)
			puts("\tsubd @one");
		else
			puts("\tdecb");
		/* Fall through */
	case T_TILDE:
		if (s > 2)
			return 0;
		if (s == 2)
			puts("\tcoma");
		puts("\tcomb");
		invalidate_d();
		return 1;
	case T_DEREF:
		invalidate_x();
		if (s == 2)
			puts("\tpulx\nstd ,x\n");
		else if (s == 1)
			puts("\tpulx\nstab ,x\n");
		else
			return 0;
	case T_LREF:
		if (s == 4) {
			offs = gen_offset(n->value, 253, 0);
			printf("\tldd %d,x\nstd @hireg", offs);
			printf("\tldd %d,x\n", offs + 2);
			invalidate_d();
			return 1;
		}
		offs = gen_offset(n->value, 255, 0);
		if (s == 2)
			printf("\tldd %d,x\n", offs);
		else if (s == 1)
			printf("\tldab %d,x\n", offs);
		return 1;
	case T_NREF:
		if (s == 4) {
			printf("\tldd _%s+%d\n\tstd @hireg",
			       namestr(n->snum), WORD(n->value));
			printf("\tldd _%s+%d\n", namestr(n->snum),
			       WORD(n->value) + 2);
		} else if (s == 2)
			printf("\tldd _%s+%d\n", namestr(n->snum),
			       WORD(n->value));
		else if (s == 1)
			printf("\tldab _%s+%d\n", namestr(n->snum),
			       WORD(n->value));
		return 1;
	case T_LBREF:
		if (s == 4) {
			printf("\tldd T%d+%d\n\tstd @hireg", n->val2,
			       WORD(n->value));
			printf("\tldd T%d+%d\n", n->val2,
			       WORD(n->value) + 2);
		} else if (s == 2)
			printf("\tldd T%d+%d\n", n->val2, WORD(n->value));
		else if (s == 1)
			printf("\tldab T%d+%d\n", n->val2, WORD(n->value));
		return 1;
#if 0
	case T_PLUS:
	case T_MINUS:
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
#endif
	}
	return 0;
}
