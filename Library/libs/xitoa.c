/*
 *	Internal wrapper for ltostr. Ought to go away
 */
#include <stdlib.h>

/*********************** xitoa.c ***************************/
char *_itoa(int i)
{
	static char buf[16];
	return ltostr((long) i, buf, 10);
}
