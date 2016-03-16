/*
 *	Stdio avoidance library
 */

#include "advent.h"

void nl(void)
{
	write(1, "\n", 1);
}

void writes(const char *p)
{
	write(1, p, strlen(p));
}

void getinp(char *p, int l)
{
	char *ep = p + l - 1;
	int e;
	while ((e = read(0, p, 1)) == 1) {
		if (*p == '\n') {
			*p = 0;
			return;
		}
		if (p < ep)
			p++;
	}
	if (e || !isatty(0))
		exit(1);
	*p = 0;
	return;
}

void writei(uint16_t v)
{
#ifdef __linux__
	char buf[16];
	sprintf(buf, "%d", (unsigned int) v);
	writes(buf);
#else
	writes(_itoa(v));
#endif
}
