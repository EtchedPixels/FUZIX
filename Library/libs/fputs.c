#include "stdio-l.h"

int fputs(const char *s, FILE * fp)
{
	register int n = 0;

	while (*s) {
		if (putc(*s++, fp) == EOF)
			return (EOF);
		++n;
	}
	return (n);
}
