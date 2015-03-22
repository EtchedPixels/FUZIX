/* This file has a unit test in Library/test/ctype.c. If you change this file,
 * please make sure the test still runs. */

#if !defined __TESTING__
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#endif

int isspace(int c)
{
	return ((uint8_t)c == ' ') ||
	       ((uint8_t)c == '\t') ||
		   ((uint8_t)c == '\n') ||
		   ((uint8_t)c == '\r') ||
		   ((uint8_t)c == '\f') ||
		   ((uint8_t)c == '\v');
}

