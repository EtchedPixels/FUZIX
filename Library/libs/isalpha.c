#include <ctype.h>

int isalpha(int c)
{
	return isupper(c) || islower(c);
}

