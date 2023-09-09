#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

/*
 *	We can push bytes so everything is native sized
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
	return get_size(t);
}

/*
 *	Private types we rewrite things into
 */
#define T_CALLNAME	(T_USER)
#define T_NREF		(T_USER+1)
#define T_NSTORE	(T_USER+2)

#define T_LREF		(T_USER+3)
#define T_LSTORE	(T_USER+4)

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
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

	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME)
		return 10;
	if (op == T_NREF || op == T_LREF)
		return 5;
	return 0;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 6809
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *r = n->right;
	struct node *l = n->left;
	unsigned s = get_size(n->type);
	unsigned op = n->op;
	/* Rewrite references into a load or store operation */
	if (s <= 2) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += 2 + frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
		}
		if (op == T_EQ) {
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += 2 + frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
		}
		/* We can turn some operators into simpler nodes for
		   code generation */
		if (op == T_PLUSPLUS || op == T_MINUSMINUS) {
			/* right is a constant scaled value, left is
			   the lval */
			n->val2 = r->value;
			free_node(r);
			/* We must keep a right when we convert */
			n->right = l;
			n->left = NULL;
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (op == T_FUNCCALL && r->op == T_NAME) {
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
		printf("\t.code\n");
		break;
	case A_DATA:
	case A_LITERAL:
		printf("\t.data\n");
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
void gen_frame(unsigned size)
{
	frame_len = size;
	if (size) {
		sp = 0;	/* Stack is relative to bottom of frame */
		printf("\tleas -%d,s\n", size);
	}
}

void gen_epilogue(unsigned size)
{
	if (sp) {
		fprintf(stderr, "sp out by %d\n", sp);
		error("sp");
	}
	if (size)
		printf("\tleas %d,s\n", size);
	printf("\trts\n");
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

void gen_switch(unsigned n, unsigned type)
{
	printf("\tldx #Sw%d\n", n);
	printf("\tjsr __switch");
	helper_type(type, 0);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
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

/* Output whatever goes in front of a helper call */
void gen_helpcall(struct node *n)
{
	printf("\tjsr ");
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
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
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
	unsigned s = get_stack_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += s;
	if (s == 1)
		printf("\tpshs b\n");
	else if (s == 2)
		printf("\tpshs d\n");
	else if (s == 4)	/* TODO check this orders right if
				   we use it for auto inits */
		printf("\tpshs d,u\n");
	else
		return 0;
	return 1;
}

unsigned constval(struct node *n)
{
	if (n->op == T_CONSTANT || n->op == T_LABEL || n->op == T_NAME)
		return 1;
	return 0;
}

/* Turn a constant node into a string - for char/int/ptr */

unsigned gen_constop(const char *op, char r, struct node *n)
{
	unsigned s;
	unsigned v = n->value;
	s = get_size(n->type);
	if (s > 2)
		return 0;
	if (r == 0) {
		r = 'd';
		if (s == 1)
			r = 'b';
	}

	/* Objects we know how to directly access */
	switch(n->op) {
	case T_CONSTANT:
		printf("\t%s%c #%d\n", op, r, v);
		return 1;
	case T_NAME:
		printf("\t%s%c #%s+%d\n", op, r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\t%s%c #T%d\n\n", op, r, v);
		return 1;
	case T_NREF:
		printf("\t%s%c %s + %d\n", op, r, namestr(n->snum), v);
		return 1;
	case T_LREF:
		printf("\t%s%c %d,s\n", op, r, v + sp);
		return 1;
	}
	return 0;
}

unsigned gen_derefop(const char *op, struct node *n)
{
	unsigned s = get_size(n->type);
	unsigned v = n->value;
	char r = 'd';

	if (s > 2)
		return 0;

	if (s == 1)
		r = 'b';

	switch(n->op) {
	case T_LOCAL:
		printf("\t%s%c %d,s\n", op, r, v + sp);
		return 1;
	case T_ARGUMENT:
		printf("\t%s%c %d,s\n", op, r, v + frame_len + sp);
		return 1;
	}
	return 0;
}

unsigned can_constop(struct node *n)
{
	if (get_size(n->type) > 2)
		return 0;
	if (n->op == T_CONSTANT || n->op == T_NAME || n->op == T_LABEL ||
		n->op == T_NREF || n->op == T_LREF)
		return 1;
	return 0;
}

unsigned gen_constpair(const char *op, struct node *n)
{
	unsigned s;
	unsigned v;
	s = get_size(n->type);
	if (s == 1)
		return gen_constop(op, 0, n);

	v = n->value;

	/* Generate pair forms */
	switch(n->op) {
	case T_CONSTANT:
		printf("\t%sa #%d\n", op, (v >> 8) & 0xFF);
		printf("\t%sb #%d\n", op, (v & 0xFF));
		return 1;
	case T_NAME:
		printf("\t%sa >#%s+%d\n", op, namestr(n->snum), v);
		printf("\t%sb <#%s+%d\n", op, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\t%sa >#T%d\n\n", op, v);
		printf("\t%sa <#T%d\n\n", op, v);
		return 1;
	case T_NREF:
		printf("\t%sa %s + %d\n", op, namestr(n->snum), v);
		printf("\t%sb %s + %d\n", op, namestr(n->snum), v + 1);
		return 1;
	case T_LREF:
		printf("\t%sa %d,s\n", op, v);
		printf("\t%sb %d,s\n", op, v + 1);
		return 1;
	}
	return 0;
}

unsigned gen_pair(const char *op, struct node *n)
{
	unsigned s = get_size(n->type);
	/* For now */
	if (s > 2)
		return 0;
	printf("\t%sb\n", op);
	if (s == 2)
		printf("\t%sa\n", op);
	return 1;
}

unsigned gen_pair_pop(const char *op, struct node *n)
{
	unsigned s = get_size(n->type);
	/* For now */
	if (s > 2)
		return 0;
	if (s == 2)
		printf("\t%sa (s+)\n", op);
	printf("\t%sb (s+)\n", op);
	return 1;
}

/* Just deal with integer types for now */
unsigned gen_cast(struct node *n, struct node *r)
{
	unsigned stype = r->type;
	unsigned dtype = n->type;
	if (PTR(stype))
		stype = CSHORT;
	if (PTR(dtype))
		dtype = CSHORT;
	if (!IS_INTARITH(stype) || !IS_INTARITH(dtype))
		return 0;
	/* Going to a smaller type is free */
	if ((stype & ~UNSIGNED) >= (dtype & ~UNSIGNED))
		return 1;
	switch(stype) {
	case CCHAR:
		printf("\tsex\n");
		break;
	case UCHAR:
		printf("\tclra\n");
		break;
	}
	if (dtype == CLONG || dtype == ULONG) {
		if (stype & UNSIGNED)
			printf("\tldu #0\n");
		else
			printf("\tjsr ___sexl\n");
	}
	return 1;
}

/*
 *	Allow the code generator to shortcut the generation of the argument
 *	of a single argument operator (for example to shortcut constant cases
 *	or simple name loads that can be done better directly)
 */
unsigned gen_uni_direct(struct node *n)
{
	struct node *r = n->right;
	char reg = 'd';
	int v2 = n->val2;
	unsigned s = get_size(n->type);

	if (s == 1)
		reg = 'b';

	switch(n->op) {
	case T_MINUSMINUS:
		v2 = -v2;
	case T_PLUSPLUS:
		if (!gen_derefop("ld", r))
			break;
		printf("\tadd%c #%d\n", reg, v2);
		gen_derefop("st", r);
		if (!(n->flags & NORETURN))
			printf("\tsub%c #%d\n", reg, v2);
		return 1;
	case T_TILDE:
		if (!gen_constop("ld", 0, r))
			break;
		return gen_pair("com", r);
	case T_NEGATE:
		if (!gen_constop("ld", 0, r))
			break;
		if (s == 1)
			return gen_pair("neg", r);
		if (gen_constpair("com", r) == 0)
			return 0;
		printf("\taddd #1\n");
		return 1;
	}
	return 0;
}

unsigned gen_compare(struct node *n, const char *p)
{
	unsigned v = n->right->value;
	unsigned s = get_size(n->type);
	if (s == 1)
		printf("\tcmpb #%d\n", v & 0xFF);
	else if (s == 2)
		printf("\tcmpd #%d\n", v & 0xFFFF);
	else
		return 0;
	/* Sets the bool and flags */
	printf("jsr %s\n", p);
	/* Tell the core backend that it doesn't need to bool this */
	n->flags |= ISBOOL;
	return 1;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	unsigned v, nv = n->value;
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	char *ldop = "ldd";
	char *stop = "std";

	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	if (n->op == T_CLEANUP) {
		v = r->value;
		if (v) {
			printf("\tleas %d,s\n", v);
			sp -= v;
		}
		return 1;
	}
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
	}

	if (r == NULL)
		return 0;
	v = r->value;
	if (s == 1) {
		ldop = "ldb";
		stop = "stb";
	}

	switch(n->op) {
	case T_NSTORE:
		if (s == 1 || s == 2) {
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s _%s+%d\n", stop, namestr(n->snum), nv);
			return 1;
		}
		if (s == 4) {
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tldu #%d\n", (v >> 16));
			printf("\tstu _%s+%d\n", namestr(n->snum), nv);
			printf("\tstd _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		if (s == 4) {
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tldu #%d\n", (v >> 16));
			printf("\tldu %d,s\n", nv);
			printf("\tldd %d,s\n", nv + 2);
			return 1;
		}
		if (s == 1 || s == 2) {
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s %d,s\n", stop, nv);
			return 1;
		}
		break;
	case T_EQ:
		if (!constval(r))
			return 0;
		printf("\ttfr d,x\n");
		switch(s) {
		case 1:
			if (v == 0) {
				printf("\tclr ,x\n");
				return 1;
			}
		case 2:
			printf("\t%s #%d\n", ldop, v);
			printf("\t%s ,x\n", stop);
			return 1;
		case 4:
			printf("\tldu #%d\n", (v >> 16));
			printf("\tstu ,x\n");
			printf("\tldd #%d\n", v & 0xFFFF);
			printf("\tstd 2,x\n");
			return 1;
		default:
			return 0;
		}
		break;
	case T_PLUS:
		return gen_constop("add", 0, r);
	case T_MINUS:
		return gen_constop("sub", 0, r);
	/* There are optimizations to consider here TODO
		and 0 = clr
		and 255 =
		or 0 =
		or 255 = 255
		eor 0 = */
	case T_AND:
		return gen_constpair("and", r);
	case T_OR:
		return gen_constpair("or", r);
	case T_HAT:
		return gen_constpair("eor", r);
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
	}
	return 0;
}

/* Generate helpers for commutative x= operations on single register objects */
static unsigned gen_xeqop(struct node *n, const char *cop, unsigned has16)
{
	struct node *l = n->left;
	unsigned s = get_size(n->type);

	if (s > 2 || !can_constop(l))
		return 0;

	/* Get the value on the right into D */
	codegen_lr(n->right);

	/* Was an lval so we want the type it referenced not the pointer */
	l->type--;

	if (has16)
		gen_constop(cop, 0, l);
	else
		gen_constpair(cop, l);
	gen_constop("st", 0, l);
	return 1;
}

/* Helpers for comparisons we can do cleanly. We might have the good stuff
   on either side so we have to pick the right compare operator according
   to our direction */
static unsigned gen_compop(struct node *n, const char *lo, const char *ro)
{
	if (can_constop(n->left)) {
		codegen_lr(n->right);
		gen_constop("subd", 0, n->left);
		printf("\tjsr bool%s\n", ro);
		/* Tell the core backend that it doesn't need to bool this */
		n->flags |= ISBOOL;
		return 1;
	}
	if (can_constop(n->right)) {
		codegen_lr(n->left);
		gen_constop("subd", 0, n->right);
		printf("\tjsr bool%s\n", lo);
		/* Tell the core backend that it doesn't need to bool this */
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	switch(n->op) {
	case T_PLUS:
		if (can_constop(l)) {
			codegen_lr(n->right);
			gen_constop("add", 0, n->left);
			return 1;
		}
		break;
	case T_PLUSEQ:
		return gen_xeqop(n, "add", 1);
	case T_ANDEQ:
		return gen_xeqop(n, "and", 0);
	case T_OREQ:
		return gen_xeqop(n, "or", 0);
	case T_HATEQ:
		return gen_xeqop(n, "eor", 0);
	case T_EQ:
		if (!can_constop(l))
			return 0;
		codegen_lr(n->right);
		/* Was an lval so we want the referenced type */
		l->type--;
		gen_constop("st", 0, l);
		return 1;
	/* Some casts are easy.. */
	case T_CAST:
		codegen_lr(n->right);
		if (gen_cast(n, n->right))
			return 1;
		/* We generated the subtree, so finish the cast off ourselves too */
		helper(n, "cast");
		return 1;
	/* Some compares */
	case T_EQEQ:
		return gen_compop(n, "eq", "eq");
	case T_BANGEQ:
		return gen_compop(n, "ne", "ne");
	case T_LT:
		return gen_compop(n, "lt", "ge");
	case T_LTEQ:
		return gen_compop(n, "le", "gt");
	case T_GT:
		return gen_compop(n, "gt", "le");
	case T_GTEQ:
		return gen_compop(n, "ge", "lt");
	/* And the mul/div/mod ones need helpers anyway */
	default:
		return 0;
	}
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned s;
	unsigned nv = n->value;
	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);

	s = get_size(n->type);

	switch(n->op) {
	case T_NREF:	/* Get the value held in a name */
		if (s == 1) {
			printf("\tldb _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 2) {
			printf("\tldd _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 4) {
			printf("\tldu _%s+%d\n", namestr(n->snum), nv);
			printf("\tldd _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LREF:
		if (s == 4) {
			printf("\tldu %d,s\n", nv);
			printf("\tldd %d,s\n", nv + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tldd %d,s\n", nv);
			return 1;
		}
		if (s == 1) {
			printf("\tldb %d,s\n", nv);
			return 1;
		}
		break;
	case T_NSTORE:
		if (s == 1) {
			printf("\tstb _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 2) {
			printf("\tstd _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 4) {
			printf("\tstu _%s+%d\n", namestr(n->snum), nv);
			printf("\tstd _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		if (s == 4) {
			printf("\tldu %d,s\n", nv);
			printf("\tldd %d,s\n", nv + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tstd %d,s\n", nv);
			return 1;
		}
		if (s == 1) {
			printf("\tstb %d,s\n", nv);
			return 1;
		}
		break;
	case T_CALLNAME:	/* Call function by name */
		printf("\tjsr _%s+%d\n", namestr(n->snum), nv);
		return 1;
	case T_EQ:
		printf("\tldx (-s)\n");
		if (s == 1)
			printf("\tstb ,x\n");
		else if (s == 2)
			printf("\tstd ,x\n");
		else if (s == 4) {
			printf("\tstd 2,x\n");
			printf("\tstu ,x\n");
		} else
			return 0;
		return 1;
	case T_DEREF:
		if (s <= 4) {
			printf("\ttfr d,x\n");
			if (s == 4) {
				printf("\tldu ,x\n");
				printf("\tldd 2,x\n");
			}
			if (s == 2)
				printf("\tldd ,x\n");
			else
				printf("\tldb ,x\n");
			return 1;
		}
		return 0;
	case T_LABEL:
		printf("\tldd #T%d\n", nv);
		return 1;
	case T_CONSTANT:
		switch(s) {
		case 1:
			printf("\tldb #%d\n", nv);
			return 1;
		case 4:
			printf("\tldu #%d\n", (unsigned)((n->value >> 16) & 0xFFFF));
		case 2:
			printf("\tldd #%d\n", nv & 0xFFFF);
			return 1;
		}
		break;
	case T_NAME:
		printf("ldd #%s+%d\n", namestr(n->snum), nv);
		return 1;
	case T_LOCAL:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", nv + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	case T_ARGUMENT:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", nv + frame_len + 2 + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	case T_PLUS:
		if (s == 1)
			printf("\taddb ,s+\n");
		if (s == 2)
			printf("\taddd ,s++\n");
		else
			return 0;
		return 1;
	case T_AND:
		return gen_pair_pop("and", n);
	case T_OR:
		return gen_pair_pop("or", n);
	case T_HAT:
		return gen_pair_pop("eor", n);
	/* We rewrote these into a new form so must handle them. We can also
	   do better probably TODO .. */
	case T_PLUSPLUS:
		helper(n, "postinc");
		printf("\t.word %d\n", n->val2);
		return 1;
	case T_MINUSMINUS:
		helper(n, "postdec");
		printf("\t.word -%d\n", n->val2);
		return 1;
	/* Some casts are easy.. */
	case T_CAST:
		return gen_cast(n, n->right);
	}
	return 0;
}
