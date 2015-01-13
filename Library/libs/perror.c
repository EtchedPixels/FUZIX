/* perror.c
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void wr2(const char *str)
{
	const char *p = str;

	while (*p)
		++p;
	write(2, str, (unsigned int) (p - str));
}

void perror(const char *str)
{
	if (!str)
		str = "error";
	wr2(str);
	wr2(": ");
	str = strerror(errno);
	wr2(str);
	wr2("\n");
}
