#include <string.h>

size_t strlen(const char *t)
{
	size_t ct = 0;
	while (*t++)
		ct++;
	return ct;
}
