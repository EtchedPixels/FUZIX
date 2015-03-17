#include <stdint.h>
#include <ctype.h>
#include <string.h>

int isspace(int c)
{
	return !!strchr(" \t\n\r\f\v", c);
}

