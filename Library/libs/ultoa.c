/* numeric/string conversions package
 */

#include "stdlib.h"

/**************************** ultoa.c ****************************/
char *ultoa(unsigned long value, char *strP, int radix)
{
	char hex = 'A';

	if (radix < 0) {
		hex = 'a';
		radix = -radix;
	}
	return __longtoa(value, strP, radix, 0, hex);
}
