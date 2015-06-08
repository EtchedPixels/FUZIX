/* macro.h - global variables for macro expansion for assembler */

EXTERN bool_t macflag;		/* inside macro flag */
EXTERN bool_t macload;		/* loading macro flag */
EXTERN unsigned macnum;		/* macro call counter */

EXTERN unsigned char maclevel;	/* nesting level */
EXTERN struct schain_s *macpar;	/* parameter save buffer */
EXTERN struct schain_s *macptop;	/* top of param buffer (+1) */
EXTERN struct macro_s *macstak;	/* stack ptr */
