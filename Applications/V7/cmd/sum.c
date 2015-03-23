/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * Sum bytes in file mod 2^16
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	register unsigned sum;
	register int i, c;
	register FILE *f;
	register long nbytes;
	int errflg = 0;

	i = 1;
	do {
		if (i < argc) {
			if ((f = fopen(argv[i], "r")) == NULL) {
				fprintf(stderr, "sum: Can't open %s\n",
					argv[i]);
				errflg += 10;
				continue;
			}
		} else
			f = stdin;
		sum = 0;
		nbytes = 0;
		while ((c = getc(f)) != EOF) {
			nbytes++;
			if (sum & 01)
				sum = (sum >> 1) + 0x8000;
			else
				sum >>= 1;
			sum += c;
			sum &= 0xFFFF;
		}
		if (ferror(f)) {
			errflg++;
			/* BUG: cast here is to work around a cc65 bug */
			fprintf(stderr, "sum: read error on %s\n",
				argc > 1 ? argv[i] : (const char*)"-");
		}
		printf("%05u%6ld", sum, (nbytes + BUFSIZ - 1) / BUFSIZ);
		if (argc > 2)
			printf(" %s", argv[i]);
		printf("\n");
		fclose(f);
	} while (++i < argc);
	exit(errflg);
}
