#include	<string.h>

/* ANSIfied from dLibs 1.2 and handling of 0 length compare done as per
   convention (equality) */

int memcmp(const void *mem1, const void *mem2, size_t len)
{
	const signed char *p1 = mem1, *p2 = mem2;

	if (!len)
		return 0;

	while (--len && *p1 == *p2) {
		p1++;
		p2++;
	}
	return *p1 - *p2;
}
