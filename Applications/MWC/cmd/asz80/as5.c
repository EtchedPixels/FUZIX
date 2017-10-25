/*
 * Z-80 assembler.
 * Build up lines for the
 * listing file.
 */
#include	"as.h"

/*
 * Copy the data in the listing
 * code buffer to the listing file.
 * Produce no file if "lfp" is NULL.
 * Honour the listing mode stored
 * in the "lmode".
 */
void list(void)
{
	char *wp;
	int nb;

	if (lfp==NULL || lmode==NLIST)
		return;
	while (ep < &eb[NERR])
		*ep++ = ' ';
	fprintf(lfp, "%.10s", eb);
	if (lmode == SLIST) {
		fprintf(lfp, "%31s %5d %s", "", line, ib);
		return;
	}
	fprintf(lfp, "   %04x", laddr);
	if (lmode == ALIST) {
		fprintf(lfp, "%24s %5d %s", "", line, ib);
		return;
	}
	wp = cb;
	nb = cp - cb;
	list1(wp, nb, 1);
	fprintf(lfp, " %5d %s", line, ib);
	while ((nb -= 8) > 0) {
		wp += 8;
		fprintf(lfp, "%17s", "");
		list1(wp, nb, 0);
		fprintf(lfp, "\n");
	}
}

/*
 * Copy out a partial line
 * to the listing. Used for the first
 * and the extra lines in BLIST and
 * WLIST mode.
 */
void list1(char *wp, int nb, int f)
{
	int d;
	int i;

	if (nb > 8)
		nb = 8;
	for (i=0; i<nb; ++i) {
		d = (*wp++) & 0xFF;
		if (lmode == BLIST)
			fprintf(lfp, " %02x", d);
		else {
			d |= *wp++ << 8;
			fprintf(lfp, "  %04x", d);
			++i;
		}
	}
	if (f != 0) {
		while (i < 8) {
			fprintf(lfp, "   ");
			++i;
		}
	}
}
