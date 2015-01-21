/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
/*
 * This deals with both the atexit and on_exit function calls
 *
 * Note: calls installed with atexit are called with the same args as
 * on_exit fuctions; the void* is given the NULL value.
 */

#include <stdlib.h>
#include <errno.h>

#define MAXATEXIT 10		/* AIUI Posix requires 10 */

typedef void (*vfuncp) (void);

struct exit_table {
	atexit_t called;
} __atexit_table[MAXATEXIT];

extern int __atexit_count;

int __atexit_count = 0;

void __do_exit(int rv)
{
	int count;
	vfuncp ptr;

	count = __atexit_count - 1;
	__atexit_count = -1;	/* ensure no more will be added */

	/* In reverse order */
	while (count >= 0) {
		ptr = (vfuncp) __atexit_table[count].called;
		(*ptr) ();
		--count;
	}
}

int atexit(atexit_t ptr)
{
	if (__atexit_count < 0 || __atexit_count >= MAXATEXIT) {
		errno = ENOMEM;
		return -1;
	}
	if (ptr) {
		__atexit_table[__atexit_count].called = ptr;
		__atexit_count++;
	}
	return 0;
}
