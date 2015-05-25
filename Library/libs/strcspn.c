/* strcspn.c */

/* from Schumacher's Atari library, improved */

#include <string.h>

size_t strcspn(const char *string, const char *set)
/*
 *	Return the length of the sub-string of <string> that consists
 *	entirely of characters not found in <set>.  The terminating '\0'
 *	in <set> is not considered part of the match set.  If the first
 *	character if <string> is in <set>, 0 is returned.
 */
{
    register const char *setptr;
    const char *start;

    start = string;
    while (*string)
    {
	setptr = set;
	do
	    if (*setptr == *string)
		goto break2;
	while (*setptr++);
	++string;
    }
break2:
    return string - start;
}
