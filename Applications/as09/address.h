/* address.h - global variables involving addresses for assembler */

EXTERN struct address_s lastexp;/* last expression parsed */

EXTERN union
{
    char fcbuf[LINLEN - 6];	/* buffer for fcb and fcc data */
				/* data is absolute in 1 char pieces */
				/* limited by FCC\t"" etc on line */
    struct address_s fdbuf[(LINLEN - 4) / 2];
				/* buffer for fdb data */
				/* data can be of any 2-byte adr type */
				/* limited by FDB\t and commas on line */
#if SIZEOF_OFFSET_T > 2
    struct address_s fqbuf[(LINLEN - 4) / 4];
				/* buffer for fqb data */
				/* data can be of any 4-byte adr type */
				/* limited by FQB\t and commas on line */
#endif
}
    databuf;

EXTERN bool_t fcflag;
EXTERN bool_t fdflag;
#if SIZEOF_OFFSET_T > 2
EXTERN bool_t fqflag;
#endif

EXTERN struct address_s immadr;
EXTERN smallcount_t immcount;
