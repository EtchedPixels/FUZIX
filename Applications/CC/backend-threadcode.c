#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/* For now assume 8/16bit */
#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned frame_off;	/* Space between arg and local */
static unsigned has_fp;		/* Uses a frame pointer ? */
static unsigned big_endian;	/* Machine word endianness */

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)

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
	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += frame_len;
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
					l->value += frame_len;
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
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		printf("\t%s\n", codeseg);
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
}

/* Generate the stack frame */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size + frame_off;
	sp += size;
	gen_helpcall(NULL);
	printf("fnenter\n");
	gen_value(UINT, size);
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != size) {
		error("sp");
	}
	sp -= size;
	gen_helpcall(NULL);
	printf("fnexit\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

unsigned gen_exit(const char *tail, unsigned n)
{
	gen_helpcall(NULL);
	printf("fnexit\n");
	return 1;
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\t.word __jump\n");
	printf("\t.word L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\t.word __jfalse\n");
	printf("\t.word L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\t.word __jtrue\n");
	printf("\t.word L%d%s\n", n, tail);
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

void gen_helpcall(struct node *n)
{
	printf("\t.word __");
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
		/* Word endianness of target */
		if (big_endian) {
			printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
			printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
		} else {
			printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
			printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		}
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	if (cpu == 1802) {
		frame_off = 4;	/* Two words - old FP and old BPC */
		has_fp = 1;	/* Offsets are FP relative */
		big_endian = 0;	/* Little endian */
	} else {
		fprintf(stderr, "threadcode: unknown CPU\n");
		exit(1);
	}
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

static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

unsigned gen_push(struct node *n)
{
	/* Our push will put the object on the stack, so account for it */
	helper(n, "push");
	sp += get_stack_size(n->type);
	return 1;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		gen_helpcall(NULL);
		printf("cleanup\n");
		gen_value(UINT, n->right->value);
		sp -= n->right->value;
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
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		r->flags |= (n->flags & NORETURN);
		codegen_lr(r);
		return 1;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned v = n->value;
	unsigned nr = n->flags & NORETURN;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);

	switch(n->op) {
	case T_NREF:
		helper(n, "nref");
		printf("\t.word _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LBREF:
		helper(n, "lbref");
		printf("\t.word T%ds+%d\n", n->val2, v);
		return 1;
	case T_LREF:
		if (nr)
			return 1;
		helper(n, "lref");
		printf("\t.word %d\n", v);
		return 1;
	case T_NSTORE:
		helper(n, "nstore");
		printf("\t.word _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LBSTORE:
		helper(n, "lbstore");
		printf("\t.word T%ds+%d\n", n->val2, v);
		return 1;
	case T_LSTORE:
		if (nr)
			return 1;
		helper(n, "lstore");
		printf("\t.word %d\n", v);
		return 1;
	case T_CALLNAME:
		helper(n, "callfn");
		printf("\t.word _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_ARGUMENT:
		/* Turn argument into local effectively */
		helper(n, "loadl");
		if (!has_fp)
			v += sp;
		printf("\t.word %d\n", v + frame_len);
		return 1;
	case T_LOCAL:
		/* Adjust offsets of convenience */
		helper(n, "loadl");
		if (!has_fp)
			v += sp;
		printf("\t.word %d\n", v);;
		return 1;
	case T_FUNCCALL:
		/* Can't use helper() directly as type is return type */
		printf("\t.word __callfunc\n");
		return 1;
	}
	return 0;
}
