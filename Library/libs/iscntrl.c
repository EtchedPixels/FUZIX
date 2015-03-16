#include <ctype.h>

int iscntrl(int c)
{
	char bc = c;
	return ((bc >= 0) && (bc <= 31)) || (bc == 127);
}
