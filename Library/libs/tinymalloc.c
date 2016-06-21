/*
 *	A tiny stub malloc for those cases that can use it
 *
 *	- malloc is fully implemented
 *	- free only applies in reverse order
 *	- realloc only applies to last malloc
 *
 *	But if you can meet those rules it's really quite efficient 8)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static intptr_t *last;

static void check(void *p)
{
	if (p != (void *)(last - 1)) {
		write(2, "tinymalloc: unsupported.\n", 26);
		exit(1);
	}
}

static uint8_t dobrk(uint8_t *p, size_t n)
{
	if (n + 7 < n)
		return 0;
	n = (n + 7) & ~7;
	/* Overflow catch */
	if (p + n < p || brk(p + n))
		return 0;
	return 1;
}

void *malloc(size_t n)
{
	uint8_t *p;


	/* Here be dragons: sbrk takes a signed value. We can however in C malloc
	   a size_t: We also assume nobody else misaligns the brk boundary, we won't
	   fix it if so - maybe we should ? */

	/* Current end of memory */
	p = sbrk(0);
	if (p == (uint8_t *) -1 || !dobrk(p, n))
		return NULL;
	/* Fake it as a used block and free it into the free list */
	*(intptr_t *)p = (intptr_t)last;
	last = (intptr_t *)p;
	return p + 1;
}

void free(void *p)
{
	intptr_t n;

	check(p);
	n = last[-1];
	brk(last);
	last = (intptr_t *)n;
}

void *realloc(void *pv, size_t n)
{
	uint8_t *p;
	check(pv);

	p = (uint8_t *)last[-1];
	if (!dobrk(p, n))
		return NULL;
	return pv;
}
