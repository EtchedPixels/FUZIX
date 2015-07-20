#define dummy /*
# fforth © 2015 David Given
# This program is available under the terms of the 2-clause BSD license.
# The full text is available here: http://opensource.org/licenses/BSD-2-Clause
#
# fforth is a small Forth written in portable C. It should Just Compile on
# most Unixy platforms. It's intended as a scripting language for the Fuzix
# operating system.
#
# It's probably a bit weird --- I'm using the ANS Forth reference here:
# http://lars.nocrew.org/dpans/dpans6.htm
# ...but I've been playing fast and loose with the standard.
#
# Peculiarities include:
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
		n = \$2
		sub(/,/, " ", n)
		print("static cdefn_t " n ";")
	}
EOF

# Now actually edit the source file.
awk -f- $0 > $0.new <<EOF
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
		\$5 = lastword ","

		printf("%s %-19s %-15s %-13s %-17s",
			\$1, \$2, \$3, \$4, \$5)

		payload = ""
		for (i=6; i<=NF; i++)
			printf(" %s", \$i)
		printf("\n")

		lastword = "&" \$2
		sub(/,/, "", lastword)

		next
	}

	/\/\/@E$/ {
		n = \$2
		sub(/,/, " ", n)
		printf("static " \$2 " " \$3 " = (defn_t*) " lastword "; //@E\n")
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
#define ALLOCATION_CHUNK_SIZE 128
#define CELL sizeof(cell_t)

#define DSTACKSIZE 64
static cell_t dstack[DSTACKSIZE];
static cell_t* dsp;

#define RSTACKSIZE 16
static cell_t rstack[RSTACKSIZE];
static cell_t* rsp;

static FILE* input_fp;
static char input_buffer[MAX_LINE_LENGTH];
static cell_t in_arrow;
static cell_t base = 10;
static cell_t state = false;

static defn_t** pc;
static defn_t* latest; /* Most recent word on dictionary */
static cdefn_t* last;   /* Last of the built-in words */

static uint8_t* here;
static uint8_t* here_top;

typedef void code_fn(cdefn_t* w);
static void align_cb(cdefn_t* w);

#define FL_IMM 0x80

struct fstring
{
	uint8_t len;
	char data[];
};

struct definition
{
	code_fn* code;
	struct fstring* name;
	cdefn_t* next;
	void* payload[];
};

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

#if !defined FAST
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

static cell_t dpeek(int count)
{
	cell_t* ptr = dsp - count;
	if (ptr < dstack)
		panic("data stack underflow");
	return *ptr;
}

static void rpush(cell_t val)
{
	if (rsp == &rstack[RSTACKSIZE])
		panic("return stack overflow");
	*rsp++ = val;
}

static cell_t rpop(void)
{
	if (rsp == &rstack[0])
		panic("return stack underflow");
	return *--rsp;
}
#else
static inline void dpush(cell_t val) { *dsp++ = val; }
static inline cell_t dpop(void) { return *--dsp; }
static inline cell_t dpeek(int count) { return dsp[-count]; }
static inline void rpush(cell_t val) { *rsp++ = val; }
static inline cell_t rpop(void) { return *--rsp; }
#endif

static void* ensure_workspace(size_t length)
{
	uint8_t* p = here + length;

	if (p > (here_top))
	{
		uint8_t* newtop = sbrk(ALLOCATION_CHUNK_SIZE);
		if (newtop != here_top)
			panic("non-contiguous sbrk memory");
		here_top = newtop + ALLOCATION_CHUNK_SIZE;
	}

	return here;
}

static void* claim_workspace(size_t length)
{
	uint8_t* p = ensure_workspace(length);
	here += length;
	return p;
}

/* Note --- this only works properly on word names, not general counted
 * strings, because it ignores the top bit of the length (used in the
 * dictionary as a flag). */
static int fstreq(const struct fstring* f1, const struct fstring* f2)
{
	int len1 = f1->len & 0x7f;
	int len2 = f2->len & 0x7f;
	if (len1 != len2)
		return 0;
	return (memcmp(f1->data, f2->data, len1) == 0);
}

static void fstrout(const struct fstring* f)
{
	fwrite(f->data, 1, f->len & 0x7f, stdout);
}

/* Forward declarations of words go here --- do not edit.*/
//@EXPORT{
static cdefn_t E_fnf_word ;
static cdefn_t E_undef_word ;
static cdefn_t _create_word ;
static cdefn_t _exit_word ;
static cdefn_t _fclose_word ;
static cdefn_t _feof_word ;
static cdefn_t _fopen_word ;
static cdefn_t _fputc_word ;
static cdefn_t _fread_word ;
static cdefn_t _fwrite_word ;
static cdefn_t _input_fp_word ;
static cdefn_t _stderr_word ;
static cdefn_t _stdin_word ;
static cdefn_t _stdout_word ;
static cdefn_t a_number_word ;
static cdefn_t accept_word ;
static cdefn_t add_one_word ;
static cdefn_t add_word ;
static cdefn_t align_word ;
static cdefn_t allot_word ;
static cdefn_t and_word ;
static cdefn_t at_word ;
static cdefn_t base_word ;
static cdefn_t branch0_word ;
static cdefn_t branchnot0_word ;
static cdefn_t branch_word ;
static cdefn_t bye_word ;
static cdefn_t c_at_word ;
static cdefn_t c_comma_word ;
static cdefn_t c_pling_word ;
static cdefn_t cell_word ;
static cdefn_t cells_word ;
static cdefn_t close_sq_word ;
static cdefn_t colon_word ;
static cdefn_t comma_word ;
static cdefn_t compile_num_word ;
static cdefn_t constant_word ;
static cdefn_t create_word ;
static cdefn_t decimal_word ;
static cdefn_t div_word ;
static cdefn_t dot_quote_rword ;
static cdefn_t dot_word ;
static cdefn_t dup_word ;
static cdefn_t emit_word ;
static cdefn_t equals_word ;
static cdefn_t execute_word ;
static cdefn_t exit_word ;
static cdefn_t fill_word ;
static cdefn_t find_word ;
static cdefn_t here_word ;
static cdefn_t hex_word ;
static cdefn_t in_arrow_word ;
static cdefn_t interpret_num_word ;
static cdefn_t interpret_word ;
static cdefn_t latest_word ;
static cdefn_t lit_word ;
static cdefn_t m_one_word ;
static cdefn_t mul_word ;
static cdefn_t not_equals_word ;
static cdefn_t one_word ;
static cdefn_t or_word ;
static cdefn_t over_word ;
static cdefn_t pad_word ;
static cdefn_t pling_word ;
static cdefn_t pop_word ;
static cdefn_t quit_word ;
static cdefn_t read_file_word ;
static cdefn_t refill_word ;
static cdefn_t rot_word ;
static cdefn_t rsp0_word ;
static cdefn_t rsp_at_word ;
static cdefn_t rsp_pling_word ;
static cdefn_t skip0_word ;
static cdefn_t skipifi_word ;
static cdefn_t skipnot0_word ;
static cdefn_t skipnotifi_word ;
static cdefn_t source_word ;
static cdefn_t sp0_word ;
static cdefn_t sp_at_word ;
static cdefn_t sp_pling_word ;
static cdefn_t state_word ;
static cdefn_t sub_one_word ;
static cdefn_t sub_word ;
static cdefn_t swap_word ;
static cdefn_t two_word ;
static cdefn_t type_word ;
static cdefn_t u_dot_word ;
static cdefn_t variable_word ;
static cdefn_t word_word ;
static cdefn_t zero_word ;
static cdefn_t immediate_word ;
static cdefn_t open_sq_word ;
static cdefn_t semicolon_word ;
//@EXPORT}

/* ======================================================================= */
/*                                  WORDS                                  */
/* ======================================================================= */

static void icodeword(cdefn_t* w) { rpush((cell_t) pc); pc = w->payload[0]; }
static void codeword(cdefn_t* w) { rpush((cell_t) pc); pc = (void*) &w->payload[0]; }
static void dataword(cdefn_t* w) { dpush((cell_t) &w->payload[0]); }
static void rvarword(cdefn_t* w) { dpush((cell_t) w->payload[0]); }
static void r2varword(cdefn_t* w) { dpush((cell_t) w->payload[0]); dpush((cell_t) w->payload[1]); }
static void wvarword(defn_t* w) { w->payload[0] = (void*) dpop(); }
static void rivarword(cdefn_t* w) { dpush(*(cell_t*) w->payload[0]); }
static void wivarword(cdefn_t* w) { *(cell_t*)(w->payload[0]) = dpop(); }

static void _freadwrite_cb(cdefn_t* w)
{
	FILE* fp = (FILE*)dpop();
	size_t len = dpop();
	void* ptr = (void*)dpop();
	int (*func)(void* ptr, size_t size, size_t nmemb, FILE* stream) = (void*) *w->payload;

	dpush(func(ptr, 1, len, fp));
}

static void _fopen_cb(cdefn_t* w)
{
	const char* mode = (void*)dpop();
	const char* filename = (void*)dpop();
	dpush((cell_t) fopen(filename, mode));
}

static void accept_cb(cdefn_t* w)
{
	cell_t max = dpop();
	char* addr = (char*)dpop();
	int len = 0;

	if (fgets(addr, max, input_fp))
	{
		len = strlen(addr);
		if ((len > 0) && (addr[len-1] == '\n'))
			len--;
	}
	dpush(len);
}

static void fill_cb(cdefn_t* w)
{
	cell_t c = dpop();
	cell_t len = dpop();
	void* ptr = (void*) dpop();

	memset(ptr, c, len);
}

static void dot_quote_rcb(cdefn_t* w)
{
	uint8_t* ptr = (void*)pc;
	size_t len = strlen((char*)ptr);
	fwrite(ptr, 1, len, stdout);
	pc = alignup(ptr+len+1);
}

static void immediate_cb(cdefn_t* w)
{
	latest->name->len |= FL_IMM;
}

static void word_cb(cdefn_t* w)
{
	int delimiter = dpop();
	struct fstring* fs = ensure_workspace(MAX_LINE_LENGTH);
	int count = 0;

	/* Skip whitespace. */
	while (in_arrow < MAX_LINE_LENGTH)
	{
		int c = input_buffer[in_arrow];
		if (c != delimiter)
			break;
		in_arrow++;
	}
	if (in_arrow != MAX_LINE_LENGTH)
	{
		while (in_arrow < MAX_LINE_LENGTH)
		{
			int c = input_buffer[in_arrow];
			if (c == delimiter)
				break;
			fs->data[count] = c;
			count++;
			in_arrow++;
		}
	}

	fs->len = count;
	dpush((cell_t) fs);
}

static void _create_cb(cdefn_t* w)
{
	/* The name of the word is passed on the stack. */

	struct fstring* name = (void*)dpop();

	/* Create the word header. */

	defn_t* defn = claim_workspace(sizeof(defn_t));
	defn->code = dataword;
	defn->name = name;
	defn->next = latest;
	latest = defn;
}

static void find_cb(cdefn_t* w)
{
	struct fstring* name = (void*) dpop();
	cdefn_t* current = latest;
	while (current)
	{
		if (current->name && fstreq(name, current->name))
		{
			dpush((cell_t) current);
			dpush((current->name->len & FL_IMM) ? 1 : -1);
			return;
		}
		current = current->next;
	}

	dpush((cell_t) name);
	dpush(0);
}

static unsigned get_digit(char p)
{
	if (p >= 'a')
		return 10 + p - 'a';
	if (p >= 'A')
		return 10 + p - 'A';
	return p - '0';
}

/* This is Forth's rather complicated number parse utility.
 * ( ud c-addr len -- ud' c-addr' len' )
 * Digits are parsed according to base and added to the accumulator ud.
 * Signs are not supported.
 */
static void a_number_cb(cdefn_t* w)
{
	int len = dpop();
	char* addr = (void*) dpop();
	cell_t ud = dpop();

	while (len > 0)
	{
		int d = get_digit(*addr);
		if (d >= base)
			break;
		ud = (ud * base) + d;

		len--;
		addr++;
	}

	dpush(ud);
	dpush((cell_t) addr);
	dpush(len);
}

static void rot_cb(cdefn_t* w)
{
	cell_t x3 = dpop();
	cell_t x2 = dpop();
	cell_t x1 = dpop();
	dpush(x3);
	dpush(x1);
	dpush(x2);
}

static void swap_cb(cdefn_t* w)
{
	cell_t x2 = dpop();
	cell_t x1 = dpop();
	dpush(x2);
	dpush(x1);
}

static char to_digit(int p)
{
	if (p < 10)
		return '0' + p;
	return 'a' + (p - 10);
}


static void u_dot_cb(cdefn_t* w)
{
	uintptr_t value = dpop();
	char* start = ensure_workspace(16);
	char* ptr = start;

	do {
		cell_t r = value % base;
		value /= base;
		*ptr++ = to_digit(r);
	} while (value > 0);

	while (ptr > start)
		putchar(*--ptr);
}

static void dot_cb(cdefn_t* w)
{
	cell_t value = dpeek(1);
	if (value < 0)
	{
		putchar('-');
		value = -value;
	}
	u_dot_cb(w);
}

static void E_fnf_cb(cdefn_t* w)      { panic("file not found"); }
static void E_undef_cb(cdefn_t* w)    { panic("unrecognised word"); }
static void _exit_cb(cdefn_t* w)      { exit(dpop()); }
static void _fclose_cb(cdefn_t* w)    { dpush(fclose((FILE*) dpop())); }
static void _feof_cb(cdefn_t* w)      { dpush(feof((FILE*) dpop())); }
static void _fputc_cb(cdefn_t* w)     { FILE* fp = (FILE*)dpop(); fputc(dpop(), fp); }
static void add_cb(cdefn_t* w)        { dpush(dpop() + dpop()); }
static void align_cb(cdefn_t* w)      { claim_workspace((CELL - (cell_t)here) & (CELL-1)); }
static void allot_cb(cdefn_t* w)      { claim_workspace(dpop()); }
static void and_cb(cdefn_t* w)        { dpush(dpop() & dpop()); }
static void at_cb(cdefn_t* w)         { dpush(*(cell_t*)dpop()); }
static void branch_cb(cdefn_t* w)     { pc = (void*) *pc; }
static void branchif_cb(cdefn_t* w)   { if (dpop() == (cell_t)*w->payload) pc = (void*)*pc; else pc++; }
static void branchnotif_cb(cdefn_t* w) { if (dpop() != (cell_t)*w->payload) pc = (void*)*pc; else pc++; }
static void c_at_cb(cdefn_t* w)       { dpush(*(uint8_t*)dpop()); }
static void c_pling_cb(cdefn_t* w)    { uint8_t* p = (uint8_t*)dpop(); *p = dpop(); }
static void close_sq_cb(cdefn_t* w)   { state = 1; }
static void div_cb(cdefn_t* w)        { cell_t a = dpop(); cell_t b = dpop(); dpush(b / a); }
static void equals_cb(cdefn_t* w)     { dpush(dpop() == dpop()); }
static void execute_cb(cdefn_t* w)    { cdefn_t* p = (void*) dpop(); p->code(p); }
static void exit_cb(cdefn_t* w)       { pc = (void*)rpop(); }
static void increment_cb(cdefn_t* w)  { dpush(dpop() + (cell_t)*w->payload); }
static void lit_cb(cdefn_t* w)        { dpush((cell_t) *pc++); }
static void mul_cb(cdefn_t* w)        { dpush(dpop() * dpop()); }
static void not_equals_cb(cdefn_t* w) { dpush(dpop() != dpop()); }
static void open_sq_cb(cdefn_t* w)    { state = 0; }
static void or_cb(cdefn_t* w)         { dpush(dpop() | dpop()); }
static void peekcon_cb(cdefn_t* w)    { dpush(dpeek((cell_t) *w->payload)); }
static void pling_cb(cdefn_t* w)      { cell_t* p = (cell_t*)dpop(); *p = dpop(); }
static void pop_cb(cdefn_t* w)        { dpop(); }
static void skipif_cb(cdefn_t* w)     { if (dpop() == (cell_t)*w->payload) pc++; }
static void skipifi_cb(cdefn_t* w)    { if (dpop() == (cell_t)*pc++) pc++; }
static void skipnotif_cb(cdefn_t* w)  { if (dpop() != (cell_t)*w->payload) pc++; }
static void skipnotifi_cb(cdefn_t* w) { if (dpop() != (cell_t)*pc++) pc++; }
static void sub_cb(cdefn_t* w)        { cell_t a = dpop(); cell_t b = dpop(); dpush(b - a); }

static cdefn_t* accept_ops[] =  { &_stdin_word, &_fread_word, &exit_word };
static cdefn_t* bye_ops[] =     { &zero_word, &_exit_word };
static cdefn_t* c_comma_ops[] = { &here_word, &c_pling_word, &one_word, &allot_word, &exit_word };
static cdefn_t* cells_ops[] =   { &cell_word, &mul_word, &exit_word };
static cdefn_t* comma_ops[] =   { &here_word, &pling_word, &cell_word, &allot_word, &exit_word };
static cdefn_t* decimal_ops[] = { &lit_word, (void*)10, &base_word, &pling_word, &exit_word };
static cdefn_t* emit_ops[] =    { &_stdout_word, &_fputc_word, &exit_word };
static cdefn_t* hex_ops[] =     { &lit_word, (void*)16, &base_word, &pling_word, &exit_word };
static cdefn_t* type_ops[] =    { &_stdout_word, &_fwrite_word, &pop_word, &exit_word };
static cdefn_t* variable_ops[] = { &create_word, &cell_word, &allot_word, &exit_word };

static cdefn_t* create_ops[] =
{
	/* Get the word name; this is written to here */
	&lit_word, (void*)' ', &word_word, /* ( addr -- ) */

	/* Advance over it */
	&dup_word, &c_at_word, &add_one_word, &allot_word, /* ( addr -- ) */

	/* Ensure the data pointer is aligned, and then create the word header */
	&align_word, &_create_word, /* ( -- ) */

	&exit_word
};

static cdefn_t* colon_ops[] =
{
	/* Create the word itself. */
	&create_word,

	/* Turn it into a runnable word. */
	&lit_word, (void*)codeword, &latest_word, &pling_word,

	/* Switch to compilation mode. */
	&close_sq_word, &exit_word
};

static cdefn_t* constant_ops[] =
{
	&create_word,
	&lit_word, (void*)rvarword, &latest_word, &pling_word,
	&comma_word, &exit_word
};

static cdefn_t* semicolon_ops[] =
{
	&lit_word, &exit_word, &comma_word,
	&open_sq_word,
	&exit_word
};

/* refill: ( -- flag )
 * Refills the input buffer. */
static cdefn_t* refill_ops[] =
{
	/* Read a line from the terminal. */
	&source_word, &accept_word, /* ( -- len ) */

	/* Is this the end? */
	&dup_word, &skipnot0_word, &exit_word, /* ( -- len ) */

	/* Clear the remainder of the buffer. */
	&dup_word, &lit_word, (void*)input_buffer, &add_word, /* ( -- len addr ) */
	&swap_word, /* ( -- addr len ) */
	&lit_word, (void*)MAX_LINE_LENGTH, &swap_word, &sub_word, /* ( -- addr remaining ) */
	&lit_word, (void*)32, /* ( -- addr remaining char ) */
	&fill_word,

	/* Reset the input pointer. */
	&zero_word, &in_arrow_word, &pling_word,

	/* We will succeed! */
	&one_word, &exit_word
};

static cdefn_t* interpreter_table[] =
{
	// compiling   not found            immediate
	&execute_word, &interpret_num_word, &execute_word, // interpreting
	&comma_word,   &compile_num_word,   &execute_word  // compiling
};

/* interpret: ( -- )
 * Parses the input buffer and executes the words therein. */
static cdefn_t* interpret_ops[] =
{
	/* Parse a word. */
	&lit_word, (void*)32, &word_word, /* ( -- c-addr ) */
	/* End of the buffer? If so, return. */
	&c_at_word, &skipnot0_word, &exit_word, /* ( -- ) */

	/* Look up the word. */
	&here_word, &find_word, /* ( -- addr n ) */

	/* What is it? Calculate an offset into the lookup table. */
	&add_one_word, &cells_word,
	&state_word, &at_word, &lit_word, (void*)(3*8), &mul_word,
	&add_word, /* ( -- addr offset ) */

	/* Now look up the result. */
	&lit_word, (void*)interpreter_table, &add_word, &at_word, &execute_word,

	/* And go round again. */
	&branch_word, (void*)interpret_ops
};

/* interpret_num: ( c-addr -- )
 * We didn't recognise this word, so parse it as a number. */
static cdefn_t* interpret_num_ops[] =
{
	/* Get the length of the input string. */
	&dup_word, &c_at_word, /* ( -- addr len ) */
	/* The address we've got is a counted string; we want the address of the
	 * data. */
	&swap_word, &add_one_word, &swap_word, /* ( -- addr+1 len ) */
	/* Initialise the accumulator. */
	&zero_word, &rot_word, /* ( -- 0 addr+1 len ) */
	/* Parse! */
	&a_number_word, /* ( -- val addr+1 len ) */
	/* We must consume all bytes. */
	&skip0_word, &E_undef_word,
	/* Huzzah! */
	&pop_word, &exit_word
};

/* compile_num: ( c-addr -- )
 * We didn't recognise this word, so parse it as a number and compile it. */
static cdefn_t* compile_num_ops[] =
{
	/* The interpreter does the heavy lifting for us! */
	&interpret_num_word,
	/* ...and compile. */
	&lit_word, &lit_word, &comma_word,
	&comma_word,
	&exit_word
};

static const char prompt_msg[4] = " ok\n";
static cdefn_t* quit_ops[] =
{
	/* If we're reading from stdin, show the prompt. */
	&_input_fp_word, &at_word, &_stdin_word, &equals_word, &branch0_word, (void*)(quit_ops+11),
	&lit_word, (void*)prompt_msg, &lit_word, (void*)4, &type_word,

	/* Refill the input buffer. If there is not input buffer, exit. */
	&refill_word, &skipnot0_word, &exit_word,

	/* Interpret it. */
	&interpret_word,

	/* And go round again */
	&branch_word, (void*)quit_ops
};

static const char fopen_mode[] = "r";
static cdefn_t* read_file_ops[] =
{
	/* Read the filename. */
	&lit_word, (void*)' ', &word_word, /* ( -- len ) */

	/* Turn it into a C string. */
	&dup_word, &c_at_word, &add_word, &add_one_word,
	&zero_word, &swap_word, &c_pling_word, /* ( -- ) */
 
	/* Open the new one. */
	&here_word, &add_one_word, &lit_word, (void*)fopen_mode, &_fopen_word,
	&dup_word, &skipnot0_word, &E_fnf_word,

	/* Swap in the new stream, saving the old one to the stack. */
	&_input_fp_word, &at_word, /* ( -- new old ) */
	&swap_word, &_input_fp_word, &pling_word, /* ( -- old ) */

	/* Run the interpreter/compiler until EOF. */
	&quit_word,

	/* Close the new stream. */
	&_input_fp_word, &at_word, &_fclose_word, &pop_word,

	/* Restore the old stream. */
	&_input_fp_word, &pling_word,
	&exit_word
};

#define WORD(w, c, n, l, f, p...) \
	struct fstring_##w { uint8_t len; char data[sizeof(n)-1]; }; \
	static struct fstring_##w w##_name = {(sizeof(n)-1) | f, n}; \
	static cdefn_t w = { c, (struct fstring*) &w##_name, l, { p } };

#define COM(w, c, n, l, p...) WORD(w, c, n, l, 0, p)
#define IMM(w, c, n, l, p...) WORD(w, c, n, l, FL_IMM, p)

/* A list of words go here. To add a word, add a new entry and run this file as
 * a shell script. The link field will be set correctly.
 * BEWARE: these lines are parsed using whitespace. LEAVE EXACTLY AS IS.*/
//@WORDLIST
COM( E_fnf_word,         E_fnf_cb,       "",           NULL,             (void*)0 ) //@W
COM( E_undef_word,       E_undef_cb,     "",           &E_fnf_word,      (void*)0 ) //@W
COM( _create_word,       _create_cb,     "",           &E_undef_word,    ) //@W
COM( _exit_word,         _exit_cb,       "_exit",      &_create_word,    ) //@W
COM( _fclose_word,       _fclose_cb,     "_fclose",    &_exit_word,      ) //@W
COM( _feof_word,         _feof_cb,       "_feof",      &_fclose_word,    ) //@W
COM( _fopen_word,        _fopen_cb,      "_fopen",     &_feof_word,      ) //@W
COM( _fputc_word,        _fputc_cb,      "_fputc",     &_fopen_word,     ) //@W
COM( _fread_word,        _freadwrite_cb, "_fread",     &_fputc_word,     &fread ) //@W
COM( _fwrite_word,       _freadwrite_cb, "_fwrite",    &_fread_word,     &fwrite ) //@W
COM( _input_fp_word,     rvarword,       "_input_fp",  &_fwrite_word,    &input_fp ) //@W
COM( _stderr_word,       rivarword,      "_stderr",    &_input_fp_word,  &stderr ) //@W
COM( _stdin_word,        rivarword,      "_stdin",     &_stderr_word,    &stdin ) //@W
COM( _stdout_word,       rivarword,      "_stdout",    &_stdin_word,     &stdout ) //@W
COM( a_number_word,      a_number_cb,    ">number",    &_stdout_word,    ) //@W
COM( accept_word,        accept_cb,      "accept",     &a_number_word,   ) //@W
COM( add_one_word,       increment_cb,   "1+",         &accept_word,     (void*)1 ) //@W
COM( add_word,           add_cb,         "+",          &add_one_word,    ) //@W
COM( align_word,         align_cb,       "align",      &add_word,        ) //@W
COM( allot_word,         allot_cb,       "allot",      &align_word,      ) //@W
COM( and_word,           and_cb,         "and",        &allot_word,      ) //@W
COM( at_word,            at_cb,          "@",          &and_word,        ) //@W
COM( base_word,          rvarword,       "base",       &at_word,         &base ) //@W
COM( branch0_word,       branchif_cb,    "",           &base_word,       (void*)0 ) //@W
COM( branch_word,        branch_cb,      "",           &branchnot0_word, ) //@W
COM( branchnot0_word,    branchnotif_cb, "",           &branch0_word,    (void*)0 ) //@W
COM( bye_word,           icodeword,      "bye",        &branch_word,     bye_ops ) //@W
COM( c_at_word,          c_at_cb,        "c@",         &bye_word,        ) //@W
COM( c_comma_word,       icodeword,      "c,",         &c_at_word,       c_comma_ops ) //@W
COM( c_pling_word,       c_pling_cb,     "c!",         &c_comma_word,    ) //@W
COM( cell_word,          rvarword,       "cell",       &c_pling_word,    (void*)CELL ) //@W
COM( cells_word,         icodeword,      "cells",      &cell_word,       cells_ops ) //@W
COM( close_sq_word,      close_sq_cb,    "]",          &cells_word,      ) //@W
COM( colon_word,         icodeword,      ":",          &close_sq_word,   colon_ops ) //@W
COM( comma_word,         icodeword,      ",",          &colon_word,      comma_ops ) //@W
COM( compile_num_word,   icodeword,      "",           &comma_word,      compile_num_ops ) //@W
COM( constant_word,      icodeword,      "constant",   &compile_num_word, constant_ops ) //@W
COM( create_word,        icodeword,      "create",     &constant_word,   create_ops ) //@W
COM( decimal_word,       icodeword,      "decimal",    &create_word,     decimal_ops ) //@W
COM( div_word,           div_cb,         "/",          &decimal_word,    ) //@W
COM( dot_quote_rword,    dot_quote_rcb,  "",           &div_word,        ) //@W
COM( dot_word,           dot_cb,         ".",          &dot_quote_rword, ) //@W
COM( dup_word,           peekcon_cb,     "dup",        &dot_word,        (void*)1 ) //@W
COM( emit_word,          icodeword,      "emit",       &dup_word,        emit_ops ) //@W
COM( equals_word,        equals_cb,      "=",          &emit_word,       ) //@W
COM( execute_word,       execute_cb,     "execute",    &equals_word,     ) //@W
COM( exit_word,          exit_cb,        "exit",       &execute_word,    ) //@W
COM( fill_word,          fill_cb,        "fill",       &exit_word,       ) //@W
COM( find_word,          find_cb,        "find",       &fill_word,       ) //@W
COM( here_word,          rivarword,      "here",       &find_word,       &here ) //@W
COM( hex_word,           icodeword,      "hex",        &here_word,       hex_ops ) //@W
COM( in_arrow_word,      rvarword,       ">in",        &hex_word,        &in_arrow ) //@W
COM( interpret_num_word, icodeword,      "",           &in_arrow_word,   interpret_num_ops ) //@W
COM( interpret_word,     icodeword,      "interpret",  &interpret_num_word, interpret_ops ) //@W
COM( latest_word,        rivarword,      "latest",     &interpret_word,  &latest ) //@W
COM( lit_word,           lit_cb,         "",           &latest_word,     ) //@W
COM( m_one_word,         rvarword,       "-1",         &lit_word,        (void*)-1 ) //@W
COM( mul_word,           mul_cb,         "*",          &m_one_word,      ) //@W
COM( not_equals_word,    not_equals_cb,  "<>",         &mul_word,        ) //@W
COM( one_word,           rvarword,       "1",          &not_equals_word, (void*)1 ) //@W
COM( or_word,            or_cb,          "or",         &one_word,        ) //@W
COM( over_word,          peekcon_cb,     "over",       &or_word,         (void*)2 ) //@W
COM( pad_word,           rvarword,       "pad",        &over_word,       &here ) //@W
COM( pling_word,         pling_cb,       "!",          &pad_word,        ) //@W
COM( pop_word,           pop_cb,         "pop",        &pling_word,      ) //@W
COM( quit_word,          icodeword,      "",           &pop_word,        quit_ops ) //@W
COM( read_file_word,     icodeword,      "read-file",  &quit_word,       read_file_ops ) //@W
COM( refill_word,        icodeword,      "refill",     &read_file_word,  refill_ops ) //@W
COM( rot_word,           rot_cb,         "rot",        &refill_word,     ) //@W
COM( rsp0_word,          rvarword,       "rsp0",       &rot_word,        rstack ) //@W
COM( rsp_at_word,        rivarword,      "rsp@",       &rsp0_word,       &rsp ) //@W
COM( rsp_pling_word,     wivarword,      "rsp!",       &rsp_at_word,     &rsp ) //@W
COM( skip0_word,         skipif_cb,      "",           &rsp_pling_word,  (void*)0 ) //@W
COM( skipifi_word,       skipifi_cb,     "",           &skip0_word,      ) //@W
COM( skipnot0_word,      skipnotif_cb,   "",           &skipifi_word,    (void*)0 ) //@W
COM( skipnotifi_word,    skipnotifi_cb,  "",           &skipnot0_word,   ) //@W
COM( source_word,        r2varword,      "source",     &skipnotifi_word, input_buffer, (void*)MAX_LINE_LENGTH ) //@W
COM( sp0_word,           rvarword,       "sp0",        &source_word,     dstack ) //@W
COM( sp_at_word,         rivarword,      "sp@",        &sp0_word,        &dsp ) //@W
COM( sp_pling_word,      wivarword,      "sp!",        &sp_at_word,      &dsp ) //@W
COM( state_word,         rvarword,       "state",      &sp_pling_word,   &state ) //@W
COM( sub_one_word,       increment_cb,   "-1",         &state_word,      (void*)-1 ) //@W
COM( sub_word,           sub_cb,         "-",          &sub_one_word,    ) //@W
COM( swap_word,          swap_cb,        "swap",       &sub_word,        ) //@W
COM( two_word,           rvarword,       "2",          &swap_word,       (void*)2 ) //@W
COM( type_word,          icodeword,      "type",       &two_word,        type_ops ) //@W
COM( u_dot_word,         u_dot_cb,       "u.",         &type_word,       ) //@W
COM( variable_word,      icodeword,      "variable",   &u_dot_word,      variable_ops ) //@W
COM( word_word,          word_cb,        "word",       &variable_word,   ) //@W
COM( zero_word,          rvarword,       "0",          &word_word,       (void*)0 ) //@W
IMM( immediate_word,     immediate_cb,   "immediate",  &zero_word,       ) //@W
IMM( open_sq_word,       open_sq_cb,     "[",          &immediate_word,  ) //@W
IMM( semicolon_word,     icodeword,      ";",          &open_sq_word,    semicolon_ops ) //@W

static defn_t* latest = (defn_t*) &semicolon_word; //@E
static cdefn_t* last = (defn_t*) &semicolon_word; //@E

int main(int argc, const char* argv[])
{
	here = here_top = sbrk(0);
	claim_workspace(0);

	setjmp(onerror);
	input_fp = stdin;
	dsp = dstack;
	rsp = rstack;

	rpush((cell_t) &bye_ops);
	pc = (defn_t**) quit_ops;
	for (;;)
	{
		const struct definition* w = (void*) *pc++;
		#if 0
			printf("stack: ");
			cell_t* p;
			for (p = dstack; p < dsp; p++)
				printf("%lx ", *p);
			putchar('[');
			fstrout(w->name);
			putchar(']');
			putchar('\n');
		#endif
		w->code(w);
	}
}

