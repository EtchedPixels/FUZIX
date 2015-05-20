#include	<string.h>

/* ANSIfied from dLibs 1.2 */

int strncmp(const char *str1, const char *str2, size_t limit)
{
	for(; ((--limit) && (*str1 == *str2)); ++str1, ++str2)
		if (*str1 == '\0')
			return 0;
	return *str1 - *str2;
}
