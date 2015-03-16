#include <ctype.h>

int isxdigit(int c)
{
	char bc = c;
	return isdigit(bc)
		|| ((bc >= 'a') && (bc <= 'f'))
		|| ((bc >= 'A') && (bc <= 'F'));
}
