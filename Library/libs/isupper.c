#include <ctype.h>

int isupper(int c)
{
	char bc = c;
	return (bc >= 'A') && (bc <= 'Z');
}
