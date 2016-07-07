#include "stdio-l.h"

int fputs(const void *str, FILE * fp)
{
	register int n = 0;
	const char *s = str;

	while (*s) {
		if (putc(*s++, fp) == EOF)
			return (EOF);
		++n;
	}
	return (n);
}
