#define dummy /*
# fforth © 2015 David Given
# This program is available under the terms of the 2-clause BSD license.
# The full text is available here: http://opensource.org/licenses/BSD-2-Clause
#
# fforth is a small Forth written in portable C. It should Just Compile on
# most Unixy platforms. It's intended as a scripting language for the Fuzix
# operating system.
#
# Note! This program looks weird. That's because it's a shell script *and* a C
# file. (And an Awk script.) However, it's necessary in order to dynamically
# generate the word list, which is just too fragile to do by hand. If you edit
# a line marked with a //@W, then just run this file, as a shell script, and
# it'll rebuild all the fiddly links in the list.
#
# No evil was harmed in the making of this file. Probably.

set -e
trap 'rm /tmp/$$.words' EXIT

# Get the list of words (for forward declaration).
awk -f- $0 >/tmp/$$.words <<EOF
	/\/\/@W$/ {
		print("static cdefn_t " \$3 ";")
	}
EOF

# Now actually edit the source file.
awk -f- $0 >$0.new <<EOF
	BEGIN {
		lastword = "NULL"
	}

	/\/\/@EXPORT}\$/ {
		print "//@EXPORT{"
		while ((getline line < "/tmp/$$.words") > 0)
			print "" line
		close("/tmp/$$.words")
		print "//@EXPORT}"
	}
	/\/\/@EXPORT{\$/, /\/\/@EXPORT}\$/ { next }


	/\/\/@W\$/ {
		\$8 = lastword ","
		lastword = "&" \$3
		printf("%-29s { %-11s %-10s %-14s %s }; //@W\n",
			"static cdefn_t " \$3 " =",
			\$6, \$7, \$8, \$9)
		next
	}

	/\/\/@E$/ {
		printf("static cdefn_t* currentword = " lastword "; //@E\n")
		next
	}

	{
		print
	}
EOF

# Replace the old file with the new.

mv $0 $0.old
mv $0.new $0

echo "Updated!"

exit 0
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>

typedef intptr_t cell_t;
typedef struct definition defn_t;
typedef const struct definition cdefn_t;

static jmp_buf onerror;

#define MAX_LINE_LENGTH 160
#define ALLOCATION_CHUNK_SIZE 1024
#define CELL sizeof(cell_t)

#define DSTACKSIZE 64
static cell_t dstack[DSTACKSIZE];
static cell_t* dsp;

#define RSTACKSIZE 16
static cell_t rstack[RSTACKSIZE];
static cell_t* rsp;

static char tib[MAX_LINE_LENGTH];

static const defn_t* const* pc;
static const defn_t* currentword;

typedef void code_fn(const defn_t* w);

#define NAMELEN 8
struct definition
{
	code_fn* code;
	const char* name;
	cdefn_t* next;
	void* payload;
	bool immediate : 1;
};

static uint8_t* here;
static uint8_t* here_top;

static void panic(const char* message)
{
	fputs("panic: ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
	longjmp(onerror, 0);
}

static void dpush(cell_t val)
{
	if (dsp == &dstack[DSTACKSIZE])
		panic("data stack overflow");
	*dsp++ = val;
}

static cell_t dpop(void)
{
	if (dsp == &dstack[0])
		panic("data stack underflow");
	return *--dsp;
}

static void rpush(cell_t val)
{
	if (rsp == &rstack[RSTACKSIZE])
		panic("return stack overflow");
	*rsp++ = val;
}

static cell_t rpop(void)
{
	if (dsp == &dstack[0])
		panic("return stack underflow");
	return *--dsp;
}

static uint8_t* claim_workspace(size_t length)
{
	uint8_t* p = here;
	here += length;
	if (here > here_top)
	{
		uint8_t* newtop = sbrk(ALLOCATION_CHUNK_SIZE);
		if (newtop != here_top)
			panic("non-contiguous sbrk memory");
		here_top = newtop + ALLOCATION_CHUNK_SIZE;
	}
	return p;
}

static cdefn_t* lookup_word(const char* name)
{
	cdefn_t* current = currentword;
	while (current)
	{
		if (current->name
				&& (strcmp(current->name, name) == 0))
			return current;
		current = current->next;
	}
	return NULL;
}

static void codeword(cdefn_t* w)
{
	rpush((cell_t) pc);
	pc = (void*) w->payload;
}

static void sysvarword(cdefn_t* w)
{
	dpush((cell_t) w->payload);
}

static void varword(cdefn_t* w)
{
	dpush((cell_t) &w->payload);
}

/* Forward declarations of words go here --- do not edit.*/
//@EXPORT{
static cdefn_t allot_word;
static cdefn_t bye_word;
static cdefn_t c_comma_word;
static cdefn_t cells_word;
static cdefn_t comma_word;
static cdefn_t exit_word;
static cdefn_t here_word;
static cdefn_t lit_word;
static cdefn_t return_word;
static cdefn_t tib_word;
//@EXPORT}

static void allot_cb(cdefn_t* w)   { claim_workspace(dpop()); }
static void c_comma_cb(cdefn_t* w) { *claim_workspace(1) = dpop(); }
static void cells_cb(cdefn_t* w)   { dpush(dpop() * CELL); }
static void comma_cb(cdefn_t* w)   { *(cell_t*)claim_workspace(CELL) = dpop(); }
static void exit_cb(cdefn_t* w)    { exit(dpop()); }
static void here_cb(cdefn_t* w)    { dpush((cell_t) here); }
static void lit_cb(cdefn_t* w)     { dpush((cell_t) *pc++); }
static void return_cb(cdefn_t* w)  { pc = (void*) rpop(); }
static void tib_cb(cdefn_t* w)     { dpush((cell_t) tib); }

static cdefn_t* bye_ops[] = {
	&lit_word,
	(void*) 0,
	&exit_word
};

/* List of words go here. To add a word, add a new entry and run this file as
 * a shell script. The link field will be set correctly. */
static cdefn_t allot_word =   { allot_cb,   "allot",   NULL,          NULL, }; //@W
static cdefn_t bye_word =     { codeword,   "bye",     &allot_word,   bye_ops, }; //@W
static cdefn_t c_comma_word = { c_comma_cb, "c,",      &bye_word,     NULL, }; //@W
static cdefn_t cells_word =   { cells_cb,   "cells",   &c_comma_word, NULL, }; //@W
static cdefn_t comma_word =   { comma_cb,   ",",       &cells_word,   NULL, }; //@W
static cdefn_t exit_word =    { exit_cb,    "_exit",   &comma_word,   NULL, }; //@W
static cdefn_t here_word =    { here_cb,    "here",    &exit_word,    NULL, }; //@W
static cdefn_t lit_word =     { lit_cb,     "",        &here_word,    NULL, }; //@W
static cdefn_t return_word =  { return_cb,  ";",       &lit_word,     NULL, }; //@W
static cdefn_t tib_word =     { sysvarword, "tib",     &return_word,  &tib, }; //@W

static cdefn_t* currentword = &tib_word; //@E

int main(int argc, const char* argv[])
{
	here = here_top = sbrk(0);

	setjmp(onerror);
	dsp = dstack;
	rsp = rstack;

	pc = (void*) bye_word.payload;
	for (;;)
	{
		const struct definition* w = (void*) *pc++;
		w->code(w);
	}
}

