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
#include <string.h>
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

#define __TESTING__

#undef isalnum
#define isalnum fuzix_isalnum
#include "../libs/isalnum.c"

#undef isalpha
#define isalpha fuzix_isalpha
#include "../libs/isalpha.c"

#undef isascii
#define isascii fuzix_isascii
#include "../libs/isascii.c"

#undef isblank
#define isblank fuzix_isblank
#include "../libs/isblank.c"

#undef iscntrl
#define iscntrl fuzix_iscntrl
#include "../libs/iscntrl.c"

#undef isdigit
#define isdigit fuzix_isdigit
#include "../libs/isdigit.c"

#undef isgraph
#define isgraph fuzix_isgraph
#include "../libs/isgraph.c"

#undef islower
#define islower fuzix_islower
#include "../libs/islower.c"

#undef isprint
#define isprint fuzix_isprint
#include "../libs/isprint.c"

#undef ispunct
#define ispunct fuzix_ispunct
#include "../libs/ispunct.c"

#undef isspace
#define isspace fuzix_isspace
#include "../libs/isspace.c"

#undef isupper
#define isupper fuzix_isupper
#include "../libs/isupper.c"

#undef isxdigit
#define isxdigit fuzix_isxdigit
#include "../libs/isxdigit.c"

#undef tolower
#define tolower fuzix_tolower
#include "../libs/tolower.c"

#undef toupper
#define toupper fuzix_toupper
#include "../libs/toupper.c"

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

	if (i != EOF)
	{
		if (sys_tolower(i) != fuzix_tolower(i))
			printf("FAIL sys_tolower(%d)=%d; fuzix_tolower(%d)=%d\n",
				i, sys_tolower(i), i, fuzix_tolower(i));
		if (sys_toupper(i) != fuzix_toupper(i))
			printf("FAIL sys_toupper(%d)=%d; fuzix_toupper(%d)=%d\n",
				i, sys_toupper(i), i, fuzix_toupper(i));
	}
}

int main(int argc, const char* argv[])
{
	int i;

	for (i=0; i<=255; i++)
		test(i);
	test(EOF);

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

