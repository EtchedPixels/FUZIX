/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

#include <stdio.h>
#include <stdlib.h>

/* reverse lines of a file */

#define N 1024
char line[N];
FILE *input;

int main(int argc, char *argv[])
{
	register int i, c;
	input = stdin;
	do {
		if (argc > 1) {
			if ((input = fopen(argv[1], "r")) == NULL) {
				fprintf(stderr, "rev: cannot open %s\n",
					argv[1]);
				exit(1);
			}
		}
		for (;;) {
			for (i = 0; i < N; i++) {
				line[i] = c = getc(input);
				switch (c) {
				case EOF:
					goto eof;
				default:
					continue;
				case '\n':
					break;
				}
				break;
			}
			while (--i >= 0)
				putc(line[i], stdout);
			putc('\n', stdout);
		}
	      eof:
		if (input != stdin)
			fclose(input);
		argc--;
		argv++;
	} while (argc > 1);
	return 0;
}
