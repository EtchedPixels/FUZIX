#define dummy /*
# fforth © 2015 David Given
# This program is available under the terms of the 2-clause BSD license.
# The full text is available here: http://opensource.org/licenses/BSD-2-Clause
#
# fforth is a small Forth written in portable C. It should Just Compile on
# most Unixy platforms. It's intended as a scripting language for the Fuzix
# operating system.
#
# It's probably a bit archaic --- I've been using the Forth 83 doc as a
# reference: http://forth.sourceforge.net/standard/fst83/fst83-16.htm
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
		printf("%-32s { %-14s %-10s %-14s %s }; //@W\n",
			"static cdefn_t " \$3 " =",
			\$6, \$7, \$8, \$9)
		next
	}

	/\/\/@E$/ {
		printf("static cdefn_t* latest = " lastword "; //@E\n")
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
#define ALLOCATION_MARGIN 16
#define CELL sizeof(cell_t)

#define DSTACKSIZE 64
static cell_t dstack[DSTACKSIZE];
static cell_t* dsp;

#define RSTACKSIZE 16
static cell_t rstack[RSTACKSIZE];
static cell_t* rsp;

static char pad[MAX_LINE_LENGTH];
static char tib[MAX_LINE_LENGTH];
static cell_t tib_h = MAX_LINE_LENGTH;
static cell_t tibo = 0;
static cell_t base = 10;

static const defn_t** pc;
static const defn_t* latest;

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

static inline void* alignup(void* ptr)
{
	return (void*)(((cell_t)ptr + sizeof(cell_t)-1) & ~sizeof(cell_t));
}

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

static void* claim_workspace(size_t length)
{
	uint8_t* p = here;
	here += length;

	if (here > (here_top-ALLOCATION_MARGIN))
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
	cdefn_t* current = latest;
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

static void indvarword(cdefn_t* w)
{
	cell_t* p = (void*) dpop();
	dpush(*p);
}

static void varword(cdefn_t* w)
{
	dpush((cell_t) &w->payload);
}

/* Forward declarations of words go here --- do not edit.*/
//@EXPORT{
static cdefn_t accept_word;
static cdefn_t add_word;
static cdefn_t allot_word;
static cdefn_t at_word;
static cdefn_t base_word;
static cdefn_t branch_word;
static cdefn_t branchif_word;
static cdefn_t bye_word;
static cdefn_t c_at_word;
static cdefn_t c_comma_word;
static cdefn_t c_pling_word;
static cdefn_t cell_word;
static cdefn_t cells_word;
static cdefn_t comma_word;
static cdefn_t div_word;
static cdefn_t dot_quote_rword;
static cdefn_t emit_word;
static cdefn_t execute_word;
static cdefn_t exit_word;
static cdefn_t fputc_word;
static cdefn_t fwrite_word;
static cdefn_t here_word;
static cdefn_t in_a_word;
static cdefn_t latest_word;
static cdefn_t lit_word;
static cdefn_t m_one_word;
static cdefn_t mul_word;
static cdefn_t one_word;
static cdefn_t pad_word;
static cdefn_t pling_word;
static cdefn_t pop_word;
static cdefn_t quit_word;
static cdefn_t return_word;
static cdefn_t rsp0_word;
static cdefn_t rsp_at_word;
static cdefn_t rsp_pling_word;
static cdefn_t sp0_word;
static cdefn_t sp_at_word;
static cdefn_t sp_pling_word;
static cdefn_t stderr_word;
static cdefn_t stdin_word;
static cdefn_t stdout_word;
static cdefn_t sub_word;
static cdefn_t tib_h_word;
static cdefn_t tib_word;
static cdefn_t type_word;
static cdefn_t zero_word;
//@EXPORT}

static void fwrite_cb(cdefn_t* w)
{
	FILE* fp = (FILE*)dpop();
	size_t n = dpop();
	void* ptr = (void*)dpop();
	dpush(fwrite(ptr, 1, n, fp));
}

static void accept_cb(cdefn_t* w)
{
	cell_t max = dpop();
	uint8_t* addr = (uint8_t*)dpop();

	cell_t i = 0;
	while (i < max)
	{
		int c = fgetc(stdin);
		if ((c == -1) || (c == '\n'))
		{
			*addr = '\n';
			i++;
			break;
		}
		*addr = c;
		i++;
	}
	dpush(i);
}

static void dot_quote_rcb(cdefn_t* w)
{
	uint8_t* ptr = (void*)pc;
	size_t len = strlen((char*)ptr);
	fwrite(ptr, 1, len, stdout);
	pc = alignup(ptr+len+1);
}

static void add_cb(cdefn_t* w)      { dpush(dpop() + dpop()); }
static void allot_cb(cdefn_t* w)    { claim_workspace(dpop()); }
static void at_cb(cdefn_t* w)       { dpush(*(cell_t*)dpop()); }
static void c_at_cb(cdefn_t* w)     { dpush(*(uint8_t*)dpop()); }
static void c_pling_cb(cdefn_t* w)  { uint8_t* p = (uint8_t*)dpop(); *p = dpop(); }
static void div_cb(cdefn_t* w)      { cell_t a = dpop(); cell_t b = dpop(); dpush(b / a); }
static void execute_cb(cdefn_t* w)  { cdefn_t* p = (void*) dpop(); codeword(p); }
static void exit_cb(cdefn_t* w)     { exit(dpop()); }
static void fputc_cb(cdefn_t* w)    { FILE* fp = (FILE*)dpop(); fputc(dpop(), fp); }
static void here_cb(cdefn_t* w)     { dpush((cell_t) here); }
static void lit_cb(cdefn_t* w)      { dpush((cell_t) *pc++); }
static void mul_cb(cdefn_t* w)      { dpush(dpop() * dpop()); }
static void pad_cb(cdefn_t* w)      { dpush((cell_t) pad); }
static void pling_cb(cdefn_t* w)    { cell_t* p = (cell_t*)dpop(); *p = dpop(); }
static void pop_cb(cdefn_t* w)      { dpop(); }
static void return_cb(cdefn_t* w)   { pc = (void*) rpop(); }
static void sub_cb(cdefn_t* w)      { cell_t a = dpop(); cell_t b = dpop(); dpush(b - a); }
static void branchif_cb(cdefn_t* w) { void* iftrue = (void*) *pc++; if (dpop() == (cell_t)w->payload) pc = iftrue; }
static void branch_cb(cdefn_t* w)   { pc = (void*) *pc; }
static void sp_pling_cb(cdefn_t* w) { dsp = (void*)dpop(); }
static void rsp_pling_cb(cdefn_t* w) { rsp = (void*)dpop(); }

static cdefn_t* bye_ops[] =     { &zero_word, &exit_word };
static cdefn_t* c_comma_ops[] = { &here_word, &c_pling_word, &one_word, &allot_word, &exit_word };
static cdefn_t* cells_ops[] =   { &cell_word, &mul_word, &exit_word };
static cdefn_t* comma_ops[] =   { &here_word, &pling_word, &cell_word, &allot_word, &exit_word };
static cdefn_t* emit_ops[] =    { &stdout_word, &fputc_word, &exit_word };
static cdefn_t* type_ops[] =    { &stdout_word, &fwrite_word, &pop_word, &exit_word };

static cdefn_t* quit_ops[] =
{
	/* Reset stacks. */
	&sp0_word, &sp_pling_word,
	&rsp0_word, &rsp_pling_word,
	/* Read a line from the terminal. */
	&tib_word, &tib_h_word, &accept_word,
	/* And go round again */
	&branch_word, (void*)quit_ops
};

/* List of words go here. To add a word, add a new entry and run this file as
 * a shell script. The link field will be set correctly. 
 * BEWARE: these lines are parsed using whitespace. LEAVE EXACTLY AS IS.*/
static cdefn_t accept_word =     { accept_cb,     "accept",  NULL,          NULL }; //@W
static cdefn_t add_word =        { add_cb,        "+",       &accept_word,  NULL }; //@W
static cdefn_t allot_word =      { allot_cb,      "allot",   &add_word,     NULL, }; //@W
static cdefn_t at_word =         { at_cb,         "@",       &allot_word,   NULL }; //@W
static cdefn_t base_word =       { sysvarword,    "base",    &at_word,      &base }; //@W
static cdefn_t branch_word =     { branch_cb,     NULL,      &base_word,    (void*)0 }; //@W
static cdefn_t branchif_word =   { branchif_cb,   NULL,      &branch_word,  (void*)0 }; //@W
static cdefn_t bye_word =        { codeword,      "bye",     &branchif_word, bye_ops, }; //@W
static cdefn_t c_at_word =       { c_at_cb,       "c@",      &bye_word,     NULL }; //@W
static cdefn_t c_comma_word =    { codeword,      "c,",      &c_at_word,    c_comma_ops, }; //@W
static cdefn_t c_pling_word =    { c_pling_cb,    "c!",      &c_comma_word, NULL }; //@W
static cdefn_t cell_word =       { sysvarword,    "cell",    &c_pling_word, (void*)CELL, }; //@W
static cdefn_t cells_word =      { codeword,      "cells",   &cell_word,    cells_ops, }; //@W
static cdefn_t comma_word =      { codeword,      ",",       &cells_word,   comma_ops, }; //@W
static cdefn_t div_word =        { div_cb,        "/",       &comma_word,   NULL }; //@W
static cdefn_t dot_quote_rword = { dot_quote_rcb, NULL,      &div_word,     NULL }; //@W
static cdefn_t emit_word =       { codeword,      "emit",    &dot_quote_rword, emit_ops }; //@W
static cdefn_t execute_word =    { execute_cb,    "execute", &emit_word,    NULL }; //@W
static cdefn_t exit_word =       { exit_cb,       "_exit",   &execute_word, NULL }; //@W
static cdefn_t fputc_word =      { fputc_cb,      "_fwrite", &exit_word,    NULL }; //@W
static cdefn_t fwrite_word =     { fwrite_cb,     "_fwrite", &fputc_word,   NULL }; //@W
static cdefn_t here_word =       { here_cb,       "here",    &fwrite_word,  NULL, }; //@W
static cdefn_t in_a_word =       { sysvarword,    ">in",     &here_word,    &tibo }; //@W
static cdefn_t latest_word =     { indvarword,    "latest",  &in_a_word,    &latest, }; //@W
static cdefn_t lit_word =        { lit_cb,        NULL,      &latest_word,  NULL, }; //@W
static cdefn_t m_one_word =      { sysvarword,    "-1",      &lit_word,     (void*)-1, }; //@W
static cdefn_t mul_word =        { mul_cb,        "*",       &m_one_word,   NULL }; //@W
static cdefn_t one_word =        { sysvarword,    "1",       &mul_word,     (void*)1, }; //@W
static cdefn_t pad_word =        { pad_cb,        "pad",     &one_word,     NULL }; //@W
static cdefn_t pling_word =      { pling_cb,      "!",       &pad_word,     NULL }; //@W
static cdefn_t pop_word =        { pop_cb,        "pop",     &pling_word,   NULL }; //@W
static cdefn_t quit_word =       { codeword,      NULL,      &pop_word,     quit_ops }; //@W
static cdefn_t return_word =     { return_cb,     ";",       &quit_word,    NULL, }; //@W
static cdefn_t rsp0_word =       { sysvarword,    "rsp0",    &return_word,  rstack }; //@W
static cdefn_t rsp_at_word =     { indvarword,    "rsp@",    &rsp0_word,    &rsp }; //@W
static cdefn_t rsp_pling_word =  { sp_pling_cb,   "rsp!",    &rsp_at_word,  NULL }; //@W
static cdefn_t sp0_word =        { sysvarword,    "sp0",     &rsp_pling_word, dstack }; //@W
static cdefn_t sp_at_word =      { indvarword,    "sp@",     &sp0_word,     &dsp }; //@W
static cdefn_t sp_pling_word =   { sp_pling_cb,   "sp!",     &sp_at_word,   NULL }; //@W
static cdefn_t stderr_word =     { indvarword,    "_stderr", &sp_pling_word, &stderr, }; //@W
static cdefn_t stdin_word =      { indvarword,    "_stdin",  &stderr_word,  &stdin, }; //@W
static cdefn_t stdout_word =     { indvarword,    "_stdout", &stdin_word,   &stdout, }; //@W
static cdefn_t sub_word =        { sub_cb,        "-",       &stdout_word,  NULL }; //@W
static cdefn_t tib_h_word =      { sysvarword,    "tib#",    &sub_word,     &tib_h }; //@W
static cdefn_t tib_word =        { sysvarword,    "tib",     &tib_h_word,   tib }; //@W
static cdefn_t type_word =       { codeword,      "emit",    &tib_word,     type_ops }; //@W
static cdefn_t zero_word =       { sysvarword,    "0",       &type_word,    (void*)0, }; //@W

static cdefn_t* latest = &zero_word; //@E

int main(int argc, const char* argv[])
{
	here = here_top = sbrk(0);
	claim_workspace(0);

	setjmp(onerror);
	dsp = dstack;
	rsp = rstack;

	pc = (void*) quit_word.payload;
	for (;;)
	{
		const struct definition* w = (void*) *pc++;
		w->code(w);
	}
}

