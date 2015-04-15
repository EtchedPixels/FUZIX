/* numeric/string conversions package
 */

#include <stdlib.h>

/**************************** atoi.c ****************************/
int atoi(const char *str)
{
	return (int) strtol(str, NULL, 10);
}
