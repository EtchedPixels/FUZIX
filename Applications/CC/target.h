#define TARGET_MAX_INT		32767L
#define TARGET_MAX_LONG		2147483647UL	/* and a double persenne prime too */
#define TARGET_MAX_UINT		65535UL
#define TARGET_MAX_PTR		TARGET_MAX_UINT

#define TARGET_CHAR_MASK	0x00FFU
#define TARGET_SHORT_MASK	0xFFFFU
#define TARGET_LONG_MASK	0xFFFFFFFFUL

extern unsigned target_sizeof(unsigned t);
extern unsigned target_alignof(unsigned t, unsigned storage);
extern unsigned target_argsize(unsigned t);
extern unsigned target_ptr_arith(unsigned t);
extern unsigned target_scale_ptr(unsigned t, unsigned scale);
extern unsigned target_type_remap(unsigned t);
extern unsigned target_register(unsigned t);
extern void target_reginit(void);

/* Default integer type is 2 byte */
#define CINT	CSHORT
#define UINT	USHORT

#define TARGET_CHAR_UNSIGNED
