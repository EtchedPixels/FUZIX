#include <string.h>

size_t strnlen(const char *t, size_t n)
{
	size_t ct = 0;
	while (*t++ && ct++ < n);
	return ct;
}
