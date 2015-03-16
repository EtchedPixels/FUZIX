#include <ctype.h>

int isdigit(int c)
{
	char bc = c;
	return (bc >= '0') && (bc <= '9');
}

