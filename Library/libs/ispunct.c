#include <stdint.h>
#include <ctype.h>

#undef ispunct
int ispunct(int c)
{
	return !iscntrl(c) && !isalpha(c) && !isspace(c);
}

