/* Test for the Fuzix libc ctype library. Compares Fuzix's implementation
 * with the system standard.
 *
 * Compile this with:
 *
 *    gcc Library/tests/ctype.c -o Library/tests/ctype
 *
 * ...and run them. They'll moan if there's anything wrong. Note that this
 * only tests the inline versions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

typedef int (*ctype_fn)(int c);

/* Import the system ctype functions under new names. */

#define define_sys(n) static ctype_fn sys_##n = n
define_sys(isalnum);
define_sys(isalpha);
define_sys(isascii);
define_sys(isblank);
define_sys(iscntrl);
define_sys(isdigit);
define_sys(isgraph);
define_sys(islower);
define_sys(isprint);
define_sys(ispunct);
define_sys(isspace);
define_sys(isupper);
define_sys(isxdigit);
define_sys(tolower);
define_sys(toupper);

/* Now import the Fuzix ones. */

#undef isalnum
#define isalnum fuzix_isalnum

#undef isalpha
#define isalpha fuzix_isalpha

#undef isascii
#define isascii fuzix_isascii

#undef isblank
#define isblank fuzix_isblank

#undef iscntrl
#define iscntrl fuzix_iscntrl

#undef isdigit
#define isdigit fuzix_isdigit

#undef isgraph
#define isgraph fuzix_isgraph

#undef islower
#define islower fuzix_islower

#undef isprint
#define isprint fuzix_isprint

#undef ispunct
#define ispunct fuzix_ispunct

#undef isspace
#define isspace fuzix_isspace

#undef isupper
#define isupper fuzix_isupper

#undef isxdigit
#define isxdigit fuzix_isxdigit

#undef tolower
#define tolower fuzix_tolower

#undef toupper
#define toupper fuzix_toupper

#undef _tolower
#undef _toupper

#undef __CTYPE_H
#include "../include/ctype.h"

static bool failed = false;

static void test(int i)
{
	#define TEST(n) \
		if (!!sys_##n(i) != !!fuzix_##n(i)) \
			printf("FAIL sys_%s(%d)=%d; fuzix_%s(%d)=%d\n", \
				#n, i, sys_##n(i), #n, i, fuzix_##n(i))

	TEST(isalnum);
	TEST(isalpha);
	TEST(isascii);
	TEST(isblank);
	TEST(iscntrl);
	TEST(isdigit);
	TEST(isgraph);
	TEST(islower);
	TEST(isprint);
	TEST(ispunct);
	TEST(isspace);
	TEST(isupper);
	TEST(isxdigit);

	if (sys_tolower(i) != fuzix_tolower(i))
		printf("FAIL sys_tolower(%d)=%d; fuzix_tolower(%d)=%d\n",
			i, sys_tolower(i), i, fuzix_tolower(i));
	if (sys_toupper(i) != fuzix_toupper(i))
		printf("FAIL sys_toupper(%d)=%d; fuzix_toupper(%d)=%d\n",
			i, sys_toupper(i), i, fuzix_toupper(i));
}

int main(int argc, const char* argv[])
{
	int i;

	for (i=0; i<=255; i++)
		test(i);
	test(EOF);

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

