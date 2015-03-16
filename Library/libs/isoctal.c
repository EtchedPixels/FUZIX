#include <ctype.h>

int isoctal(int c)
{
	char bc = c;
	return (bc >= '0') && (bc <= '7');
}

