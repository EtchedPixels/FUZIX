/*
 *	Things to improve
 *	- Use the BC register either via a register hint in the compiler or by looking for
 *	  a non pointer arg candidate (especially on 8080 where the loads are expensive)
 *	- Improve initialized variable generation in stack frame creation
 *	- Avoid the two xchg calls on a void function cleanup (see gen_cleanup)
 *	- Do we want a single "LBREF or NREF name print" fuinction
 *	- Optimize xor 0xff with cpl ?
 *	- Inline load and store of long to static/global/label (certainly for -O2)
 *	- See if we can think down support routines that use the retaddr patching (ideally
 *	  remove them, if not fix Fuzix task switch to save/restore it)
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

#define LWDIRECT 24	/* Number of __ldword1 __ldword2 etc forms for fastest access */

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
void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size)
{
	frame_len = size;
	sp = 0;

	argbase = ARGBASE;
	if (func_flags & F_REG(1)) {
		printf("\tpush b\n");
		argbase += 2;
	}
	if (size > 10) {
		printf("\tlxi h,%d\n", -size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		return;
	}
	if (size & 1) {
		printf("\tdcx sp\n");
		size--;
	}
	while(size) {
		printf("\tpush h\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size)
{
	if (sp != 0)
		error("sp");
	/* Return in HL, does need care on stack. TOOD: flag void functions
	   where we can burn the return */
	sp -= size;
	if (cpu == 8085 && size <= 255 && size > 4) {
		printf("\tldsi %d\n", size);
		printf("\txchg\n");
		printf("\tsphl\n");
		printf("\txchg\n");
	} else if (size > 10) {
		unsigned x = func_flags & F_VOIDRET;
		if (!x)
			printf("\txchg\n");
		printf("\tlxi h,0x%x\n", (uint16_t)size);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		if (!x)
			printf("\txchg\n");
	} else {
		if (size & 1) {
			printf("\tinx sp\n");
			size--;
		}
		while (size) {
			printf("\tpop d\n");
			size -= 2;
		}
	}
	if (func_flags & F_REG(1))
		printf("\tpop b\n");
	printf("\tret\n");
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
	if (v > 10) {
		/* This is more expensive, but we don't often pass that many
		   arguments so it seems a win to stay in HL */
		/* TODO: spot void function and skip xchg */
		printf("\txchg\n");
		printf("\tlxi h,%d\n", v);
		printf("\tdad sp\n");
		printf("\tsphl\n");
		printf("\txchg\n");
	} else {
		while(v >= 2) {
			printf("\tpop d\n");
			v -= 2;
		}
		if (v)
			printf("\tinx sp\n");
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
	printf("\tcall __");
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
 */
static unsigned access_direct(struct node *n)
{
	unsigned op = n->op;
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	/* TODO group the user ones together for a range check ? */
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
 */

static unsigned load_r_with(const char r, struct node *n)
{
	unsigned v = WORD(n->value);
	const char *name;

	switch(n->op) {
	case T_NAME:
		printf("\tlxi %c,_%s+%d\n", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\tlxi %c,T%d+%d\n", r, n->val2, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tlxi %c,%d\n", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		if (r == 'b')
			return 0;
		else if (r == 'h') {
			printf("\tlhld _%s+%d\n", name, v);
			return 1;
		} else if (r == 'd') {
			/* We know it is int or pointer */
			printf("\txchg\n");
			printf("\tlhld _%s+%d\n", name, v);
			printf("\txchg\n");
			return 1;
		}
		break;
	/* TODO: fold together cleanly with NREF */
	case T_LBREF:
		if (r == 'b')
			return 0;
		else if (r == 'h') {
			printf("\tlhld T%d+%d\n", n->val2, v);
			return 1;
		} else if (r == 'd') {
			/* We know it is int or pointer */
			printf("\txchg\n");
			printf("\tlhld T%d+%d\n", n->val2, v);
			printf("\txchg\n");
			return 1;
		}
		break;
	case T_RREF:
		if (r == 'd')
			printf("\tmov d,b\n\tmov e,c\n");
		else if (r == 'h')
			printf("\tmov l,c\n\tmov h,b\n");
		/* Assumes that BC isn't corrupted yet so is already the right value. Use
		   this quirk with care (eg T_MINUS) */
		return 1;
	default:
		return 0;
	}
	return 1;
}

static unsigned load_bc_with(struct node *n)
{
	return load_r_with('b', n);
}

static unsigned load_de_with(struct node *n)
{
	return load_r_with('d', n);
}

static unsigned load_hl_with(struct node *n)
{
	return load_r_with('h', n);
}

static unsigned load_a_with(struct node *n)
{
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tmvi a,%d\n", BYTE(n->value));
		break;
	case T_NREF:
		printf("\tlda _%s+%d\n", namestr(n->snum), WORD(n->value));
		break;
	case T_LBREF:
		printf("\tlda T%d+%d\n", n->val2, WORD(n->value));
		break;
	case T_RREF:
		printf("\tmov a,c\n");
		break;
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

static void loadhl(struct node *n, unsigned s)
{
	if (n && (n->flags & NORETURN))
		return;
	printf("\tmov l,c\n");
	if (s == 2)
		printf("\tmov h,b\n");
}

static void loadbc(unsigned s)
{
	printf("\tmov c,l\n");
	if (s == 2)
		printf("\tmov b,h\n");
}

/* We use "DE" as a name but A as register for 8bit ops... probably ought to rework one day */
static unsigned gen_deop(const char *op, struct node *n, struct node *r, unsigned sign)
{
	unsigned s = get_size(n->type);
	if (s > 2)
		return 0;
	if (s == 2) {
		if (load_de_with(r) == 0)
			return 0;
	} else {
		if (load_a_with(r) == 0)
			return 0;
	}
	if (sign)
		helper_s(n, op);
	else
		helper(n, op);
	return 1;
}

/* TODO: someone needs to own eliminating no side effect impossible
   or true expressions like unsigned < 0 */
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
	if (gen_deop(op, n, r, sign)) {
		n->flags |= ISBOOL;
		return 1;
	}
	return 0;
}

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
			printf("\tpush h\n");
		}
		printf("\tdad h\n");
		n >>= 1;
	}
	while(pops--) {
		printf("\tpop d\n\tdad d\n");
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
		printf("\tlxi h,0\n");
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
	if (cpu != 8085)
		return 0;

	if (n & (n - 1))
		return 0;

	while(n > 1) {
		printf("\tarhl\n");
		n >>= 1;
	}
	return 1;
}


/* TODO : we could in theory optimize xor 255 with cpl ? */
static unsigned gen_logicc(struct node *n, unsigned s, const char *op, unsigned v, unsigned code)
{
	unsigned h = (v >> 8) & 0xFF;
	unsigned l = v & 0xFF;

	if (s > 2 || (n && n->op != T_CONSTANT))
		return 0;

	/* If we are trying to be compact only inline the short ones */
	if (optsize && ((h != 0 && h != 255) || (l != 0 && l != 255)))
		return 0;

	if (s == 2) {
		if (h == 0) {
			if (code == 1)
				printf("\tmvi h,0\n");
		}
		else if (h == 255 && code != 3) {
			if (code == 2)
				printf("\tmvi h,255\n");
		} else {
			printf("\tmov a,h\n\t%s %d\n\tmov h,a\n", op, h);
		}
	}
	if (l == 0) {
		if (code == 1)
			printf("\tmvi l,0\n");
	} else if (l == 255 && code != 3) {
		if (code == 2)
			printf("\tmvi l,255\n");
	} else {
		printf("\tmov a,l\n\t%s %d\n\tmov l,a\n", op, l);
	}
	return 1;
}

static unsigned gen_fast_remainder(unsigned n, unsigned s)
{
	unsigned mask;
	if (s != 2)
		return 0;
	if (n == 1) {
		printf("\tlxi h,0\n");
		return 1;
	}
	if (n == 256) {
		printf("\tmvi h,0\n");
		return 1;
	}
	if (n & (n - 1))
		return 0;
	if (!optsize) {
		mask = n - 1;
		gen_logicc(NULL, s, "ani", mask, 1);
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
			printf("\tmov a,l\n\tsta");
		else
			printf("\tshld ");
		printf("_%s+%d\n", namestr(n->snum), WORD(n->value));
			return 1;
		/* TODO 4/8 for long etc */
		return 0;
	case T_LBSTORE:
		if (s > 2)
			return 0;
		if (s == 1)
			printf("\tmov a,l\n\tsta");
		else
			printf("\tshld");
		printf(" T%d+%d\n", n->val2, v);
		return 1;
	case T_RSTORE:
		loadbc(s);
		return 1;
	case T_EQ:
		/* The address is in HL at this point */
		if (cpu == 8085 && s == 2 ) {
			printf("\txchg\n");
			if (load_hl_with(r) == 0)
				error("teq");
			printf("\tshlx\n");
			return 1;
		}
		if (s == 1) {
			/* We need to end up with the value in l if this is not NORETURN, also
			   we can optimize constant a step more */
			if (r->op == T_CONSTANT && (n->flags & NORETURN))
				printf("\tmvi m,%d\n", ((unsigned)r->value) & 0xFF);
			else {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tmov m,a\n");
				if (!(n->flags & NORETURN))
					printf("\tmov l,a\n");
			}
			return 1;
		}
		return 0;
	case T_PLUS:
		/* Zero should be eliminated in cc1 FIXME */
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 4 && s <= 2) {
				if (s == 1)
					repeated_op("inr l", v);
				else
					repeated_op("inx h", v);
				return 1;
			}
		}
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tmov e,a\n");
			}
			/* Short cut register case */
			if (r->op == T_REG) {
				printf("\tdad b\n");
				return 1;
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			printf("\tdad d\n");
			return 1;
		}
		return 0;
	case T_MINUS:
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (v < 6 && s <= 2) {
				if (s == 1)
					repeated_op("dcr l", v);
				else
					repeated_op("dcx h", v);
				return 1;
			}
			printf("\tlxi d,%d\n", 65536 - v);
			printf("\tdad d\n");
			return 1;
		}
		if (cpu == 8085 && s <= 2)  {
			/* Shortcut subtracting register from working value */
			if (r->op == T_REG) {
				printf("\tdsub\n");
				return 1;
			}
			if (access_direct_b(r)) {
				printf("\tpush b\n");
				sp += 2;
				/* Must not corrupt B before we are ready */
				/* LHS is in HL at the moment, end up with the result in HL */
				if (s == 1) {
					if (load_a_with(r) == 0)
						error("min1");
					printf("\tmov c,a\n");
				} else {
					if (load_bc_with(r) == 0)
						error("min2");
				}
				printf("\tdsub  ; b\n\tpop b\n");
				sp -= 2;
				return 1;
			}
		}
		return 0;
	case T_STAR:
		if (r->op == T_CONSTANT) {
			if (s <= 2 && can_fast_mul(s, r->value)) {
				 gen_fast_mul(s, r->value);
				return 1;
			}
		}
		return gen_deop("mulde", n, r, 0);
	case T_SLASH:
		/* FIXME: 8085 ARHL is signed so this is wrong for large numbers ? */
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_div(s, r->value))
				return 1;
		}
		return gen_deop("divde", n, r, 1);
	case T_PERCENT:
		if (r->op == T_CONSTANT && (n->type & UNSIGNED)) {
			if (s <= 2 && gen_fast_remainder(s, r->value))
				return 1;
		}
		return gen_deop("remde", n, r, 1);
	case T_AND:
		if (gen_logicc(r, s, "ani", r->value, 1))
			return 1;
		return gen_deop("bandde", n, r, 0);
	case T_OR:
		if (gen_logicc(r, s, "ori", r->value, 2))
			return 1;
		return gen_deop("borde", n, r, 0);
	case T_HAT:
		if (gen_logicc(r, s, "xri", r->value, 3))
			return 1;
		return gen_deop("bxorde", n, r, 0);
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
			if (r->value < 8)
				repeated_op("dad h", r->value);
			else
				printf("\tmov h,l\n\tmvi l,0\n");
			return 1;
		}
		return gen_deop("shlde", n, r, 0);
	case T_GTGT:
		/* >> by 8 unsigned */
		if (s == 2 && (n->type & UNSIGNED) && r->op == T_CONSTANT && r->value == 8) {
			printf("\tmov l,h\n\tmvi h,0\n");
			return 1;
		}
		/* 8085 has a signed right shift 16bit */
		if (cpu == 8085 && (!(n->type & UNSIGNED)) && s == 2) {
			if (s <= 2 && r->op == T_CONSTANT && r->value < 8) {
				repeated_op("arhl", r->value);
				return 1;
			}
		}
		return gen_deop("shrde", n, r, 1);
	/* Shorten post inc/dec if result not needed - in which case it's the same as
	   pre inc/dec */
	case T_PLUSPLUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_PLUSEQ:
		if (s == 1) {
			if (r->op == T_CONSTANT && r->value < 4 && (n->flags & NORETURN))
				repeated_op("inr m", r->value);
			else {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tadd m\n\tmov m,a\n");
				if (!(n->flags & NORETURN))
					printf("\tmov l,a\n");
			}
			return 1;
		}
		return gen_deop("pluseqde", n, r, 0);
	case T_MINUSMINUS:
		if (!(n->flags & NORETURN))
			return 0;
	case T_MINUSEQ:
		if (s == 1) {
			/* Shortcut for small 8bit values */
			if (r->op == T_CONSTANT && r->value < 4 && (n->flags & NORETURN)) {
				repeated_op("dcr m", r->value);
			} else {
				/* Subtraction is not transitive so this is
				   messier */
				if (r->op == T_CONSTANT) {
					if (r->value == 1)
						printf("\tmov a,m\n\tdcr a\n\tmov m,a");
					else
						printf("\tmov a,m\n\tsbi %d\n\tmov m,a",
							(int)r->value);
				} else {
					if (load_a_with(r) == 0)
						return 0;
					printf("\tcma\n\tinr a\n\n");
					printf("\tsub m\n\tmov m,a\n");
				}
				if (!(n->flags & NORETURN))
					printf("\tmov l,a\n");
			}
			return 1;
		}
		return gen_deop("minuseqde", n, r, 0);
	case T_ANDEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			printf("\tana m\n\tmov m,a\n");
			if (!(n->flags & NORETURN))
				printf("\tmov l,a\n");
			return 1;
		}
		return gen_deop("andeqde", n, r, 0);
	case T_OREQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			printf("\tora m\n\tmov m,a\n");
			if (!(n->flags & NORETURN))
				printf("\tmov l,a\n");
			return 1;
		}
		return gen_deop("oreqde", n, r, 0);
	case T_HATEQ:
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			printf("\txra m\n\tmov m,a\n");
			if (!(n->flags & NORETURN))
				printf("\tmov l,a\n");
			return 1;
		}
		return gen_deop("xoreqde", n, r, 0);
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

static void reg_logic(struct node *n, unsigned s, unsigned op, const char *i)
{
	/* TODO : constant optimization */
	codegen_lr(n->right);
	/* HL is now the value to combine with BC */
	if (opt > 1) {
		/* TODO - can avoid the reload into HL if NORETURN */
		if (s == 2)
			printf("\tmov a,b\n\t%s h\n\tmov b,a\nmov h,a\n", i + 2);
		printf("\tmov a,c\n\t%s c\n\tmov c,a\nmov l,a\n", i + 2);
	} else {
		helper(n, i);
		loadhl(n, s);
	}
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
			printf("\tshld");
		else
			printf("\tmov a,l\n\tsta");
		printf(" _%s+%d\n", namestr(n->snum), WORD(n->value));
		return 1;
	}
	/* Locals we can do on 8085, 8080 is doable but messy - so not worth it */
	if (n->op == T_LSTORE && s <= 2) {
		if (n->value + sp == 0 && s == 2) {
			/* The one case 8080 is worth doing */
			codegen_lr(r);
			if (n->flags & NORETURN)
				printf("\txthl\n");
			else
				printf("\tpop psw\n\tpush h\n");
			return 1;
		}
		if (cpu == 8085 && n->value + sp < 255) {
			codegen_lr(r);
			printf("\tldsi %d\n", WORD(n->value + sp));
			if (s == 2)
				printf("\tshlx\n");
			else
				printf("\tmov a,l\n\tstax d\n");
			return 1;
		}
	}
	/* Shortcut any initialization of BC we can do directly */
	if (n->op == T_RSTORE) {
		if (load_bc_with(r))
			return 1;
		return 0;
	}
	/* Assignment to *BC, byte pointer always */
	if (n->op == T_REQ) {
		/* Try and get the value into A */
		if (!load_a_with(r)) {
			codegen_lr(r);		/* If not then into HL */
			printf("\tmov a,l\n");
		}
		printf("\tstax b\n");	/* Do in case volatile */
		if (!nr)
			printf("\tmov l,a\n");
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
				loadhl(n, s);
				reg_incdec(s, v);
				return 1;
			}
			if (!nr) {
				printf("\tpush b\n");
				sp += 2;
			}
			/* Fall through */
		case T_PLUSEQ:
			if (reg_canincdec(r, s, v)) {
				reg_incdec(s, v);
				if (nr)
					return 1;
				if (n->op == T_PLUSEQ) {
					loadhl(n, s);
				}
			} else {
				/* Amount to add into HL */
				codegen_lr(r);
				printf("\tdad b\n");
				printf("\tmov c,l\n");
				if (s == 2)
					printf("\tmov b,h\n");
			}
			if (n->op == T_PLUSPLUS && !(n->flags & NORETURN)) {
				printf("\tpop h\n");
				sp -= 2;
			}
			return 1;
		case T_MINUSMINUS:
			if (!(n->flags & NORETURN)) {
				if (reg_canincdec(r, s, -v)) {
					loadhl(n, s);
					reg_incdec(s, -v);
					return 1;
				}
				codegen_lr(r);
				if (s == 1) {
					printf("\tmov a,c\n\tsub l\n\tmov l,c\n\tmov c,a\n");
					return 1;
				}
				/* TODO: can we inline constants by doing an add of negative ? */
				helper(n, "bcsub");
				return 1;
			}
			/* If we don't care about the return they look the same so fall
			   through */
		case T_MINUSEQ:
			if (reg_canincdec(r, s, -v)) {
				reg_incdec(s, -v);
				loadhl(n, s);
				return 1;
			}
			/* Get the subtraction value into HL */
			codegen_lr(r);
			helper(n, "bcsub");
			/* Result is only left in BC reload if needed */
			loadhl(n, s);
			return 1;
		/* For now - we can do better - maybe just rewrite them into load,
		   op, store ? */
		case T_STAREQ:
			/* TODO: constant multiply */
			if (r->op == T_CONSTANT) {
				if (can_fast_mul(s, v)) {
					loadhl(NULL, s);
					gen_fast_mul(s, v);
					loadbc(s);
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
		printf("\tpush h\n");
		return 1;
	case 4:
		if (optsize)
			printf("\tcall __pushl\n");
		else
			printf("\txchg\n\tlhld __hireg\n\tpush h\n\tpush d\n");
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
	printf("\tmvi h,0\n");
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
			printf("\tlda _%s+%d\n", namestr(n->snum), v);
			printf("\tmov l,a\n");
		} else if (size == 2) {
			printf("\tlhld _%s+%d\n", namestr(n->snum), v);
			return 1;
		} else if (size == 4) {
			printf("\tlhld _%s+%d\n", namestr(n->snum), v + 2);
			printf("\tshld __hireg\n");
			printf("\tlhld _%s+%d\n", namestr(n->snum), v);
		} else
			error("nrb");
		return 1;
	case T_LBREF:
		if (size == 1) {
			printf("\tlda T%d+%d\n", n->val2, v);
			printf("\tmov l,a\n");
		} else if (size == 2) {
			printf("\tlhld T%d+%d\n", n->val2, v);
		} else if (size == 4) {
			printf("\tlhld T%d+%d\n", n->val2, v + 2);
			printf("\tshld __hireg\n");
			printf("\tlhld T%d+%d\n", n->val2, v);
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
		if (v == 0 && size == 2) {
			printf("\tpop h\n\tpush h\n");
			return 1;
		}
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			if (size == 2)
				printf("\tlhlx\n");
			else
				printf("\tldax d\n\tmov l,a\n");
			return 1;
		}
		/* Byte load is shorter inline for most cases */
		if (size == 1 && (!optsize || v >= LWDIRECT)) {
			printf("lxi h,%d\n\tdad sp\n\tmov l,m\n", v);
			return 1;
		}
		/* Word load is long winded on 8080 */
		if (size == 2 && (cpu == 8085 || opt > 2)) {
			printf("\tlxi h,%d\n\tdad sp\n", WORD(v));
			if (cpu == 8085)
				printf("\txchg\nlhlx\n");
			else
				printf("\tmov a,m\n\tinx h\n\tmov h,m\n\tmov l,a\n");
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		if (size == 1)
			name = "ldbyte";
		else if (size == 2)
			name = "ldword";
		else
			return 0;	/* Can't happen currently but trap it */
		/* We do a call so the stack offset is two bigger */
		if (v < LWDIRECT)
			printf("\tcall __%s%d\n", name, v + 2);
		else if (v < 253)
			printf("\tcall __%s\n\t.byte %d\n", name, v + 2);
		else
			printf("\tcall __%sw\n\t.word %d\n", name, v + 2);
		return 1;
	case T_RREF:
		if (nr)
			return 1;
		printf("\tmov l,c\n");
		if (size == 2)
			printf("\tmov h,b\n");
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
		if (v == 0 && size == 2 ) {
			if (nr)
				printf("\txthl\n");
			else
				printf("\tpop psw\n\tpush h\n");
			return 1;
		}
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			if (size == 2)
				printf("\tshlx\n");
			else
				printf("\tmov a,l\n\tstax d\n");
			return 1;
		}
		/* Large offsets for word on 8085 are 7 bytes, a helper call is 5 (3 with rst hacks)
		   and much slower. As these are fairly rare just inline it */
		if (cpu == 8085 && size == 2) {
			printf("\txchg\n\tlxi h,%d\n\tdad sp\n\txchg\n\tshlx\n", WORD(v));
			return 1;
		}
		if (size == 1 && (!optsize || v >= LWDIRECT)) {
			printf("\tmov a,l\n\tlxi h,%d\n\tdad sp\n\tmov m,a\n", WORD(v));
			if (!nr)
				printf("\tmov l,a\n");
			return 1;
		}
		/* For -O3 they asked for it so inline the lot */
		/* We dealt with size one above */
		if (opt > 2 && size == 2) {
			printf("\txchg\n\tlxi h,%d\n\tdad sp\n\tmov m,e\n\tinx h\n", WORD(v));
			printf("\tmov m,d\n");
			if (!nr)
				printf("\txchg\n");
			return 1;
		}
		/* Via helper magic for compactness on 8080 */
		/* Can rewrite some of them into rst if need be */
		if (size == 1)
			name = "stbyte";
		else if (size == 2)
			name = "stword";
		/* FIXME */
		else
			return 0;
		/* Like load the helper is offset by two because of the
		   stack */
		if (v < 24)
			printf("\tcall __%s%d\n", name, v + 2);
		else if (v < 253)
			printf("\tcall __%s\n\t.byte %d\n", name, v + 2);
		else
			printf("\tcall __%sw\n\t.word %d\n", name, v + 2);
		return 1;
	case T_RSTORE:
		loadbc(size);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		if (size == 2) {
			if (cpu == 8085)
				printf("\tpop d\n\tshlx\n");
			else {
				printf("\txchg\n\tpop h\n\tmov m,e\n\tinx h\n\tmov m,d\n");
				if (!(nr))
					printf("\txchg\n");
			}
			return 1;
		}
		if (size == 1) {
			printf("\tpop d\n\txchg\n\tmov m,e\n");
			if (!(n->flags & NORETURN))
				printf("\txchg\n");
			return 1;
		}
		break;
	case T_RDEREF:
		/* RREFs on 8080 will always be byte pointers */
		printf("\tldax b\n");	/* NORETURN but generated imples volatile so keep the ldax */
		if (!(n->flags & NORETURN))
			printf("\tmov l,a\n");
		return 1;
	case T_DEREF:
		if (size == 2) {
			if (cpu == 8085)
				printf("\txchg\n\tlhlx\n");
			else
				printf("\tmov e,m\n\tinx h\n\tmov d,m\n\txchg\n");
			return 1;
		}
		if (size == 1) {
			printf("\tmov l,m\n");
			return 1;
		}
		if (size == 4 && cpu == 8085 && !optsize) {
			printf("\txchg\n\tinx d\n\tinx d\n\tlhlx\n\tshld __hireg\t\n\tdcx d\n\tdcx d\n\tlhlx\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		printf("\tcall __callhl\n");
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
/*		printf(";AR sp %d spval %d %s(%ld)\n", sp, spval, namestr(n->snum), n->value); */
		if (cpu == 8085 && v <= 255) {
			printf("\tldsi %d\n", v);
			printf("\txchg\n");
		} else {
			printf("\tlxi h,%d\n", v);
			printf("\tdad sp\n");
		}
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
			printf("\tpop d\n\tdad d\n");
			return 1;
		}
		break;
	}
	return 0;
}
