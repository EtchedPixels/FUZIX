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

/* ATEXIT.H */
#define MAXONEXIT 10		/* AIUI Posix requires 10 */

typedef void (*vfuncp) (int, void *);

extern struct exit_table {
	onexit_t called;
	void *argument;
} __on_exit_table[MAXONEXIT];

extern int __on_exit_count;

/* End ATEXIT.H */
int __on_exit_count = 0;
struct exit_table __on_exit_table[MAXONEXIT];

void __do_exit(int rv)
{
	/* Static to work around a bug in cc65 */
	static int count;
	vfuncp ptr;

	count = __on_exit_count - 1;
	__on_exit_count = -1;	/* ensure no more will be added */

	/* In reverse order */
	while (count >= 0) {
		ptr = (vfuncp) __on_exit_table[count].called;
		(*ptr) (rv, __on_exit_table[count].argument);
		--count;
	}
}

int on_exit(onexit_t ptr, void *arg)
{
	if (__on_exit_count < 0 || __on_exit_count >= MAXONEXIT) {
		errno = ENOMEM;
		return -1;
	}
	if (ptr) {
		__on_exit_table[__on_exit_count].called = ptr;
		__on_exit_table[__on_exit_count].argument = arg;
		__on_exit_count++;
	}
	return 0;
}

int atexit(atexit_t ptr)
{
	return on_exit((onexit_t) ptr, 0);
}
