struct symbol
{
    unsigned name;		/* The name of the symbol */
    unsigned infonext;		/* Info and next */
    unsigned type;		/* Type of the symbol */
    union {
        unsigned *idx;		/* Index into object specific data */
        int offset;		/* Offset for locals */
    } data;
};

#define INITIALIZED	0x0800
#define S_FREE		0x0000	/* Unused */
#define S_AUTO		0x1000	/* Auto */
#define S_REGISTER	0x2000	/* Register */
#define S_ARGUMENT	0x3000	/* Argument */
#define S_LSTATIC	0x4000	/* Static in local scope */
#define S_STATIC	0x5000	/* Static in public scope */
#define S_EXTERN	0x6000	/* External reference */
#define S_EXTDEF	0x7000	/* Exported global */
#define S_TYPEDEF	0x8000	/* A typedef */
#define S_STRUCT	0x9000	/* The name of a struct type */
#define S_UNION		0xA000	/* The name of a union type */
#define S_ARRAY		0xB000	/* An array description slot (unnamed) */
#define S_FUNCDEF	0xC000	/* A function definition */
#define S_BSS		0xD000	/* Only used to pass info to code generator */
#define S_ENUM		0xE000	/* An enumeration name */

#define	S_STORAGE(x)	((x) & 0xF000)
#define S_INDEX(x)	((x) & 0x07FF)

#define S_NONE		0x0000	/* Used to tell internal code not to add symbols */

/* For types idx always points to the symbol entry holding the complex type.
   In turn the idx for it points to the desciption blocks.
   Offset for locals gives the base stack offset of the symbol */

extern struct symbol *update_symbol(struct symbol *sym, unsigned name, unsigned storage, unsigned type);
extern struct symbol *update_symbol_by_name(unsigned name, unsigned storage, unsigned type);
extern struct symbol *find_symbol(unsigned name, unsigned global);
extern struct symbol *find_symbol_by_class(unsigned name, unsigned class);
extern struct symbol *alloc_symbol(unsigned name, unsigned local);
extern void pop_local_symbols(struct symbol *top);
extern struct symbol *mark_local_symbols(void);
extern unsigned *sym_find_idx(unsigned storage, unsigned *idx, unsigned len);
extern unsigned func_return(unsigned type);
extern unsigned *func_args(unsigned type);
extern unsigned make_function(unsigned type, unsigned *id);
extern unsigned func_symbol_type(unsigned type, unsigned *idx);
extern struct symbol *symbol_ref(unsigned t);
extern unsigned array_num_dimensions(unsigned type);
extern unsigned array_dimension(unsigned type, unsigned depth);
extern unsigned array_type(unsigned type);
extern unsigned make_array(unsigned type, unsigned *id);
extern unsigned array_with_size(unsigned type, unsigned size);
extern unsigned array_compatible(unsigned t1, unsigned t2);
extern struct symbol *update_struct(unsigned name, unsigned isstruct);
extern unsigned type_of_struct(struct symbol *sym);
extern unsigned *struct_find_member(unsigned name, unsigned fname);

extern void write_bss(void);
