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
# file. (And an Awk script.) The awk file will autogenerate the Forth dictionary
# and precompiled words in the C source, which is just too fragile to do by
# hand.
# 
# //@W: marks a dictionary entry. This will get updated in place to form a linked
# list.
#
# //@C: cheesy™ precompiled Forth. Put semi-Forth code on the following comment
# lines; the line immediately afterwards will be updated to contain the byte-
# compiled version. Don't put a trailing semicolon.
#
# C compilation options:
#
#   -DFAST             don't bounds check the stack (smaller, faster code)
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

		ord_table = ""
		for (i = 0; i < 256; i++)
			ord_table = ord_table sprintf("%c", i)
	}

	function ord(s) {
		return index(ord_table, s) - 1
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

		wordname = \$4
		sub(/^"/, "", wordname)
		sub(/",$/, "", wordname)
		sub(/\\\\./, "&", wordname)
		words[wordname] = \$2

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

	function push(n) {
		stack[sp++] = n
	}

	function pop() {
		return stack[--sp]
	}

	function comma(s) {
		if (s !~ /,$/)
			s = s ","
		bytecode[pc++] = s
	}

	function compile(n) {
		if (n == "IF")
		{
			comma("&branch0_word")
			push(pc)
			comma(0)
			return
		}
		if (n == "THEN")
		{
			bytecode[pop()] = "(&" word ".payload[0] + " pc "),"
			return
		}
			
		if (n == "BEGIN")
		{
			push(pc)
			return
		}
		if (n == "AGAIN")
		{
			comma("&branch_word")
			comma("(&" word ".payload[0] + " pop() "),")
			return
		}
		if (n == "UNTIL")
		{
			comma("&branch0_word")
			comma("(&" word ".payload[0] + " pop() "),")
			return
		}

		if (n ~ /^\[.*]$/)
		{
			sub(/^\\[/, "", n)
			sub(/]$/, "", n)
			comma("(" n ")")
			return
		}

		wordsym = words[n]
		if (wordsym == "")
		{
			if (n ~ /-?[0-9]/)
			{
				comma("&lit_word,")
				comma(n ",")
				return
			}

			printf("Unrecognised word '%s' while defining '%s'\n", n, wordstring) > "/dev/stderr"
			exit(1)
		}
		comma("&" wordsym)
	}

	/^\/\/@C/ {
		print

		wordstring = \$2

		word = ""
		for (i=1; i<=length(wordstring); i++)
		{
			c = substr(wordstring, i, 1)
			if (c ~ /[A-Za-z_$]/)
				word = word c
			else
				word = word sprintf("_%02x_", ord(c))
		}
		word = tolower(word) "_word"

		sub(/\\\\/, "\\\\\\\\", wordstring)

		immediate = (\$3 == "IMMEDIATE")
		hidden = (\$3 == "HIDDEN")
		sp = 0
		pc = 0

		# (Yes, this is supposed to consume and not print one extra line.)
		while (getline)
		{
			if (\$1 != "//")
			{
				# Consume and do not print.
				break
			}
			print 
			
			for (i=2; i<=NF; i++)
			{
				if (\$i == "\\\\")
					break;
				compile(\$i)
			}
		}

		if (immediate)
			printf("IMM( ")
		else
			printf("COM( ")
		printf("%s, codeword, \"%s\", %s, ", word, hidden ? "" : wordstring, lastword)
		for (i = 0; i < pc; i++)
			printf("(void*)%s ", bytecode[i])
		printf("(void*)&exit_word )\n")
		lastword = "&" word
		words[wordstring] = word ","

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef intptr_t cell_t;
typedef uintptr_t ucell_t;
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

static int input_fd;
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

static void strerr(const char* s)
{
	write(2, s, strlen(s));
}

static void panic(const char* message)
{
	strerr("panic: ");
	strerr(message);
	strerr("\n");
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

	if (p > here_top)
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

/* Forward declarations of words go here --- do not edit.*/
//@EXPORT{
static cdefn_t E_fnf_word ;
static cdefn_t E_undef_word ;
static cdefn_t _O_RDONLY_word ;
static cdefn_t _O_RDWR_word ;
static cdefn_t _O_WRONLY_word ;
static cdefn_t _close_word ;
static cdefn_t _create_word ;
static cdefn_t _exit_word ;
static cdefn_t _input_fd_word ;
static cdefn_t _open_word ;
static cdefn_t _read_word ;
static cdefn_t _stderr_word ;
static cdefn_t _stdin_word ;
static cdefn_t _stdout_word ;
static cdefn_t _write_word ;
static cdefn_t a_number_word ;
static cdefn_t accept_word ;
static cdefn_t add_one_word ;
static cdefn_t add_word ;
static cdefn_t align_word ;
static cdefn_t allot_word ;
static cdefn_t and_word ;
static cdefn_t arrow_r_word ;
static cdefn_t at_word ;
static cdefn_t base_word ;
static cdefn_t branch0_word ;
static cdefn_t branch_word ;
static cdefn_t c_at_word ;
static cdefn_t c_pling_word ;
static cdefn_t cell_word ;
static cdefn_t close_sq_word ;
static cdefn_t div_word ;
static cdefn_t drop_word ;
static cdefn_t dup_word ;
static cdefn_t equals0_word ;
static cdefn_t equals_word ;
static cdefn_t execute_word ;
static cdefn_t exit_word ;
static cdefn_t fill_word ;
static cdefn_t find_word ;
static cdefn_t here_word ;
static cdefn_t in_arrow_word ;
static cdefn_t latest_word ;
static cdefn_t less0_word ;
static cdefn_t lit_word ;
static cdefn_t m_one_word ;
static cdefn_t more0_word ;
static cdefn_t mul_word ;
static cdefn_t not_equals_word ;
static cdefn_t notequals0_word ;
static cdefn_t one_word ;
static cdefn_t or_word ;
static cdefn_t over_word ;
static cdefn_t pad_word ;
static cdefn_t pling_word ;
static cdefn_t r_arrow_word ;
static cdefn_t rot_word ;
static cdefn_t rsp0_word ;
static cdefn_t rsp_at_word ;
static cdefn_t rsp_pling_word ;
static cdefn_t source_word ;
static cdefn_t sp0_word ;
static cdefn_t sp_at_word ;
static cdefn_t sp_pling_word ;
static cdefn_t state_word ;
static cdefn_t sub_one_word ;
static cdefn_t sub_word ;
static cdefn_t swap_word ;
static cdefn_t two_word ;
static cdefn_t word_word ;
static cdefn_t zero_word ;
static cdefn_t immediate_word ;
static cdefn_t open_sq_word ;
//@EXPORT}

/* ======================================================================= */
/*                                  WORDS                                  */
/* ======================================================================= */

static void codeword(cdefn_t* w) { rpush((cell_t) pc); pc = (void*) &w->payload[0]; }
static void dataword(cdefn_t* w) { dpush((cell_t) &w->payload[0]); }
static void rvarword(cdefn_t* w) { dpush((cell_t) w->payload[0]); }
static void r2varword(cdefn_t* w) { dpush((cell_t) w->payload[0]); dpush((cell_t) w->payload[1]); }
static void wvarword(defn_t* w) { w->payload[0] = (void*) dpop(); }
static void rivarword(cdefn_t* w) { dpush(*(cell_t*) w->payload[0]); }
static void wivarword(cdefn_t* w) { *(cell_t*)(w->payload[0]) = dpop(); }

static void _readwrite_cb(cdefn_t* w)
{
	size_t len = dpop();
	void* ptr = (void*)dpop();
	int fd = dpop();
	int (*func)(int fd, void* ptr, size_t size) = (void*) *w->payload;

	dpush(func(fd, ptr, len));
}

static void _open_cb(cdefn_t* w)
{
	int flags = dpop();
	const char* filename = (void*)dpop();
	dpush(open(filename, flags));
}

static void accept_cb(cdefn_t* w)
{
	cell_t max = dpop();
	char* addr = (char*)dpop();
	int len = 0;

	while (len < max)
	{
		char c;
		if (read(input_fd, &c, 1) <= 0)
		{
			if (len == 0)
				len = -1;
			break;
		}
		if (c == '\n')
			break;

		addr[len++] = c;
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
	#if 0
		printf("[defined ");
		fwrite(&defn->name->data[0], 1, defn->name->len & 0x7f, stdout);
		printf("]\n");
	#endif
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
	ucell_t ud = dpop();

	while (len > 0)
	{
		unsigned int d = get_digit(*addr);
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
	dpush(x2);
	dpush(x3);
	dpush(x1);
}

static void swap_cb(cdefn_t* w)
{
	cell_t x2 = dpop();
	cell_t x1 = dpop();
	dpush(x2);
	dpush(x1);
}

static void E_fnf_cb(cdefn_t* w)      { panic("file not found"); }
static void E_undef_cb(cdefn_t* w)    { panic("unrecognised word"); }
static void _close_cb(cdefn_t* w)     { dpush(close(dpop())); }
static void _exit_cb(cdefn_t* w)      { exit(dpop()); }
static void add_cb(cdefn_t* w)        { dpush(dpop() + dpop()); }
static void align_cb(cdefn_t* w)      { claim_workspace((CELL - (cell_t)here) & (CELL-1)); }
static void allot_cb(cdefn_t* w)      { claim_workspace(dpop()); }
static void and_cb(cdefn_t* w)        { dpush(dpop() & dpop()); }
static void arrow_r_cb(cdefn_t* w)    { rpush(dpop()); }
static void at_cb(cdefn_t* w)         { dpush(*(cell_t*)dpop()); }
static void branch_cb(cdefn_t* w)     { pc = (void*) *pc; }
static void branchif_cb(cdefn_t* w)   { if (dpop() == (cell_t)*w->payload) pc = (void*)*pc; else pc++; }
static void c_at_cb(cdefn_t* w)       { dpush(*(uint8_t*)dpop()); }
static void c_pling_cb(cdefn_t* w)    { uint8_t* p = (uint8_t*)dpop(); *p = dpop(); }
static void close_sq_cb(cdefn_t* w)   { state = 1; }
static void div_cb(cdefn_t* w)        { cell_t a = dpop(); cell_t b = dpop(); dpush(b / a); }
static void drop_cb(cdefn_t* w)       { dpop(); }
static void equals0_cb(cdefn_t* w)    { dpush(dpop() == 0); }
static void equals_cb(cdefn_t* w)     { dpush(dpop() == dpop()); }
static void execute_cb(cdefn_t* w)    { cdefn_t* p = (void*) dpop(); p->code(p); }
static void exit_cb(cdefn_t* w)       { pc = (void*)rpop(); }
static void increment_cb(cdefn_t* w)  { dpush(dpop() + (cell_t)*w->payload); }
static void less0_cb(cdefn_t* w)      { dpush(dpop() < 0); }
static void lit_cb(cdefn_t* w)        { dpush((cell_t) *pc++); }
static void more0_cb(cdefn_t* w)      { dpush(dpop() > 0); }
static void mul_cb(cdefn_t* w)        { dpush(dpop() * dpop()); }
static void not_equals_cb(cdefn_t* w) { dpush(dpop() != dpop()); }
static void notequals0_cb(cdefn_t* w) { dpush(dpop() != 0); }
static void open_sq_cb(cdefn_t* w)    { state = 0; }
static void or_cb(cdefn_t* w)         { dpush(dpop() | dpop()); }
static void peekcon_cb(cdefn_t* w)    { dpush(dpeek((cell_t) *w->payload)); }
static void pling_cb(cdefn_t* w)      { cell_t* p = (cell_t*)dpop(); *p = dpop(); }
static void r_arrow_cb(cdefn_t* w)    { dpush(rpop()); }
static void sub_cb(cdefn_t* w)        { cell_t a = dpop(); cell_t b = dpop(); dpush(b - a); }

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
COM( E_fnf_word,         E_fnf_cb,       "E_fnf",      NULL,             (void*)0 ) //@W
COM( E_undef_word,       E_undef_cb,     "E_undef",    &E_fnf_word,      (void*)0 ) //@W
COM( _O_RDONLY_word,     rvarword,       "O_RDONLY",   &E_undef_word,    (void*)O_RDONLY ) //@W
COM( _O_RDWR_word,       rvarword,       "O_RDWR",     &_O_RDONLY_word,  (void*)O_RDWR ) //@W
COM( _O_WRONLY_word,     rvarword,       "O_WRONLY",   &_O_RDWR_word,    (void*)O_WRONLY ) //@W
COM( _close_word,        _close_cb,      "_close",     &_O_WRONLY_word,  ) //@W
COM( _create_word,       _create_cb,     "",           &_close_word,     ) //@W
COM( _exit_word,         _exit_cb,       "_exit",      &_create_word,    ) //@W
COM( _input_fd_word,     rvarword,       "_input_fd",  &_exit_word,      &input_fd ) //@W
COM( _open_word,         _open_cb,       "_open",      &_input_fd_word,  ) //@W
COM( _read_word,         _readwrite_cb,  "_read",      &_open_word,      &read ) //@W
COM( _stderr_word,       rvarword,       "_stderr",    &_read_word,      (void*)2 ) //@W
COM( _stdin_word,        rvarword,       "_stdin",     &_stderr_word,    (void*)0 ) //@W
COM( _stdout_word,       rvarword,       "_stdout",    &_stdin_word,     (void*)1 ) //@W
COM( _write_word,        _readwrite_cb,  "_write",     &_stdout_word,    &write ) //@W
COM( a_number_word,      a_number_cb,    ">NUMBER",    &_write_word,     ) //@W
COM( accept_word,        accept_cb,      "ACCEPT",     &a_number_word,   ) //@W
COM( add_one_word,       increment_cb,   "1+",         &accept_word,     (void*)1 ) //@W
COM( add_word,           add_cb,         "+",          &add_one_word,    ) //@W
COM( align_word,         align_cb,       "ALIGN",      &add_word,        ) //@W
COM( allot_word,         allot_cb,       "ALLOT",      &align_word,      ) //@W
COM( and_word,           and_cb,         "AND",        &allot_word,      ) //@W
COM( arrow_r_word,       arrow_r_cb,     ">R",         &and_word,        ) //@W
COM( at_word,            at_cb,          "@",          &arrow_r_word,    ) //@W
COM( base_word,          rvarword,       "BASE",       &at_word,         &base ) //@W
COM( branch0_word,       branchif_cb,    "0BRANCH",    &base_word,       (void*)0 ) //@W
COM( branch_word,        branch_cb,      "BRANCH",     &branch0_word,    ) //@W
COM( c_at_word,          c_at_cb,        "C@",         &branch_word,     ) //@W
COM( c_pling_word,       c_pling_cb,     "C!",         &c_at_word,       ) //@W
COM( cell_word,          rvarword,       "CELL",       &c_pling_word,    (void*)CELL ) //@W
COM( close_sq_word,      close_sq_cb,    "]",          &cell_word,       ) //@W
COM( div_word,           div_cb,         "/",          &close_sq_word,   ) //@W
COM( drop_word,          drop_cb,        "DROP",       &div_word,        ) //@W
COM( dup_word,           peekcon_cb,     "DUP",        &drop_word,       (void*)1 ) //@W
COM( equals0_word,       equals0_cb,     "0=",         &dup_word,        ) //@W
COM( equals_word,        equals_cb,      "=",          &equals0_word,    ) //@W
COM( execute_word,       execute_cb,     "EXECUTE",    &equals_word,     ) //@W
COM( exit_word,          exit_cb,        "EXIT",       &execute_word,    ) //@W
COM( fill_word,          fill_cb,        "FILL",       &exit_word,       ) //@W
COM( find_word,          find_cb,        "FIND",       &fill_word,       ) //@W
COM( here_word,          rvarword,       "HERE",       &find_word,       &here ) //@W
COM( in_arrow_word,      rvarword,       ">IN",        &here_word,       &in_arrow ) //@W
COM( latest_word,        rvarword,       "LATEST",     &in_arrow_word,   &latest ) //@W
COM( less0_word,         less0_cb,       "0<",         &latest_word,     ) //@W
COM( lit_word,           lit_cb,         "LIT",        &less0_word,      ) //@W
COM( m_one_word,         rvarword,       "-1",         &lit_word,        (void*)-1 ) //@W
COM( more0_word,         more0_cb,       "0>",         &m_one_word,      ) //@W
COM( mul_word,           mul_cb,         "*",          &more0_word,      ) //@W
COM( not_equals_word,    not_equals_cb,  "<>",         &mul_word,        ) //@W
COM( notequals0_word,    notequals0_cb,  "0<>",        &not_equals_word, ) //@W
COM( one_word,           rvarword,       "1",          &notequals0_word, (void*)1 ) //@W
COM( or_word,            or_cb,          "OR",         &one_word,        ) //@W
COM( over_word,          peekcon_cb,     "OVER",       &or_word,         (void*)2 ) //@W
COM( pad_word,           rvarword,       "PAD",        &over_word,       &here ) //@W
COM( pling_word,         pling_cb,       "!",          &pad_word,        ) //@W
COM( r_arrow_word,       r_arrow_cb,     "R>",         &pling_word,      ) //@W
COM( rot_word,           rot_cb,         "ROT",        &r_arrow_word,    ) //@W
COM( rsp0_word,          rvarword,       "RSP0",       &rot_word,        rstack ) //@W
COM( rsp_at_word,        rivarword,      "RSP@",       &rsp0_word,       &rsp ) //@W
COM( rsp_pling_word,     wivarword,      "RSP!",       &rsp_at_word,     &rsp ) //@W
COM( source_word,        r2varword,      "SOURCE",     &rsp_pling_word,  input_buffer, (void*)MAX_LINE_LENGTH ) //@W
COM( sp0_word,           rvarword,       "SP0",        &source_word,     dstack ) //@W
COM( sp_at_word,         rivarword,      "SP@",        &sp0_word,        &dsp ) //@W
COM( sp_pling_word,      wivarword,      "SP!",        &sp_at_word,      &dsp ) //@W
COM( state_word,         rvarword,       "STATE",      &sp_pling_word,   &state ) //@W
COM( sub_one_word,       increment_cb,   "-1",         &state_word,      (void*)-1 ) //@W
COM( sub_word,           sub_cb,         "-",          &sub_one_word,    ) //@W
COM( swap_word,          swap_cb,        "SWAP",       &sub_word,        ) //@W
COM( two_word,           rvarword,       "2",          &swap_word,       (void*)2 ) //@W
COM( word_word,          word_cb,        "WORD",       &two_word,        ) //@W
COM( zero_word,          rvarword,       "0",          &word_word,       (void*)0 ) //@W
IMM( immediate_word,     immediate_cb,   "IMMEDIATE",  &zero_word,       ) //@W
IMM( open_sq_word,       open_sq_cb,     "[",          &immediate_word,  ) //@W

//@C ( IMMEDIATE
//   10 WORD DROP
IMM( _28__word, codeword, "(", &open_sq_word, (void*)&lit_word, (void*)10, (void*)&word_word, (void*)&drop_word, (void*)&exit_word )

//@C \ IMMEDIATE
//   40 WORD DROP
IMM( _5c__word, codeword, "\\", &_28__word, (void*)&lit_word, (void*)40, (void*)&word_word, (void*)&drop_word, (void*)&exit_word )

//@C CELLS
//  CELL *
COM( cells_word, codeword, "CELLS", &_5c__word, (void*)&cell_word, (void*)&mul_word, (void*)&exit_word )

//@C ,
//  HERE @ !
//  CELL ALLOT
COM( _2c__word, codeword, ",", &cells_word, (void*)&here_word, (void*)&at_word, (void*)&pling_word, (void*)&cell_word, (void*)&allot_word, (void*)&exit_word )

//@C C,
//  HERE @ C!
//  1 ALLOT
COM( c_2c__word, codeword, "C,", &_2c__word, (void*)&here_word, (void*)&at_word, (void*)&c_pling_word, (void*)&one_word, (void*)&allot_word, (void*)&exit_word )

//@C CREATE
//  \ Get the word name; this is written as a counted string to here.
//  32 WORD                            \ addr --
//
//  \ Advance over it.
//  DUP C@ 1+ ALLOT                    \ addr --
//
//  \ Ensure alignment, then create the low level header.
//  ALIGN [&_create_word]
COM( create_word, codeword, "CREATE", &c_2c__word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&dup_word, (void*)&c_at_word, (void*)&add_one_word, (void*)&allot_word, (void*)&align_word, (void*)(&_create_word), (void*)&exit_word )

//@C EMIT
//   HERE @ C!
//   _stdout HERE @ 1 _write DROP
COM( emit_word, codeword, "EMIT", &create_word, (void*)&here_word, (void*)&at_word, (void*)&c_pling_word, (void*)&_stdout_word, (void*)&here_word, (void*)&at_word, (void*)&one_word, (void*)&_write_word, (void*)&drop_word, (void*)&exit_word )

//@C TYPE
// \ ( addr n -- )
//   _stdout ROT ROT _write DROP
COM( type_word, codeword, "TYPE", &emit_word, (void*)&_stdout_word, (void*)&rot_word, (void*)&rot_word, (void*)&_write_word, (void*)&drop_word, (void*)&exit_word )

//@C CR
//   10 EMIT
COM( cr_word, codeword, "CR", &type_word, (void*)&lit_word, (void*)10, (void*)&emit_word, (void*)&exit_word )

//@C SPACE
//   32 EMIT
COM( space_word, codeword, "SPACE", &cr_word, (void*)&lit_word, (void*)32, (void*)&emit_word, (void*)&exit_word )

//@C NEGATE
//   0 SWAP -
COM( negate_word, codeword, "NEGATE", &space_word, (void*)&zero_word, (void*)&swap_word, (void*)&sub_word, (void*)&exit_word )

//@C TRUE
//   1
COM( true_word, codeword, "TRUE", &negate_word, (void*)&one_word, (void*)&exit_word )

//@C FALSE
//   0
COM( false_word, codeword, "FALSE", &true_word, (void*)&zero_word, (void*)&exit_word )

//@C BYE
//   0 _exit
COM( bye_word, codeword, "BYE", &false_word, (void*)&zero_word, (void*)&_exit_word, (void*)&exit_word )

//@C REFILL
//  \ Read a line from the terminal.
//  SOURCE ACCEPT              \ -- len
//
//  \ Is this the end?
//  DUP 0< IF
//    DROP 0 EXIT
//  THEN
//
//  \ Clear the remainder of the buffer.
//  DUP [&lit_word] [input_buffer] +       \ -- len addr
//  SWAP                                   \ -- addr len
//  [&lit_word] [MAX_LINE_LENGTH] SWAP -   \ -- addr remaining
//  32                                     \ -- addr remaining char
//  FILL
//
//  \ Reset the input pointer.
//  0 >IN !
//
//  \ We must succeed!
//  1
COM( refill_word, codeword, "REFILL", &bye_word, (void*)&source_word, (void*)&accept_word, (void*)&dup_word, (void*)&less0_word, (void*)&branch0_word, (void*)(&refill_word.payload[0] + 9), (void*)&drop_word, (void*)&zero_word, (void*)&exit_word, (void*)&dup_word, (void*)(&lit_word), (void*)(input_buffer), (void*)&add_word, (void*)&swap_word, (void*)(&lit_word), (void*)(MAX_LINE_LENGTH), (void*)&swap_word, (void*)&sub_word, (void*)&lit_word, (void*)32, (void*)&fill_word, (void*)&zero_word, (void*)&in_arrow_word, (void*)&pling_word, (void*)&one_word, (void*)&exit_word )

//@C INTERPRET_NUM HIDDEN
// \ Evaluates a number, or perish in the attempt.
// \ ( c-addr -- value )
//   \ Get the length of the input string.
//   DUP C@                            \ -- addr len
//
//   \ The address we've got is a counted string; we want the address of the data.
//   SWAP 1+                           \ -- len addr+1
//
//   \ Initialise the accumulator.
//   0 SWAP ROT                        \ -- 0 addr+1 len
//
//   \ Parse!
//   >NUMBER                           \ -- val addr+1 len
//
//   \ We must consume all bytes to succeed.
//   IF E_undef THEN
//
//   \ Huzzah!
//   DROP
COM( interpret_num_word, codeword, "", &refill_word, (void*)&dup_word, (void*)&c_at_word, (void*)&swap_word, (void*)&add_one_word, (void*)&zero_word, (void*)&swap_word, (void*)&rot_word, (void*)&a_number_word, (void*)&branch0_word, (void*)(&interpret_num_word.payload[0] + 11), (void*)&E_undef_word, (void*)&drop_word, (void*)&exit_word )

//@C COMPILE_NUM HIDDEN
// \ Compiles a number (or at least, a word we don't recognise).
// \ ( c-addr -- )
//   \ The interpreter does the heavy lifting for us!
//   INTERPRET_NUM                     \ -- value
//
//   \ ...and compile.
//   [&lit_word] [&lit_word] , ,
COM( compile_num_word, codeword, "", &interpret_num_word, (void*)&interpret_num_word, (void*)(&lit_word), (void*)(&lit_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&exit_word )

static cdefn_t* interpreter_table[] =
{
	// compiling   not found            immediate
	&execute_word, &interpret_num_word, &execute_word, // interpreting
	&_2c__word,    &compile_num_word,   &execute_word  // compiling
};

//@C INTERPRET
// \ Parses the input buffer and executes the words therein.
//   BEGIN
//     \ Parse a word.
//     \ (This relies of word writing the result to here.)
//     32 WORD                         \ -- c-addr
//
//     \ End of the buffer? If so, return.
//     C@ 0= IF EXIT THEN              \ --
//
//     \ Look up the word.
//     HERE @ FIND                     \ -- addr kind
//
//     \ What is it? Calculate an offset into the lookup table.
//     1+ CELLS
//     STATE @ 24 *
//     +                               \ -- addr offset
//
//     \ Look up the right word and run it.
//     [&lit_word] [interpreter_table] + @ EXECUTE \ -- addr
//   AGAIN
COM( interpret_word, codeword, "INTERPRET", &compile_num_word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&c_at_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&interpret_word.payload[0] + 8), (void*)&exit_word, (void*)&here_word, (void*)&at_word, (void*)&find_word, (void*)&add_one_word, (void*)&cells_word, (void*)&state_word, (void*)&at_word, (void*)&lit_word, (void*)24, (void*)&mul_word, (void*)&add_word, (void*)(&lit_word), (void*)(interpreter_table), (void*)&add_word, (void*)&at_word, (void*)&execute_word, (void*)&branch_word, (void*)(&interpret_word.payload[0] + 0), (void*)&exit_word )

static const char prompt_msg[4] = " ok\n";
//@C INTERACT
//  BEGIN
//    \ If we're reading from stdin, show the prompt.
//    _input_fd @ _stdin = IF
//      [&lit_word] [prompt_msg] 4 TYPE
//    THEN
//
//    \ Refill the input buffer; if we run out, exit.
//    REFILL 0= IF EXIT THEN
//
//    \ Interpret the contents of the buffer.
//    INTERPRET
//  AGAIN
COM( interact_word, codeword, "INTERACT", &interpret_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&_stdin_word, (void*)&equals_word, (void*)&branch0_word, (void*)(&interact_word.payload[0] + 11), (void*)(&lit_word), (void*)(prompt_msg), (void*)&lit_word, (void*)4, (void*)&type_word, (void*)&refill_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&interact_word.payload[0] + 16), (void*)&exit_word, (void*)&interpret_word, (void*)&branch_word, (void*)(&interact_word.payload[0] + 0), (void*)&exit_word )

//@C QUIT
//  SP0 SP!
//  RSP0 RSP!
//  [&lit_word] [&bye_word] >R
//  INTERACT
COM( quit_word, codeword, "QUIT", &interact_word, (void*)&sp0_word, (void*)&sp_pling_word, (void*)&rsp0_word, (void*)&rsp_pling_word, (void*)(&lit_word), (void*)(&bye_word), (void*)&arrow_r_word, (void*)&interact_word, (void*)&exit_word )

//@C READ-FILE
//   \ Read the filename.
//   32 WORD
//
//   \ Turn it into a C string.
//   DUP C@ + 1+ 0 SWAP C!
//
//   \ Open the new file.
//   HERE @ 1+ O_RDONLY _open
//   DUP 0= IF E_fnf THEN
//
//   \ Swap in the new stream, saving the old one to the stack.
//   _input_fd @
//   SWAP _input_fd !
//
//   \ Run the interpreter/compiler until EOF.
//   INTERACT
//
//   \ Close the new stream.
//   _input_fd @ _close DROP
//
//   \ Restore the old stream.
//   _input_fd !
COM( read_2d_file_word, codeword, "READ-FILE", &quit_word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&dup_word, (void*)&c_at_word, (void*)&add_word, (void*)&add_one_word, (void*)&zero_word, (void*)&swap_word, (void*)&c_pling_word, (void*)&here_word, (void*)&at_word, (void*)&add_one_word, (void*)&_O_RDONLY_word, (void*)&_open_word, (void*)&dup_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&read_2d_file_word.payload[0] + 20), (void*)&E_fnf_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&swap_word, (void*)&_input_fd_word, (void*)&pling_word, (void*)&interact_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&_close_word, (void*)&drop_word, (void*)&_input_fd_word, (void*)&pling_word, (void*)&exit_word )

//@C :
//  \ Create the word itself.
//  CREATE
//
//  \ Turn it into a runnable word.
//  [&lit_word] [codeword] LATEST @ !
//
//  \ Switch to compilation mode.
//  ]
COM( _3a__word, codeword, ":", &read_2d_file_word, (void*)&create_word, (void*)(&lit_word), (void*)(codeword), (void*)&latest_word, (void*)&at_word, (void*)&pling_word, (void*)&close_sq_word, (void*)&exit_word )

//@C ; IMMEDIATE
//  [&lit_word] [&exit_word] ,
//  [
IMM( _3b__word, codeword, ";", &_3a__word, (void*)(&lit_word), (void*)(&exit_word), (void*)&_2c__word, (void*)&open_sq_word, (void*)&exit_word )

//@C CONSTANT
// \ ( value -- )
//  CREATE
//  [&lit_word] [rvarword] LATEST @ !
//  ,
COM( constant_word, codeword, "CONSTANT", &_3b__word, (void*)&create_word, (void*)(&lit_word), (void*)(rvarword), (void*)&latest_word, (void*)&at_word, (void*)&pling_word, (void*)&_2c__word, (void*)&exit_word )

//@C VARIABLE
//  CREATE 0 ,
COM( variable_word, codeword, "VARIABLE", &constant_word, (void*)&create_word, (void*)&zero_word, (void*)&_2c__word, (void*)&exit_word )

//@C IF IMMEDIATE
// \ -- addr
//  [&lit_word] [&branch0_word] ,
//  HERE @
//  0 ,
IMM( if_word, codeword, "IF", &variable_word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&here_word, (void*)&at_word, (void*)&zero_word, (void*)&_2c__word, (void*)&exit_word )

//@C THEN IMMEDIATE
// \ addr --
//  HERE @ SWAP !
IMM( then_word, codeword, "THEN", &if_word, (void*)&here_word, (void*)&at_word, (void*)&swap_word, (void*)&pling_word, (void*)&exit_word )

//@C ELSE IMMEDIATE
// \ if-addr -- else-addr
//  \ Emit a branch over the false part.
//  [&lit_word] [&branch_word] ,      \ -- if-addr
//
//  \ Remember where the branch label is for patching later.
//  HERE @ 0 ,                         \ -- if-addr else-addr
//
//  \ Patch the *old* branch label (from the condition) to the current address.
//  SWAP                               \ -- else-addr if-addr
//  [&then_word]
IMM( else_word, codeword, "ELSE", &then_word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&here_word, (void*)&at_word, (void*)&zero_word, (void*)&_2c__word, (void*)&swap_word, (void*)(&then_word), (void*)&exit_word )

//@C BEGIN IMMEDIATE
// \ -- start-addr
//  HERE @
IMM( begin_word, codeword, "BEGIN", &else_word, (void*)&here_word, (void*)&at_word, (void*)&exit_word )

//@C AGAIN IMMEDIATE
// \ start-addr --
//   [&lit_word] [&branch_word] , ,
IMM( again_word, codeword, "AGAIN", &begin_word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&exit_word )

//@C UNTIL IMMEDIATE
// \ start-addr --
//   [&lit_word] [&branch0_word] , ,
IMM( until_word, codeword, "UNTIL", &again_word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&exit_word )

//@C WHILE IMMEDIATE
// \ Used as 'begin <cond> while <loop-body> repeat'.
// \ start-addr -- start-addr while-target-addr
//   [&lit_word] [&branch0_word] ,
//   HERE @
//   0 ,
IMM( while_word, codeword, "WHILE", &until_word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&here_word, (void*)&at_word, (void*)&zero_word, (void*)&_2c__word, (void*)&exit_word )

//@C REPEAT IMMEDIATE
// \ start-addr while-target-addr --
//   SWAP
//   [&lit_word] [&branch_word] , ,
//
//   HERE @ SWAP !
IMM( repeat_word, codeword, "REPEAT", &while_word, (void*)&swap_word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&here_word, (void*)&at_word, (void*)&swap_word, (void*)&pling_word, (void*)&exit_word )

//@C HEX
//  16 STATE !
COM( hex_word, codeword, "HEX", &repeat_word, (void*)&lit_word, (void*)16, (void*)&state_word, (void*)&pling_word, (void*)&exit_word )

//@C DECIMAL
//  10 STATE !
COM( decimal_word, codeword, "DECIMAL", &hex_word, (void*)&lit_word, (void*)10, (void*)&state_word, (void*)&pling_word, (void*)&exit_word )

static cdefn_t* last = (defn_t*) &decimal_word; //@E
static defn_t* latest = (defn_t*) &decimal_word; //@E

int main(int argc, const char* argv[])
{
	here = here_top = sbrk(0);
	claim_workspace(0);

	setjmp(onerror);
	input_fd = 0;
	dsp = dstack;
	rsp = rstack;

	pc = (defn_t**) &quit_word.payload[0];
	for (;;)
	{
		const struct definition* w = (void*) *pc++;
		#if 0
			printf("stack: ");
			cell_t* p;
			for (p = dstack; p < dsp; p++)
				printf("%lx ", *p);
			putchar('[');
			fwrite(&w->name->data[0], 1, w->name->len & 0x7f, stdout);
			putchar(']');
			putchar('\n');
		#endif
		w->code(w);
	}
}

