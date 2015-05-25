#include <string.h>

void *memcpy(void *dest, const void *src, size_t len)
{
	uint8_t *dp = dest;
	const uint8_t *sp = src;
	while(len-- > 0)
		*dp++=*sp++;
	return dest;
}
