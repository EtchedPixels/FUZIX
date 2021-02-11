#include <kernel.h>

void* memset(void* s, int c, size_t n)
{
	uint8_t* ss = s;
	while (n--)
		*ss++ = c;
	return s;
}

void* memcpy(void* dest, const void* src, size_t n)
{
	if ((((uint32_t)src | (uint32_t)dest | n) & 3) == 0)
	{
		uint32_t* d = dest;
		const uint32_t* s = src;
		n /= 4;
		while (n--)
			*d++ = *s++;
	}
	else
	{
		uint8_t* d = dest;
		const uint8_t* s = src;
		while (n--)
			*d++ = *s++;
	}
	return dest;
}

size_t strlen(const char* s)
{
	const char* p = s;
	while (*s++)
		;
	return s - p - 1;
}

/* vim: sw=4 ts=4 et: */

