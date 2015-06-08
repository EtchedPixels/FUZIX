/* type.h - types for assembler */

typedef unsigned short u2_t;
typedef unsigned u2_pt;
typedef unsigned long u4_t;
typedef unsigned long u4_pt;

/* redefine foo_t's because their use has become non-portable */

#define bool_t bool_T
#define count_t count_T
#define fd_t fd_T
#define indcount_t indcount_T
#define offset_t offset_T
#define opcode_t opcode_T
#define opsize_t opsize_T
#define scale_t scale_T
#define sem_t sem_T
#define smallcount_t smallcount_T
#define soffset_t soffset_T

typedef unsigned char bool_t;
typedef int bool_pt;
typedef unsigned count_t;
typedef int fd_t;
typedef unsigned char indcount_t;
#ifdef I80386
typedef unsigned long offset_t;
typedef long soffset_t;
# define SIZEOF_OFFSET_T 4	/* non-portable */
#endif
#ifdef MC6809
typedef unsigned offset_t;
typedef int soffset_t;
# define SIZEOF_OFFSET_T 2	/* but sizeof (offset_t) often breaks cpp */
#endif
typedef int opcode_pt;
typedef unsigned char opcode_t;
typedef int opsize_pt;
typedef unsigned char opsize_t;
typedef unsigned reg_pt;
typedef unsigned char scale_t;
typedef unsigned char smallcount_t;
typedef /* signed */ char sem_t;
typedef unsigned u16_pt;
typedef unsigned short u16_T;
typedef unsigned long u32_T;

/* symbol table entry */

struct sym_s
{
    struct sym_s *next;		/* next symbol in hash chain (NUL_PTR if none) */
				/* zero offset because it is accessed most */
    unsigned char type;
    unsigned char data;		/* flags valid for expressions as well as syms*/
    union
    {
	offset_t value;		/* value, if sym is a label */
	unsigned char reg;	/* register code, if sym is a register */
	struct
	{
	    unsigned char routine;	/* routine number */
	    opcode_t opcode;	/* opcode, if sym is a hardware op */
	}
	    op;			/* if sym is pseudo-op or hardware op */
    }
        value_reg_or_op;
    unsigned char length;	/* length of symbol string */
    char name[1];		/* string of variable length */
};

/* address */

struct address_s
{
    offset_t offset;
    unsigned char data;
    struct sym_s *sym;
};

#ifdef I80386

/* effective address */

struct ea_s
{
    indcount_t indcount;
    opsize_t size;
    reg_pt base;
    reg_pt index;
    scale_t scale;
    struct address_s displ;
};

#endif

/* flags */

struct flags_s
{
    bool_t global;
    bool_t current;
    int semaphore;
};

/* location counter */

struct lc_s
{
    unsigned char data;
    offset_t lc;
};

/* string chain */

struct schain_s
{
    struct schain_s *next;
    char string[2];		/* variable length */
};

/* block stack */

struct block_s
{
    unsigned char data;
    unsigned char dp;
    offset_t lc;
};

/* if stack */

struct if_s
{
    bool_t ifflag;
    bool_t elseflag;
};

/* macro stack */

struct macro_s
{
    char *text;
    struct schain_s *parameters;
};

/* symbol listing format */

struct sym_listing_s
{
    char name[SYMLIS_NAMELEN];
    char zname[2];
    char segm[1];
    char pad1[1];
    char value[4];
    char pad2[1];
    char ar[1];
    char pad3[1];
    char cein[1];
    char pad4[1];
    char nullterm;
};

#if __STDC__
typedef void (*pfv)(void);
#else
typedef void (*pfv)();
#endif

#include "proto.h"
