#include <stdint.h>
#include <ctype.h>

#undef isgraph
int isgraph(int c)
{
	uint8_t cb = c;
	return (cb >= 33) && (cb <= 126);
}

