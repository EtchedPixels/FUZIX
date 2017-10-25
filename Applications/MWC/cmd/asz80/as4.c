/*
 * Z-80 assembler.
 * Output Intel compatable
 * hex files.
 */
#include	"as.h"

#define	NHEX	32			/* Longest record */

VALUE	hexla;
VALUE	hexpc;
char	hexb[NHEX];
char	*hexp	= &hexb[0];

/*
 * Output a word. Use the
 * standard Z-80 ordering (low
 * byte then high byte).
 */
void outaw(int w)
{
	outab(w);
	outab(w >> 8);
}

/*
 * Output an absolute
 * byte to the code and listing
 * streams.
 */
void outab(int b)
{
	if (pass != 0) {
		if (cp < &cb[NCODE])
			*cp++ = b;
		outbyte(b);
	}
	++dot;
}

/*
 * Put out the end of file
 * hex item at the very end of 
 * the object file.
 */
void outeof(void)
{
	outflush();
	fprintf(ofp, ":00000001FF\n");
}

/*
 * Output a hex byte. Flush
 * the buffer if no room. Store the
 * byte in the buffer, for future
 * checksumming. Remember the load
 * address for flushing.
 */
void outbyte(int b)
{
	if (hexp>=&hexb[NHEX] || hexpc!=dot) {
		outflush();
		hexp = &hexb[0];
	}
	if (hexp == &hexb[0]) {
		hexla = dot;
		hexpc = dot;
	}
	*hexp++ = b;
	++hexpc;
}

/*
 * Flush out a block of
 * code to the hex file. Figure
 * out the length word and the
 * checksum byte.
 */
void outflush(void)
{
	char *p;
	int b;
	int c;

	if ((b = hexp-&hexb[0]) != 0) {
		putc(':', ofp);
		outhex(b);
		outhex(hexla >> 8);
		outhex(hexla);
		outhex(0);
		c = b + (hexla>>8) + hexla;
		p = &hexb[0];
		while (p < hexp) {
			b = *p++;
			outhex(b);
			c += b;
		}
		outhex(-c);
		putc('\n', ofp);
	}
}

/*
 * Put out "b", as a 
 * two character hex value.
 * We cannot use printf because
 * of case problems on VMS.
 * Upper case ascii.
 */
void outhex(int b)
{
	static	const char hex[] = {
		'0',	'1',	'2',	'3',
		'4',	'5',	'6',	'7',
		'8',	'9',	'A',	'B',
		'C',	'D',	'E',	'F'
	};

	putc(hex[(b>>4)&0x0F], ofp);
	putc(hex[b&0x0F], ofp);
}
