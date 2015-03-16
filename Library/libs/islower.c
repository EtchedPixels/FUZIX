#include <ctype.h>

int islower(int c)
{
	char bc = c;
	return (bc >= 'a') && (bc <= 'z');
}
