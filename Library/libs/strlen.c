#include <string.h>

size_t strlen(const char *t)
{
	register size_t ct = 0;
	while (*t++)
		ct++;
	return ct;
}
