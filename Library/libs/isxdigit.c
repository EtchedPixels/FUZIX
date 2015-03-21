#include <stdint.h>

#define HAVE_STATIC_INLINE 0
#include <ctype.h>

int isxdigit(int c)
{
	return (((uint8_t)c >= '0') && ((uint8_t)c <= '9')) ||
		((((uint8_t)c|0x20) >= 'a') && (((uint8_t)c|0x20) <= 'f'));
}

