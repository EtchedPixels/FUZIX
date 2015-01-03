/*
 *	Internal wrapper for ltostr. Ought to go away
 */
#include <stdlib.h>

/*********************** xitoa.c ***************************/
char *_itoa(int i)
{
    return _ltoa((long)i);
}
