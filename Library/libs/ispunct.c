#include <stdint.h>
#include <ctype.h>

int ispunct(int c)
{
	return !iscntrl(c) && !isalpha(c) && !isspace(c);
}

