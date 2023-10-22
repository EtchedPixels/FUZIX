/*
 *	DG Nova 3 or 4
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */
static unsigned argbase;	/* Argument offset in current function */
static unsigned unreachable;	/* Code following an unconditional jump */
static unsigned func_cleanup;	/* Zero if we can just ret out */

/* Take care - this is a byte size but on a word machine */
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
 *	Heuristic for guessing what to put on the right. Pretty much
 *	anything in our case
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
void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size, unsigned aframe)
{
	frame_len = size;
	sp = 0;

	if (size == 0)
		printf("\tsav\n");
	else
		printf("\tsavn\n\t.word %d\n", size);

	/* TODO: register variables */
	func_cleanup = 1;
}

void gen_epilogue(unsigned size, unsigned argsize)
{
	if (sp != 0)
		error("sp");
	/* Skip a word on return (the call addr */
	printf("\tinc 5,3\n");
	/* Store rhe return value back into the frame for caller */
	printf("\tsta 0,1,3\n");
	/* TODO : registers, decide on 32bit return */	   
	sp -= size;
	if (size == 0)
		printf("\tret\n");
	else
		printf("\tretn\n\t.word %d\n", size);
		
}

void gen_label(const char *tail, unsigned n)
{
	unreachable = 0;
	printf("L%d%s:\n", n, tail);
}

/* A return statement. We can sometimes shortcut this if we have
   no cleanup to do */
unsigned gen_exit(const char *tail, unsigned n)
{
	gen_jump(tail, n);
	return 0;
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjmp .+1\n\t.word L%d%s\n", n, tail);
	unreachable = 1;
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tmov# 0,0,snr");
	/* The tool chain will fix this up with a helper if needed */
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tmov# 0,0,szr");
	/* The tool chain will fix this up with a helper if needed */
	printf("\tjmp L%d%s\n", n, tail);
}

static void gen_cleanup(unsigned v)
{
	if (v == 0)
		return;
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v < 4) {
		while(v--)
			printf("\tpop 1\n");
	} else {
		printf("\tmfsp 1\n");
		printf("\tadd 1,.+1,skp\n");
		printf("\t.word %d\n", -v);
		printf("\tmtsp 1\n");
	}
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */
void gen_helpcall(struct node *n)
{
	printf("\tjsr .+1\n\t.word __");
}

void gen_helpclean(struct node *n)
{
}

void gen_switch(unsigned n, unsigned type)
{
	/* TODO */
	printf("\tlxi d,Sw%d\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	unreachable = 0;
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
	/* FIXME: need to deal with packing and the like here and
	   in the core */
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
 */
static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	/* TODO group the user ones together for a range check ? */
	/* TODO - we can do locals for some forms */
	if (op != T_CONSTANT && op != T_NAME && op != T_LABEL &&
		 op != T_NREF && op != T_LBREF && op != T_RREF)
		 return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

static unsigned access_direct_b(struct node *n)
{
	/* We can't access as much via B directly because we've got no easy xchg with b */
	/* TODO: if we want BC we need to know if BC currently holds the reg var */
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_REG)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 *
 *	TODO: figure out which forms want byte ptrs and the impact
 *	Think we need to double address refs but not offset ?
 */
static unsigned load_r_with(unsigned r, struct node *n, unsigned b)
{
	unsigned v = WORD(n->value);
	const char *name;

	if (b)
		v &= 0xFF;

	switch(n->op) {
	case T_NAME:
		/* TODO: literal packing */
		printf("\tsub# 0,0,skip\n");
		printf("\t.word _%s+%d\n", namestr(n->snum), v);
		printf("\tlda %d,.-1\n"), r);
		return 1;
	case T_LABEL:
		/* TODO: literal packing */
		printf("\tsub# 0,0,skip\n");
		printf("\t.wordT%d+%d\n", n->val2, v);
		printf("\tlda %d,.-1\n"), r);
		return 1;
	case T_CONSTANT:
		/* Look for quick forms too */
		load_constant(r, v);
		return 1;
	case T_NREF:
		printf("\tsub# 0,0,skip\n");
		printf("\t.word _%s+%d\n", namestr(n->snum), v);
		printf("\tlda 2,.-1\n"), r);
		if (b) {
			printf("\tlda 2,.-1\n");
			printf("\tldb  2,%d\n", r);
		} else
			printf("\tlda @2,.-1\n"), r);
		break;
	/* TODO: fold together cleanly with NREF */
	case T_LBREF:
		printf("\tsub# 0,0,skip\n");
		printf("\t.wordT%d+%d\n", n->val2, v);
		if (b == 0)
			printf("\tlda @%d,.-1\n"), r);
		else {
			printf("\tlda %d,0,%d\n", r, r);
			printf("\tldb %d,%d\n", r, r);
		}
		return 1;
	case T_LREF:
		if (b == 0) {
			printf("\tlda@ 0,%d,3\n", v + sp);
			return 1;
		}
		printf("\tlda 0,%d,3\n", v + sp);
		printf("\tldb 0, 0\n");
		return 1;
	case T_RREF:
		printf("\tlda %d,__reg%d\n", n->val2);
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

static void load0r(struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	printf("\tlda 0,__reg%d\n", n->val2);
	if (s == 1)
		printf("\tand 0,__ff\n");
}

static void saver(unsigned s)
{
	printf("\tsta 0,__reg%d\n", n->val2);
}

static unsigned gen_pairop(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (load_r_with(1, r, s == 1) == 0)
		return 0;
	if (sign)
		helper_s(n, op);
	else
		helper(n, op);
	return 1;
}

static unsigned gen_compc(const char *op, struct node *n, struct node *r, unsigned sign)
{
	if (gen_pairop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

/* TODO */
static int count_mul_cost(unsigned n)
{
	int cost = 0;
	if ((n & 0xFF) == 0) {
		n >>= 8;
		cost += 3;		/* mov mvi */
	}
	while(n > 1) {
		if (n & 1)
			cost += 3;	/* push pop dad d */
		n >>= 1;
		cost++;			/* dad h */
	}
	return cost;
}

/* Write the multiply for any value > 0 */
static void write_mul(unsigned n)
{
	unsigned pops = 0;
	if ((n & 0xFF) == 0) {
		printf("\tmov h,l\n\tmvi l,0\n");
		n >>= 8;
	}
	while(n > 1) {
		if (n & 1) {
			pops++;
			printf("\tpush 0\n");
		}
		printf("\tadd 0,0\n");
		n >>= 1;
	}
	while(pops--) {
		printf("\tpop 1\n\tadd 1,0\n");
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
		load_constant(0, 0);
	else
		write_mul(n);
}

static unsigned gen_fast_div(unsigned n, unsigned s)
{
	if (s != 2)
		return 0;
	if (n == 1)
		return 1;
	if (n == 256) {
		printf("\tmov l,h\n\tmvi h,0\n");
		return 1;
	}
	if (n & (n - 1))
		return 0;

	while(n > 1) {
		/* TODO */
		if (sign) {
			/* Load carry bit into C leave reg untouched */
			printf("\tmovl# 0,0\n");
			/* Now rotate right copying top bit via C */
			printf("\tmovr 0,0\n");
		} else
			printf("\tmovzr 0,0\n");
		n >>= 1;
	}
	return 1;
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
	case T_LBSTORE:
		if (s > 2)
			return 0;
		load_r_with(2, r);
		if (s == 1)
			printf("\tstb 0,2\n");
		else
			printf("\tsta 0,0,2\n");
		return 0;
	case T_RSTORE:
		if (s > 2)
			return 0;
		loadreg(n->val2, s);
		return 1;
	case T_EQ:
		/* The address is in 0 at this point */
		if (s == 2) {
			load_r_with(2, r, 2);
			printf("\tsta 0,0,2\n");
			return 1;
		}
		if (s == 1) {
			load_r_with(1, r, 1);
			printf("\tstb 0,1\n");
			return 1;
		}
		return 0;
	case T_PLUS:
		/* TODO: return result register from load_r_with,
		   usually one asked but if reg then not */
		load_r_with(1, r, s);
		printf("\tadd 1,0\n");
		return 1;
	case T_MINUS:
		load_r_with(1, r, s);
		printf("\tsub 1,0\n");
		return 1;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && can_fast_mul(s, r->value)) {
				 gen_fast_mul(s, r->value);
				return 1;
			}
		}
		return gen_pairop("mulde", n, r, 0);
	case T_SLASH:
		/* FIXME: 8085 ARHL is signed so this is wrong for large numbers ? */
		if (r->op == T_CONSTANT) {
			if (s <= 2 && gen_fast_div(s, r->value, (n->type & UNSIGNED)))
				return 1;
		}
		return gen_pairop("divde", n, r, 1);
	case T_PERCENT:
		return gen_pairop("remde", n, r, 1);
	case T_AND:
		load_r_with(1, r, s);
		printf("\tand 1,0\n");
		return 1;
	case T_OR:
		return gen_pairop("borde", n, r, 0);
	case T_HAT:
		return gen_pairop("bxorde", n, r, 0);
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
		if (s <= 2 && r->op == T_CONSTANT && r->value <= 4) {
			repeated_op("add 0,0", r->value);
			return 1;
		}
		return gen_pairop("shlde", n, r, 0);
	case T_GTGT:
		/* TODO: could optimize some right shift cases */
		return gen_pairop("shrde", n, r, 1);
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		if (!(n->flags & NORETURN))
			return 0;
	/* TODO: the byte versions are fun. */
	case T_PLUSEQ:
		/* Some opportunities to use ISZ ? */
		return gen_pairop("pluseqde", n, r, 0);
	case T_MINUSMINUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		return gen_pairop("minuseqde", n, r, 0);
	case T_ANDEQ:
		return gen_pairop("andeqde", n, r, 0);
	case T_OREQ:
		return gen_pairop("oreqde", n, r, 0);
	case T_HATEQ:
		return gen_pairop("xoreqde", n, r, 0);
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
	if (s != 2 || n->op != T_CONSTANT || v > 4 || v < 0)
		return 0;
	return 1;
}

static unsigned reg_incdec(unsigned s, int v)
{
	if (v > 0)
		repeated_op("inc 0,0", v);
	return 1;
}

static void reg_logic(struct node *n, unsigned s, unsigned op, const char *i)
{
	/* TODO : constant optimization */
	codegen_lr(n->right);
	helper(n, i);
	loadreg(n, s);
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
	/* We don't know if the result has set the condition flags
	 * until we generate the subtree. So generate the tree, then
	 * either do nice things or use the helper */
	if (n->op == T_BOOL) {
		codegen_lr(r);
		if (r->flags & ISBOOL)
			return 1;
		printf("\tmov# 0,0,srz\n\tsubzl 0,0\n");
		n->flags |= ISBOOL;
		return 1;
	}
	/* Re-order assignments we can do the simple way */
	if (n->op == T_NSTORE && s <= 2) {
		codegen_lr(r);
		/* Expression result is now in 0 */
		printf("\tsub# 0,0,skp\n");
		printf(".word _%s+%d\n", namestr(n->snum), WORD(n->value));
		printf("\tlda 2,.-1\n");
		if (s == 1)
			printf("\tstb 0,2\n");
		else
			printf("\tsta 0,0,2\n");
		return 1;
	}
	/* LBSTORE ? TODO */
	/* Locals are actual the easy one */
	if (n->op == T_LSTORE && s <= 2) {
		printf("\tsta 0,%d,3\n", n->value + sp);
		return 1;
	}
	/* Register targetted ops. These are totally different to the normal EQ ops because
	   we don't have an address we can push and then do a memory op */
	if (l && l->op == T_REG) {
		v = r->value;
		switch(n->op) {
		/* TODO review incdec and get the ret values right for it */
		case T_PLUSPLUS:
			if (reg_canincdec(r, s, v)) {
				loadreg(n, s);
				reg_incdec(s, v);
				return 1;
			}
			if (r->op == T_CONSTANT) {
				printf("\tlda 1,.+2\n");
				printf("\tadd 0, 1, skp\n");
				printf("\t.word %d\n", v);
				printf("\tsta 1,__reg%d\n", l->value);
				return 1;
			}
			printf("\tpush 0\n");
			codegen_lr(r);
			printf("\tsta 0,__reg%d\n", l->value);
			printf("\tpop 0\n");
			return 1;
		case T_PLUSEQ:
			if (reg_canincdec(r, s, v)) {
				reg_incdec(s, v);
				if (nr)
					return 1;
				loadreg(n, s);
			} else {
			if (r->op == T_CONSTANT) {
				printf("\tlda 1,.+2\n");
				printf("\tadd 1, 0, skp\n");
				printf("\t.word %d\n", v);
				printf("\tsta 0,__reg%d\n", l->value);
				return 1;
			}
			codegen_lr(r);
			printf("\tsta 0,__reg%d\n", l->value);
			return 1;
		case T_MINUSMINUS:
			if (reg_canincdec(r, s, -v)) {
				loadreg(n, s);
				reg_incdec(s, -v);
				return 1;
			}
			if (r->op == T_CONSTANT) {
				printf("\tlda 1,.+2\n");
				printf("\tadd 0, 1, skp\n");
				printf("\t.word %d\n", -v);
				printf("\tsta 1,__reg%d\n", l->value);
				return 1;
			}
			printf("\tpush 0\n");
			codegen_lr(r);
			printf("\tsta 0,__reg%d\n", l->value);
			printf("\tpop 0\n");
			return 1;
			/* If we don't care about the return they look the same so fall
			   through */
		case T_MINUSEQ:
			if (reg_canincdec(r, s, -v)) {
				reg_incdec(s, -v);
				if (nr)
					return 1;
				loadreg(n, s);
			} else {
			if (r->op == T_CONSTANT) {
				printf("\tlda 1,.+2\n");
				printf("\tadd 1, 0, skp\n");
				printf("\t.word %d\n", -v);
				printf("\tsta 0,__reg%d\n", l->value);
				return 1;
			}
			codegen_lr(r);
			printf("\tsta 0,__reg%d\n", l->value);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					loadreg(NULL, s);
					gen_fast_mul(s, v);
					storereg(s);
					return 1;
				}
			}
			codegen_lr(r);
			helper(n, "bcmul");
			return 1;
		case T_SLASHEQ:
			/* TODO: power of 2 constant divide maybe ? */
			codegen_lr(r);
			helper_s(n, "bcdiv");
			return 1;
		case T_PERCENTEQ:
			/* TODO: spot % 256 case */
			codegen_lr(r);
			helper(n, "bcrem");
			return 1;
		case T_SHLEQ:
			if (r->op == T_CONSTANT) {
				if (s == 1 && v >= 8) {
					printf("\tmvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				if (s == 1) {
					printf("\tmov a,c\n");
					repeated_op("add a", v);
					printf("\tmov c,a\n");
					loadhl(n, s);
					return 1;
				}
				/* 16 bit */
				if (v >= 16) {
					printf("\tlxi b,0\n");
					loadhl(n, s);
					return 1;
				}
				if (v == 8) {
					printf("\tmov b,c\n\tmvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				if (v > 8) {
					printf("\tmov a,c\n");
					repeated_op("add a", v - 8);
					printf("\tmov b,a\nvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				/* 16bit full shifting */
				loadhl(NULL, s);
				repeated_op("dad h", v);
				loadbc(s);
				return 1;
			}
			codegen_lr(r);
			helper(n, "bcshl");
			return 1;
		case T_SHREQ:
			if (r->op == T_CONSTANT) {
				if (v >= 8 && s == 1) {
					printf("\tmvi c,0\n");
					loadhl(n, s);
					return 1;
				}
				if (v >= 16) {
					printf("\tlxi b,0\n");
					loadhl(n, s);
					return 1;
				}
				if (v == 8 && (n->type & UNSIGNED)) {
					printf("\tmov c,b\nmvi b,0\n");
					loadhl(n, s);
					return 1;
				}
				if (!(n->type & UNSIGNED) && cpu == 8085 && v < 2 + 4 * opt) {
					loadhl(NULL,s);
					repeated_op("arhl", v);
					loadbc(s);
					return 1;
				}
			}
			codegen_lr(r);
			helper_s(n, "bcshr");
			return 1;
		case T_ANDEQ:
			reg_logic(n, s, 0, "bcana");
			return 1;
		case T_OREQ:
			reg_logic(n, s, 1, "bcora");
			return 1;
		case T_HATEQ:
			reg_logic(n, s, 2, "bcxra");
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
		printf("\tpush 0\n");
		return 1;
	case 4:
		if (optsize)
			printf("\tjsr __pushl\n");
		else
			/* Check order.. */
			printf("\tlda 1,__hireg\n\tpush 0\n\tpush 1\n");
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
	printf("\tlda 1,__ff\n\tand 1,0\n");
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
		if (size < 4) {
			return load_r_with(0, r, size == 1);
		} else if (size == 4) {
			/* TODO */
			return 0;
		} else
			error("nrb");
		return 0;
	case T_LBREF:
		if (size < 4)
			return load_r_with(0, r, size == 1);
		/* TODO 4 */
		return 0;
	case T_LREF:
		/* We are loading something then not using it, and it's local
		   so can go away */
		printf(";L sp %d %s(%ld)\n", sp, namestr(n->snum), n->value);
		if (nr)
			return 1;
		v += sp;
		printf("lda 0,%d,3\n", v);
		/* TODO: size 4 - can't happen currently */
		return 1;
	case T_RREF:
		if (nr)
			return 1;
		printf("\tlda 0,__reg%d\n", n->value);
		return 1;
	case T_NSTORE:
		if (size == 4) {
			printf("\tshld %s+%d\n", namestr(n->snum), v);
			printf("\txchg\n\tlhld __hireg\nshld %s+%d\n\txchg\n",
				namestr(n->snum), v + 2);
			return 1;
		}
		if (size == 1)
			printf("\tmov a,l\n\tsta");
		else
			printf("\tshld");
		printf(" _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LBSTORE:
		if (size == 4) {
			printf("\tshld T%d+%d\n", n->val2, v);
			printf("\txchg\n\tlhld __hireg\n\tshld T%d+%d\n\txchg\n",
				n->val2, v + 2);
			return 1;
		}
		if (size == 1)
			printf("\tmov a,l\n\tsta");
		else
			printf("\tshld");
		printf(" T%d+%d\n", n->val2, v);
		return 1;
	case T_LSTORE:
/*		printf(";L sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		v += sp;
		printf("\tsta 0,%d,3\n", v);
		/* TODO size 4 */
		return 1;
	case T_RSTORE:
		printf("\tsta 0,__reg%d\n", n->value);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tjsr .+1\n\t.word %s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		printf("\tmov 0,2\n"); 
		if (size == 2) {
			printf("\tpop 0\nsta 0,0,2\n");
		} else if (size == 1)
			printf("\tpop 0\nstb 0,2\n");
		break;
	case T_RDEREF:
		/* TODO: rewrite with offsets like Z80 */
		printf("\tlda@ 0,__r%d\n", n->value);
		return 1;
	case T_DEREF:
		if (size == 2) {
			printf("\tmov 0,2\n");
			printf("\tlda 0,0,2\n");
			return 1;
		}
		if (size == 1) {
			printf("\tldb 0,0\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		/* Pad word as functions always return one word on */
		printtf("\tjsr 0,3\n\t.word 0\n");
		return 1;
	case T_LABEL:
		if (nr)
			return 1;
		/* Used for const strings and local static */
		printf("\tlxi h,T%d+%d\n", n->val2, v);
		return 1;
	case T_CONSTANT:
		if (nr)
			return 1;
		switch(size) {
		case 4:
			printf("\tlxi h,%u\n", ((v >> 16) & 0xFFFF));
			printf("\tshld __hireg\n");
		case 2:
			printf("\tlxi h,%d\n", (v & 0xFFFF));
			return 1;
		case 1:
			printf("\tmvi l,%d\n", (v & 0xFF));
			return 1;
		}
		break;
	case T_NAME:
		if (nr)
			return 1;
		printf("\tlxi h,");
		printf("_%s+%d\n", namestr(n->snum), v);
		return 1;
	/* FIXME: LBNAME ?? */
	case T_LOCAL:
		if (nr)
			return 1;
		v += sp;
/*		printf(";LO sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			printf("\txchg\n");
		} else {
			printf("\tlxi h,%d\n", v);
			printf("\tdad sp\n");
		}
		return 1;
	case T_ARGUMENT:
		if (nr)
			return 1;
		v += frame_len + argbase + sp;
		printf("\tlda 0,%d,3\n", v);
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
			printf("\tpop 1\n\tadd 1,0\n");
			return 1;
		}
		break;
	case T_MINUS:
		if (size <= 2) {
			printf("\tpop 1\n\tsub 0,1\n\tmov 1,0\n");
			return 1;
		}
		break;
	case T_BANG:
		if (size <= 2) {
			printf("\tmov# 0,0,szr\n");	/* Skip if zero */
			printf("\tadc 0,0,skp\n");	/* Generate -1, skip */
			printf("\tsubo 0,0\n");		/* Generate 0 */
			return 1;
		}
		break;
	}
	return 0;
}
