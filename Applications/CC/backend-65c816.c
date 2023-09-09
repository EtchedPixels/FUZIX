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

/*
 *	Our chance to do tree rewriting. We don't do much for the 65C816
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *r = n->right;
	struct node *l = n->left;
	unsigned s = get_size(n->type);
	/* Rewrite references into a load or store operation */
	if (s <= 2) {
		if (n->op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += 4 + frame_len;
				if (r->value < 254)
					squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
		}
		if (n->op == T_EQ) {
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += 4 + frame_len;
				if (l ->value < 254) {								
					squash_left(n, T_LSTORE);
				}
				return n;
			}
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
		}
		/* We can turn some operators into simpler nodes for
		   code generation */
		if (n->op == T_PLUSPLUS || n->op == T_MINUSMINUS) {
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
	if (n->op == T_FUNCCALL && r->op == T_NAME) {
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
/* We point the direct page at the bottom of our frame so we can use direct
   page for locals and arguments most of the time. The CPU has a load/store
   via S but has a lot of operations via DP and DP is more compact so this is
   a win. Look at helper options for this lot */
void gen_frame(unsigned size)
{
	frame_len = size;
	printf("\tphd\n");
	sp = 0;	/* Stack is relative to bottom of frame */
	if (size > 12) { 
		printf("\ttsx\n\ttxa\n\tsec\tsbc #%d\n", -size);
		printf("\ttcd\n\ttax\n\ttxs\n");
	} else {
		while(size >= 2) {
			printf("\tphx\n");
			size -= 2;
		}
		if (size == 1)
			printf("\tdes\n");
		printf("\ttsx\n\ttxa\n\ttcd\n");
	}
}

void gen_epilogue(unsigned size)
{
	if (sp) {
		fprintf(stderr, "sp out by %d\n", sp);
		error("sp");
	}
	if (size > 12) { 
		printf("\ttsx\n\ttxa\n\tsec\tsbc #%d\n", -size);
		printf("\tpld\n");
	} else {
		while(size >= 2) {
			printf("\tplx\n");
			size -= 2;
		}
		if (size == 1)
			printf("\tins\n");
		printf("\tpld\n");
	}
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
	printf("\tjeq L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjne L%d%s\n", n, tail);
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

/* TODO check values */
static void mode8bit(void)
{
	printf("\trep 0x20\n");
}

static void mode16bit(void)
{
	printf("\tsep 0x20\n");
}

unsigned gen_push(struct node *n)
{
	unsigned s = get_stack_size(n->type);
	/* Our push will put the object on the stack, so account for it */
	sp += s;
	/* We work in words so we don't have to sep/rep a lot */
	if (s == 1 || s == 2)
		printf("\tpha\n");
	else if (s == 4) {
		printf("\tphy\n");
		printf("\tpha\n");
	} else
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

unsigned gen_constop(const char *op, struct node *n)
{
	unsigned s;
	unsigned v = n->value;
	s = get_size(n->type);
	if (s > 2)
		return 0;

	if (s == 1)
		mode8bit();
	/* Objects we know how to directly access */
	switch(n->op) {
	case T_CONSTANT:
		printf("\t%s #%d\n", op, v);
		break;
	case T_NAME:
		printf("\t%s #%s+%d\n", op, namestr(n->snum), v);
		break;
	case T_LABEL:
		printf("\t%s #T%d\n\n", op, v);
		break;
	case T_NREF:
		printf("\t%s %s + %d\n", op, namestr(n->snum), v);
		break;
	case T_LREF:
		if (v + sp > 255)
			return 0;
		printf("\t%s z:%d\n", op, v + sp);
		break;
	default:
		return 0;
	}
	if (s == 1)
		mode16bit();
	return 1;
}

unsigned gen_derefop(const char *op, struct node *n)
{
	unsigned s;
	unsigned v = n->value;

	s = get_size(n->type);
	if (s > 2)
		return 0;

	if (s == 1)
		mode8bit();
	switch(n->op) {
	case T_LOCAL:
		printf("\t%s z:%d\n", op, v + sp);
		break;
	case T_ARGUMENT:
		printf("\t%s z:%d\n", op, v + frame_len + sp);
		break;
	default:
		return 0;
	}
	if (s == 1)
		mode16bit();
	return 1;
}

unsigned can_constop(struct node *n)
{
	if (get_size(n->type) > 2)
		return 0;
	if (n->op == T_LREF && n->value + sp > 255)
		return 0;
	if (n->op == T_CONSTANT || n->op == T_NAME || n->op == T_LABEL ||
		n->op == T_NREF || n->op == T_LREF /* || n->op == T_LOCAL ||
		n->op == T_ARGUMENT*/)
		return 1;
	return 0;
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
	case UCHAR:
		printf("\tand #0xFF\n");
		return 1;
	case CCHAR:
		/* For now */
		return 0;
	}
	if (dtype == CLONG || dtype == ULONG) {
		if (stype & UNSIGNED)
			printf("\tldy #0\n");
		else
			printf("\tjsr ___sexl\n");
		return 1;
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
	int v2 = n->val2;

	switch(n->op) {
	case T_MINUSMINUS:
		v2 = -v2;
	case T_PLUSPLUS:
		/* TODO optimise using inc dec on A */
		if (!gen_derefop("lda", r))
			break;
		printf("\tclc\n\tadc #%d\n", v2);
		gen_derefop("sta", r);
		if (!(n->flags & NORETURN))
			printf("\tsec\n\tsbc #%d\n", v2);
		return 1;
	case T_TILDE:
		if (!gen_constop("lda", r))
			break;
		printf("\txor #0xFFFF\n");
		return 1;
	case T_NEGATE:
		if (!gen_constop("ld", r))
			break;
		printf("\txor #0xFFFF\n");
		printf("\tclc\n");
		printf("\tadc #1\n");
		return 1;
	}
	return 0;
}

unsigned gen_compare(struct node *n, const char *p)
{
	unsigned v = n->right->value;
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 1)
		mode8bit();
	printf("\tsec\n");
	printf("\tsbc #%d\n", v & 0xFFFF);
	if (s == 1)
		mode16bit();
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

	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	if (n->op == T_CLEANUP) {
		v = r->value;
		if (v) {
			/* FIXME .. mod stack helper ?*/
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

	switch(n->op) {
	case T_NSTORE:
		if (s <= 2) {
			if (s == 1)
				mode8bit();
			printf("\tlda #%d\n", v);
			printf("\tsta _%s+%d\n", namestr(n->snum), nv);
			if (s == 1)
				mode16bit();
			return 1;
		}
		if (s == 4) {
			printf("\tlda #%d\n", v & 0xFFFF);
			printf("\tldy #%d\n", (v >> 16));
			printf("\tsta _%s+%d\n", namestr(n->snum), nv);
			/* FIXME: check valid sty mode */
			printf("\tsty _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		/* FIXME: dp or ,s by range - or maybe limit the rewrite
		   accordingly ? */
		if (s == 4) {
			printf("\tlda #%d\n", v & 0xFFFF);
			printf("\tldy #%d\n", (v >> 16));
			printf("\tsta z:%d\n", nv);
			printf("\tsty z:%d\n", nv + 2);
			return 1;
		}
		if (s == 1 || s == 2) {
			if (s == 1)
				mode8bit();
				
			printf("\tlda #%d\n", v);
			printf("\tsta z:%d\n", nv);
			if (s == 1)
				mode16bit();
			return 1;
		}
		break;
	case T_EQ:
		if (!constval(r))
			return 0;
		printf("\tax\n");
		switch(s) {
		case 1:
			mode8bit();
		case 2:
			printf("\tlda #%d\n", v);
			printf("\tsta f:0,x\n");
			if (s == 1)
				mode16bit();
			return 1;
		case 4:
			printf("\tldy #%d\n", (v >> 16));
			printf("\tsty f:0,x\n");
			printf("\tlda #%d\n", v & 0xFFFF);
			printf("\tsta f:2,x\n");
			return 1;
		default:
			return 0;
		}
		break;
	case T_PLUS:
		return gen_constop("clc\n\tadc", r);
	case T_MINUS:
		return gen_constop("sec\n\tsbc", r);
	case T_AND:
		return gen_constop("and", r);
	case T_OR:
		return gen_constop("or", r);
	case T_HAT:
		return gen_constop("eor", r);
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
static unsigned gen_xeqop(struct node *n, const char *cop)
{
	struct node *l = n->left;
	unsigned s = get_size(n->type);

	if (s > 2 || !can_constop(l))
		return 0;

	/* Get the value on the right into D */
	codegen_lr(n->right);

	/* Was an lval so we want the type it referenced not the pointer */
	l->type--;

	gen_constop(cop, l);
	gen_constop("sta", l);
	return 1;
}

/* Helpers for comparisons we can do cleanly. We might have the good stuff
   on either side so we have to pick the right compare operator according
   to our direction */
static unsigned gen_compop(struct node *n, const char *lo, const char *ro)
{
	/* TODO: compare operator instead ? */
	if (can_constop(n->left)) {
		codegen_lr(n->right);
		printf("\tsec\n");
		gen_constop("sbc", n->left);
		printf("\tjsr bool%s\n", ro);
		/* Tell the core backend that it doesn't need to bool this */
		n->flags |= ISBOOL;
		return 1;
	}
	if (can_constop(n->right)) {
		codegen_lr(n->left);
		printf("\tsec\n");
		gen_constop("sbc", n->right);
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
			gen_constop("add", l);
			return 1;
		}
		break;
	case T_PLUSEQ:
		return gen_xeqop(n, "add");
	case T_ANDEQ:
		return gen_xeqop(n, "and");
	case T_OREQ:
		return gen_xeqop(n, "ora");
	case T_HATEQ:
		return gen_xeqop(n, "eor");
	case T_EQ:
		if (!can_constop(l))
			return 0;
		codegen_lr(n->right);
		/* Was an lval so we want the referenced type */
		l->type--;
		gen_constop("st", l);
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

static unsigned gen_pop_and_op(const char *op, struct node *n)
{
	if (get_size(n->type) > 2)
		return 0;
	/* need to optimize the X usage */
	printf("\ttsx\n");
	printf("\t%s f:0,x\n", op);
	printf("\tply\n");	/* 16 or 8bit op so y is sacrificial */
	return 1;
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
			mode8bit();
			printf("\tlda _%s+%d\n", namestr(n->snum), nv);
			mode16bit();
			return 1;
		}
		if (s == 2) {
			printf("\tlda _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 4) {
			printf("\tldy _%s+%d\n", namestr(n->snum), nv);
			printf("\tlda _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LREF:
		if (s == 4) {
			printf("\tlda z:%d\n", nv);
			printf("\tldy z:%d\n", nv + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tlda z:%d\n", nv);
			return 1;
		}
		if (s == 1) {
			mode8bit();
			printf("\tlda z:%d\n", nv);
			mode16bit();
			return 1;
		}
		break;
	case T_NSTORE:
		if (s == 1) {
			mode8bit();
			printf("\tsta _%s+%d\n", namestr(n->snum), nv);
			mode16bit();
			return 1;
		}
		if (s == 2) {
			printf("\tsta _%s+%d\n", namestr(n->snum), nv);
			return 1;
		}
		if (s == 4) {
			printf("\tsta _%s+%d\n", namestr(n->snum), nv);
			printf("\tsty _%s+%d\n", namestr(n->snum), nv + 2);
			return 1;
		}
		break;
	case T_LSTORE:
		if (s == 4) {
			printf("\tsta z:%d\n", nv);
			printf("\tsty z:%d\n", nv + 2);
			return 1;
		}
		if (s == 2) {
			printf("\tsta z:%d\n", nv);
			return 1;
		}
		if (s == 1) {
			mode8bit();
			printf("\tsta z:%d\n", nv);
			mode16bit();
			return 1;
		}
		break;
	case T_CALLNAME:	/* Call function by name */
		printf("\tjsr _%s+%d\n", namestr(n->snum), nv);
		return 1;
	case T_EQ:
		printf("\tphx\n");
		if (s == 1) {
			mode8bit();
			printf("\tsta f:0,x\n");
			mode16bit();
		}
		else if (s == 2)
			printf("\tsta f:0,x\n");
		else if (s == 4) {
			printf("\tsta f:0,x\n");
			printf("\tsty f:2,x\n");
		} else
			return 0;
		return 1;
	case T_DEREF:
		if (s <= 4) {
			printf("\ttax\n");
			if (s == 4) {
				printf("\tlda f:0,x\n");
				printf("\tldy f:2,x\n");
			}
			if (s == 2)
				printf("\tlda f:0,x\n");
			else {
				mode8bit();
				printf("\tlda f:0,x\n");
				mode16bit();
			}
			return 1;
		}
		return 0;
	case T_LABEL:
		printf("\tlda #T%d\n", nv);
		return 1;
	case T_CONSTANT:
		switch(s) {
		case 1:
			/* Cheaper to do the extra byte load than rep/sep */
		case 2:
			printf("\tlda #%d\n", nv);
			return 1;
		case 4:
			printf("\tldy #%d\n", (unsigned)((n->value >> 16) & 0xFFFF));
			printf("\tlda #%d\n", nv);
			return 1;
		}
		break;
	case T_NAME:
		printf("lda #%s+%d\n", namestr(n->snum), nv);
		return 1;
	case T_LOCAL:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", nv + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	case T_ARGUMENT:
		/* FIXME: correct offsets */
		printf("\tleax %d,s\n", nv + frame_len + 4 + sp);
		/* Will need a lot of peepholing */
		printf("\ttfr x,d\n");
		return 1;
	/* Not as nice as 6809 as we have to set up x, go via ,x and pop */
	case T_PLUS:
		return gen_pop_and_op("clc\n\tadc", n);
	case T_MINUS:
		return gen_pop_and_op("sec\n\tsbc", n);
	case T_AND:
		return gen_pop_and_op("and", n);
	case T_OR:
		return gen_pop_and_op("ora", n);
	case T_HAT:
		return gen_pop_and_op("eor", n);
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
