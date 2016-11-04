#define dummy /*
# fforth © 2015 David Given
# All rights reserved.
#
# --- BSD 2 CLAUSE LICENSE FOLLOWS ---
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE. is available here:
#
# --- END OF LICENSE ---
# 
# fforth is a small indirect-threaded Forth written in portable C. It should
# Just Compile on most Unixy platforms. It's intended as a scripting language
# for the Fuzix operating system but should work fine on any small system.
#
# It supports most of the core ANS Forth words, and passes (most of) John
# Hayes' ANS conformance test. Or at least that of the version I found.
#
# For reference, the version of the standard I've been referring to is:
# http://lars.nocrew.org/dpans/dpans6.htm
#
# 
# Peculiarities include:
# 
# DOES>
#     ANS Forth decrees that you can call DOES> multiple times on a word, where
#     each time it changes the code part of the word. fforth doesn't support
#     that. If you call DOES> twice here, you end up *appending* the behaviour
#     after the DOES> --- so the old code will be called, then the new code
#     will be called.
#
# READ-FILE filename
#     Opens and then executes Forth code supplied in the named file.
#
# In addition, it's probably full of bugs.
#
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
# //@E: patches up the attached statement to refer to the last word defined.
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
		if (n == "ELSE")
		{
			elsejump = pop()
			comma("&branch_word")
			push(pc)
			comma(0)

			bytecode[elsejump] = "(&" word ".payload[0] + " pc "),"
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
		if (n == "WHILE")
		{
			comma("&branch0_word")
			push(pc)
			comma(0)
			return
		}
		if (n == "REPEAT")
		{
			whilefalse = pop()
			comma("&branch_word")
			comma("(&" word ".payload[0] + " pop() "),")
			bytecode[whilefalse] = "(&" word ".payload[0] + " pc "),"
			return
		}

		if (n == "RECURSE")
		{
			comma("&" word)
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
		sub(/"/, "\\\\\\"", wordstring)

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
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/signal.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <utime.h>
#include <fcntl.h>

#if INTPTR_MAX == INT16_MAX
	typedef int16_t cell_t;
	typedef uint16_t ucell_t;
	typedef int32_t pair_t;
	typedef uint32_t upair_t;
#elif INTPTR_MAX == INT32_MAX
	typedef int32_t cell_t;
	typedef uint32_t ucell_t;
	typedef int64_t pair_t;
	typedef uint64_t upair_t;
#elif INTPTR_MAX == INT64_MAX
	typedef int64_t cell_t;
	typedef uint64_t ucell_t;
	/* This works on gcc and is useful for debugging; I'm not really expecting
	 * people will be using fforth for real on 64-bit platforms. */
	typedef __int128_t pair_t;
	typedef __uint128_t upair_t;
#else
	#error "Don't understand the size of your platform!"
#endif

typedef struct definition defn_t;
typedef const struct definition cdefn_t;

static jmp_buf onerror;

#define MAX_LINE_LENGTH 160
#define ALLOCATION_CHUNK_SIZE 128
#define PAD_SIZE 140
#define CELL sizeof(cell_t)

#define DSTACKSIZE 64
static cell_t dstack[DSTACKSIZE];
static cell_t* dsp;

#define RSTACKSIZE 64
static cell_t rstack[RSTACKSIZE];
static cell_t* rsp;

static int input_fd;
static char input_buffer[MAX_LINE_LENGTH];
static char* in_base;
static cell_t in_len;
static cell_t in_arrow;
static cell_t base = 10;
static char* holdptr;
static cell_t state = false;

static defn_t** pc;
static defn_t* latest; /* Most recent word on dictionary */
static cdefn_t* last;   /* Last of the built-in words */

static uint8_t* here;
static uint8_t* here_top;

static const char** global_argv;
static int global_argc;

typedef void code_fn(cdefn_t* w);
static void align_cb(cdefn_t* w);

#define FL_IMMEDIATE 0x80
#define FL_SMUDGE    0x40
#define FL__MASK     0x3f

#define CONST_STRING(n, v) \
	static const char n[sizeof(v)-1] = v

struct fstring
{
	uint8_t len;
	char data[];
};

struct definition
{
	code_fn* code;
	struct fstring* name; // top bits of len are flags!
	cdefn_t* next;
	void* payload[];
};

static void strerr(const char* s)
{
	write(2, s, strlen(s));
}

static void panic(const char* message)
{
	strerr("panic: ");
	strerr(message);
	strerr("\n");
	longjmp(onerror, 1);
}

#if !defined FAST
static void dadjust(int delta)
{
	dsp -= delta;
	if (dsp <= dstack)
		panic("data stack overflow");
	if (dsp > dstack+DSTACKSIZE)
		panic("data stack underflow");
}

static void radjust(int delta)
{
	rsp -= delta;
	if (rsp <= rstack)
		panic("return stack overflow");
	if (rsp > rstack+RSTACKSIZE)
		panic("return stack underflow");
}

static void dpush(cell_t val)
{
	dadjust(1);
	*dsp = val;
}

static cell_t dpop(void)
{
	cell_t v = *dsp;
	dadjust(-1);
	return v;
}

static cell_t* daddr(int count)
{
	cell_t* ptr = dsp + count;
	if (ptr > dstack+DSTACKSIZE)
		panic("data stack underflow");
	return ptr;
}

static void rpush(cell_t val)
{
	radjust(1);
	*rsp = val;
}

static cell_t rpop(void)
{
	cell_t v = *rsp;
	radjust(-1);
	return v;
}

static cell_t* raddr(int count)
{
	cell_t* ptr = rsp + count;
	if (ptr >= rstack+RSTACKSIZE)
		panic("return stack underflow");
	return ptr;
}

#else
static inline void dadjust(cell_t val, int delta) { dsp -= delta; }
static inline void radjust(cell_t val, int delta) { rsp -= delta; }
static inline void dpush(cell_t val) { *--dsp = val; }
static inline cell_t dpop(void) { return *dsp++; }
static inline cell_t daddr(int count) { return dsp+count; }
static inline void rpush(cell_t val) { *--rsp = val; }
static inline cell_t rpop(void) { return *rsp++; }
static inline cell_t raddr(int count) { return rsp+count; }
#endif

static pair_t readpair(ucell_t* ptr)
{
	upair_t p = ptr[0];
	p <<= sizeof(cell_t)*8;
	p |= ptr[1];
	return p;
}

static void writepair(ucell_t* ptr, upair_t p)
{
	ptr[1] = p;
	ptr[0] = p >> (sizeof(cell_t)*8);
}

static pair_t dpopd(void)
{
	pair_t v = readpair((ucell_t*) dsp);
	dadjust(-2);
	return v;
}

static void dpushd(upair_t p)
{
	dadjust(2);
	writepair((ucell_t*) dsp, p);
}

static void dpushbool(bool b)
{
	dpush(b ? -1 : 0);
}

static void* ensure_workspace(size_t length)
{
	uint8_t* p = here + length;

	while ((p+PAD_SIZE) > here_top)
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
 * strings, because it ignores the top bits of the length (used in the
 * dictionary as flags). */
static int fstreq(const struct fstring* f1, const struct fstring* f2)
{
	int len1 = f1->len & FL__MASK;
	int len2 = f2->len & FL__MASK;
	if (len1 != len2)
		return 0;
	return (memcmp(f1->data, f2->data, len1) == 0);
}

/* Forward declarations of words go here --- do not edit.*/
//@EXPORT{
static cdefn_t E_fnf_word ;
static cdefn_t _O_RDONLY_word ;
static cdefn_t _O_RDWR_word ;
static cdefn_t _O_WRONLY_word ;
static cdefn_t _chdir_word ;
static cdefn_t _close_word ;
static cdefn_t _create_word ;
static cdefn_t _exit_word ;
static cdefn_t _input_fd_word ;
static cdefn_t _mknod_word ;
static cdefn_t _open_word ;
static cdefn_t _read_word ;
static cdefn_t _link_word ;
static cdefn_t _lseek_word ;
static cdefn_t _rename_word ;
static cdefn_t _sync_word ;
static cdefn_t _access_word ;
static cdefn_t _chmod_word ;
static cdefn_t _chown_word ;
static cdefn_t _stat_word ;
static cdefn_t _fstat_word ;
static cdefn_t _dup_word ;
static cdefn_t _getpid_word ;
static cdefn_t _getppid_word ;
static cdefn_t _getuid_word ;
static cdefn_t _umask_word ;
static cdefn_t _write_word ;
static cdefn_t _execve_word ;
static cdefn_t _setuid_word ;
static cdefn_t _setgid_word ;
static cdefn_t _ioctl_word ;
static cdefn_t _fork_word ;
static cdefn_t _mount_word ;
static cdefn_t _umount_word ;
static cdefn_t _signal_word ;
static cdefn_t _dup2_word ;
static cdefn_t _pause_word ;
static cdefn_t _alarm_word ;
static cdefn_t _kill_word ;
static cdefn_t _pipe_word ;
static cdefn_t _getgid_word ;
static cdefn_t _times_word ;
static cdefn_t _utime_word ;
static cdefn_t _getegid_word ;
static cdefn_t _geteuid_word ;
static cdefn_t _chroot_word ;
static cdefn_t _fcntl_word ;
static cdefn_t _fchdir_word ;
static cdefn_t _fchmod_word ;
static cdefn_t _fchown_word ;
static cdefn_t _mkdir_word ;
static cdefn_t _rmdir_word ;
static cdefn_t _setpgrp_word ;
static cdefn_t _getpgrp_word ;
static cdefn_t _uname_word ;
static cdefn_t _wait_word ;
static cdefn_t _nice_word ;
static cdefn_t _flock_word ;
static cdefn_t _stderr_word ;
static cdefn_t _stdin_word ;
static cdefn_t _stdout_word ;
static cdefn_t a_number_word ;
static cdefn_t abort_word ;
static cdefn_t abs_word ;
static cdefn_t add_one_word ;
static cdefn_t add_word ;
static cdefn_t align_word ;
static cdefn_t allot_word ;
static cdefn_t and_word ;
static cdefn_t argc_word ;
static cdefn_t argv_word ;
static cdefn_t arrow_r_word ;
static cdefn_t at_word ;
static cdefn_t base_word ;
static cdefn_t branch0_word ;
static cdefn_t branch_word ;
static cdefn_t c_at_word ;
static cdefn_t c_pling_word ;
static cdefn_t cell_word ;
static cdefn_t ccount_word ;
static cdefn_t close_sq_word ;
static cdefn_t dabs_word ;
static cdefn_t drop_word ;
static cdefn_t dup_word ;
static cdefn_t equals0_word ;
static cdefn_t equals_word ;
static cdefn_t execute_word ;
static cdefn_t exit_word ;
static cdefn_t fill_word ;
static cdefn_t find_word ;
static cdefn_t fm_mod_word ;
static cdefn_t ge_word ;
static cdefn_t gt_word ;
static cdefn_t h_pad ;
static cdefn_t here_word ;
static cdefn_t in_arrow_word ;
static cdefn_t inlen_arrow_word ;
static cdefn_t inbase_arrow_word ;
static cdefn_t invert_word ;
static cdefn_t latest_word ;
static cdefn_t le_word ;
static cdefn_t less0_word ;
static cdefn_t lit_word ;
static cdefn_t lshift_word ;
static cdefn_t lt_word ;
static cdefn_t m_one_word ;
static cdefn_t m_star_word ;
static cdefn_t more0_word ;
static cdefn_t move_word ;
static cdefn_t mul_word ;
static cdefn_t not_equals_word ;
static cdefn_t notequals0_word ;
static cdefn_t one_word ;
static cdefn_t or_word ;
static cdefn_t over_word ;
static cdefn_t pad_word ;
static cdefn_t pick_word ;
static cdefn_t pling_word ;
static cdefn_t q_dup_word ;
static cdefn_t r_arrow_word ;
static cdefn_t rdrop_word ;
static cdefn_t rot_word ;
static cdefn_t rpick_word ;
static cdefn_t rshift_word ;
static cdefn_t rsp0_word ;
static cdefn_t rsp_at_word ;
static cdefn_t rsp_pling_word ;
static cdefn_t rsshift_word ;
static cdefn_t smudge_word ;
static cdefn_t source_word ;
static cdefn_t sp0_word ;
static cdefn_t sp_at_word ;
static cdefn_t sp_pling_word ;
static cdefn_t state_word ;
static cdefn_t sub_one_word ;
static cdefn_t sub_word ;
static cdefn_t swap_word ;
static cdefn_t t_drop_word ;
static cdefn_t t_dup_word ;
static cdefn_t t_over_word ;
static cdefn_t t_swap_word ;
static cdefn_t tuck_word ;
static cdefn_t two_word ;
static cdefn_t u_lt_word ;
static cdefn_t u_m_star_word ;
static cdefn_t ud_mod_word ;
static cdefn_t word_word ;
static cdefn_t xor_word ;
static cdefn_t zero_word ;
static cdefn_t immediate_word ;
static cdefn_t open_sq_word ;
//@EXPORT}

/* ======================================================================= */
/*                                SYSCALLS                                 */
/* ======================================================================= */

static void _readwrite_cb(cdefn_t* w)
{
	size_t len = dpop();
	void* ptr = (void*)dpop();
	int fd = dpop();
	int (*func)(int fd, void* ptr, size_t size) = (void*) *w->payload;

	dpush(func(fd, ptr, len));
}

static void _sys_cb(cdefn_t* w)
{
	int (*func)(void) = (void*) *w->payload;
	dpush(func());
}

static void _sys_s_cb(cdefn_t* w)
{
	int (*func)(void* s) = (void*) *w->payload;
	dpush(func((void*)dpop()));
}

static void _sys_is_cb(cdefn_t* w)
{
	void* s = (void*)dpop();
	int i = dpop();
	int (*func)(int i, void* s) = (void*) *w->payload;
	dpush(func(i, s));
}

static void _sys_si_cb(cdefn_t* w)
{
	int i = dpop();
	void* s = (void*)dpop();
	int (*func)(void* s, int i) = (void*) *w->payload;
	dpush(func(s, i));
}

static void _sys_sii_cb(cdefn_t* w)
{
	int i2 = dpop();
	int i1 = dpop();
	void* s = (void*)dpop();
	int (*func)(void* s, int i1, int i2) = (void*) *w->payload;
	dpush(func(s, i1, i2));
}

static void _sys_ss_cb(cdefn_t* w)
{
	void* s2 = (void*)dpop();
	void* s1 = (void*)dpop();
	int (*func)(void* s1, void* s2) = (void*) *w->payload;
	dpush(func(s1, s2));
}

static void _sys_ssi_cb(cdefn_t* w)
{
	cell_t i = dpop();
	void* s2 = (void*)dpop();
	void* s1 = (void*)dpop();
	int (*func)(void* s1, void* s2, int i) = (void*) *w->payload;
	dpush(func(s1, s2, i));
}

static void _sys_sss_cb(cdefn_t* w)
{
	void* s3 = (void*)dpop();
	void* s2 = (void*)dpop();
	void* s1 = (void*)dpop();
	int (*func)(void* s1, void* s2, void* s3) = (void*) *w->payload;
	dpush(func(s1, s2, s3));
}

static void _sys_i_cb(cdefn_t* w)
{
	int (*func)(cell_t i) = (void*) *w->payload;
	dpush(func(dpop()));
}

static void _sys_ii_cb(cdefn_t* w)
{
	cell_t i2 = dpop();
	cell_t i1 = dpop();
	int (*func)(int i1, int i2) = (void*) *w->payload;
	dpush(func(i1, i2));
}

static void _sys_iii_cb(cdefn_t* w)
{
	cell_t i3 = dpop();
	cell_t i2 = dpop();
	cell_t i1 = dpop();
	int (*func)(int i1, int i2, int i3) = (void*) *w->payload;
	dpush(func(i1, i2, i3));
}

static void _sys_iiv_cb(cdefn_t* w)
{
	void* s = (void*)dpop();
	cell_t i2 = dpop();
	cell_t i1 = dpop();
	dpush(ioctl(i1, i2, s));
}

/* ======================================================================= */
/*                                  WORDS                                  */
/* ======================================================================= */

static void codeword(cdefn_t* w)
{
	rpush((cell_t) pc);
	pc = (void*) &w->payload[0];
	#if 0
		printf("[bytecode %p]\n", pc);
	#endif
}

static void dataword(cdefn_t* w) { dpush((cell_t) &w->payload[0]); }
static void rvarword(cdefn_t* w) { dpush((cell_t) w->payload[0]); }
static void r2varword(cdefn_t* w) { dpush((cell_t) w->payload[0]); dpush((cell_t) w->payload[1]); }
static void wvarword(defn_t* w) { w->payload[0] = (void*) dpop(); }
static void rivarword(cdefn_t* w) { dpush(*(cell_t*) w->payload[0]); }
static void wivarword(cdefn_t* w) { *(cell_t*)(w->payload[0]) = dpop(); }

static void fill_cb(cdefn_t* w)
{
	cell_t c = dpop();
	cell_t len = dpop();
	void* ptr = (void*) dpop();

	memset(ptr, c, len);
}

static void move_cb(cdefn_t* w)
{
	cell_t len = dpop();
	cell_t dest = dpop();
	cell_t src = dpop();
	memcpy((void*)dest, (void*)src, len);
}

static void immediate_cb(cdefn_t* w)
{
	latest->name->len |= FL_IMMEDIATE;
}

static void smudge_cb(cdefn_t* w)
{
	latest->name->len ^= FL_SMUDGE;
}

static bool is_delimiter(int c, int delimiter)
{
	if (c == delimiter)
		return true;
	if ((delimiter == ' ') && (c < 32))
		return true;
	return false;
}

static void skip_ws(int delimiter)
{
	while (in_arrow < in_len)
	{
		int c = in_base[in_arrow];
		if (!is_delimiter(c, delimiter))
			break;
		in_arrow++;
	}
}

static void word_cb(cdefn_t* w)
{
	int delimiter = dpop();
	struct fstring* fs = ensure_workspace(in_len + 1);
	int count = 0;

	skip_ws(delimiter);
	if (in_arrow != in_len)
	{
		while (in_arrow < in_len)
		{
			int c = in_base[in_arrow];
			if (is_delimiter(c, delimiter))
				break;
			fs->data[count] = c;
			count++;
			in_arrow++;
		}
	}
	skip_ws(delimiter);

	#if 0
		strerr("<");
		write(2, &fs->data[0], count);
		strerr(">");
	#endif

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
		fwrite(&defn->name->data[0], 1, defn->name->len & FL__MASK, stdout);
		printf(" (name @ %p) -> %p]\n", defn->name, defn);
	#endif
	latest = defn;
}

static void find_cb(cdefn_t* w)
{
	struct fstring* name = (void*) dpop();
	cdefn_t* current = latest;

	#if 0
		printf("[find ");
		fwrite(&name->data[0], 1, name->len & 0x7f, stdout);
		printf("]\n");
	#endif

	while (current)
	{
		if (current->name
			&& !(current->name->len & FL_SMUDGE)
			&& fstreq(name, current->name))
		{
			dpush((cell_t) current);
			dpush((current->name->len & FL_IMMEDIATE) ? 1 : -1);
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
	upair_t ud = dpopd();

	while (len > 0)
	{
		unsigned int d = get_digit(*addr);
		if (d >= base)
			break;
		ud = (ud * base) + d;

		len--;
		addr++;
	}

	dpushd(ud);
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

static void t_swap_cb(cdefn_t* w)
{
	cell_t x4 = dpop();
	cell_t x3 = dpop();
	cell_t x2 = dpop();
	cell_t x1 = dpop();
	dpush(x3);
	dpush(x4);
	dpush(x1);
	dpush(x2);
}

static void execute_cb(cdefn_t* w)
{
	cdefn_t* p = (void*) dpop();
	#if 0
		printf("[execute ");
		if (p->name)
			fwrite(&p->name->data[0], 1, p->name->len & FL__MASK, stdout);
		else
			printf("(null)");
		printf(" (name @ %p) -> %p]\n", p->name, p);
	#endif
	p->code(p);
}

static void abs_cb(cdefn_t* w)
{
	cell_t d = dpop();
	if (d < 0)
		d = -d;
	dpush(d);
}

static void dabs_cb(cdefn_t* w)
{
	pair_t d = dpopd();
	if (d < 0)
		d = -d;
	dpushd(d);
}

static void tuck_cb(cdefn_t* w)
{
	cell_t x2 = dpop();
	cell_t x1 = dpop();
	dpush(x2);
	dpush(x1);
	dpush(x2);
}

static void fm_mod_cb(cdefn_t* w)
{
	cell_t den = dpop();
	pair_t num = dpopd();
	cell_t q = num / den;
	cell_t r = num % den;

	if ((num^den) <= 0)
	{
		if (r)
		{
			q -= 1;
			r += den;
		}
    }

	dpush(r);
	dpush(q);
}

static void ud_mod_cb(cdefn_t* w)
{
	upair_t den = dpopd();
	upair_t num = dpopd();
	upair_t q = num / den;
	upair_t r = num % den;
	dpushd(r);
	dpushd(q);
}

static void ccount_cb(cdefn_t* w)
{
	const char* cptr = (const char*) *daddr(0);
	if (cptr)
		dpush(strlen(cptr));
	else
		dpush(0);
}

static void E_fnf_cb(cdefn_t* w)      { panic("file not found"); }
static void _exit_cb(cdefn_t* w)      { exit(dpop()); }
static void abort_cb(cdefn_t* w)      { longjmp(onerror, 1); }
static void add_cb(cdefn_t* w)        { dpush(dpop() + dpop()); }
static void dadjust_cb(cdefn_t* w)    { dadjust((cell_t) *w->payload); }
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
static void equals0_cb(cdefn_t* w)    { dpushbool(dpop() == 0); }
static void equals_cb(cdefn_t* w)     { dpushbool(dpop() == dpop()); }
static void exit_cb(cdefn_t* w)       { pc = (void*)rpop(); }
static void ge_cb(cdefn_t* w)         { cell_t a = dpop(); cell_t b = dpop(); dpushbool(b >= a); }
static void gt_cb(cdefn_t* w)         { cell_t a = dpop(); cell_t b = dpop(); dpushbool(b > a); }
static void increment_cb(cdefn_t* w)  { dpush(dpop() + (cell_t)w->payload[0]); }
static void invert_cb(cdefn_t* w)     { dpush(~dpop()); }
static void le_cb(cdefn_t* w)         { cell_t a = dpop(); cell_t b = dpop(); dpushbool(b <= a); }
static void less0_cb(cdefn_t* w)      { dpushbool(dpop() < 0); }
static void lit_cb(cdefn_t* w)        { dpush((cell_t) *pc++); }
static void lshift_cb(cdefn_t* w)     { cell_t u = dpop(); ucell_t a = dpop(); dpush(a << u); }
static void lt_cb(cdefn_t* w)         { cell_t a = dpop(); cell_t b = dpop(); dpushbool(b < a); }
static void m_star_cb(cdefn_t* w)     { dpushd((pair_t)dpop() * (pair_t)dpop()); }
static void more0_cb(cdefn_t* w)      { dpushbool(dpop() > 0); }
static void mul_cb(cdefn_t* w)        { dpush(dpop() * dpop()); }
static void not_equals_cb(cdefn_t* w) { dpushbool(dpop() != dpop()); }
static void notequals0_cb(cdefn_t* w) { dpushbool(dpop() != 0); }
static void open_sq_cb(cdefn_t* w)    { state = 0; }
static void or_cb(cdefn_t* w)         { dpush(dpop() | dpop()); }
static void peekcon_cb(cdefn_t* w)    { dpush(*daddr((cell_t) *w->payload)); }
static void peekcon2_cb(cdefn_t* w)   { peekcon_cb(w); peekcon_cb(w); }
static void pick_cb(cdefn_t* w)       { dpush(*daddr(dpop())); }
static void pling_cb(cdefn_t* w)      { cell_t* p = (cell_t*)dpop(); *p = dpop(); }
static void pokecon_cb(cdefn_t* w)    { cell_t v = dpop(); *daddr((cell_t) *w->payload) = v; }
static void q_dup_cb(cdefn_t* w)      { cell_t a = *daddr(0); if (a) dpush(a); }
static void r_arrow_cb(cdefn_t* w)    { dpush(rpop()); }
static void radjust_cb(cdefn_t* w)    { radjust((cell_t) *w->payload); }
static void rpeekcon_cb(cdefn_t* w)   { dpush(*raddr((cell_t) *w->payload)); }
static void rpick_cb(cdefn_t* w)      { dpush(*raddr(dpop())); }
static void rpokecon_cb(cdefn_t* w)   { cell_t v = dpop(); *raddr((cell_t) *w->payload) = v; }
static void rshift_cb(cdefn_t* w)     { cell_t u = dpop(); ucell_t a = dpop(); dpush(a >> u); }
static void rsshift_cb(cdefn_t* w)    { dpush(dpop() >> 1); }
static void source_cb(cdefn_t* w)     { dpush((cell_t) in_base); dpush(in_len); }
static void sub_cb(cdefn_t* w)        { cell_t a = dpop(); cell_t b = dpop(); dpush(b - a); }
static void u_lt_cb(cdefn_t* w)       { ucell_t a = dpop(); ucell_t b = dpop(); dpushbool(b < a); }
static void u_m_star_cb(cdefn_t* w)   { dpushd((upair_t)(ucell_t)dpop() * (upair_t)(ucell_t)dpop()); }
static void xor_cb(cdefn_t* w)        { dpush(dpop() ^ dpop()); }

#define WORD(w, c, n, l, f, p...) \
	struct fstring_##w { uint8_t len; char data[sizeof(n)-1]; }; \
	static struct fstring_##w w##_name = {(sizeof(n)-1) | f, n}; \
	static cdefn_t w = { c, (struct fstring*) &w##_name, l, { p } };

#define COM(w, c, n, l, p...) WORD(w, c, n, l, 0, p)
#define IMM(w, c, n, l, p...) WORD(w, c, n, l, FL_IMMEDIATE, p)

/* A list of words go here. To add a word, add a new entry and run this file as
 * a shell script. The link field will be set correctly. (They'll also be lined
 * up for you.)
 */
//@WORDLIST
COM( E_fnf_word,         E_fnf_cb,       "E_fnf",      NULL,             (void*)0 ) //@W
COM( _O_RDONLY_word,     rvarword,       "O_RDONLY",   &E_fnf_word,      (void*)O_RDONLY ) //@W
COM( _O_RDWR_word,       rvarword,       "O_RDWR",     &_O_RDONLY_word,  (void*)O_RDWR ) //@W
COM( _O_WRONLY_word,     rvarword,       "O_WRONLY",   &_O_RDWR_word,    (void*)O_WRONLY ) //@W
COM( _chdir_word,        _sys_s_cb,      "_chdir",     &_O_WRONLY_word,  &chdir ) //@W
COM( _close_word,        _sys_i_cb,      "_close",     &_chdir_word,     &close ) //@W
COM( _create_word,       _create_cb,     "_create",    &_close_word,     ) //@W
COM( _exit_word,         _exit_cb,       "_exit",      &_create_word,    ) //@W
COM( _input_fd_word,     rvarword,       "_input_fd",  &_exit_word,      &input_fd ) //@W
COM( _mknod_word,        _sys_sii_cb,    "_mknod",     &_input_fd_word,  ) //@W
COM( _open_word,         _sys_si_cb,     "_open",      &_mknod_word,     &open ) //@W
COM( _read_word,         _readwrite_cb,  "_read",      &_open_word,      &read ) //@W
COM( _link_word,         _sys_ss_cb,     "_link",      &_read_word,      &link ) //@W
COM( _lseek_word,        _sys_iii_cb,    "_lseek",     &_link_word,      &lseek ) //@W
COM( _rename_word,       _sys_ss_cb,     "_rename",    &_lseek_word,     &rename ) //@W
COM( _sync_word,         _sys_i_cb,      "_sync",      &_rename_word,    &sync ) //@W
COM( _access_word,       _sys_si_cb,     "_access",    &_sync_word,      &access ) //@W
COM( _chmod_word,        _sys_si_cb,     "_chmod",     &_access_word,    &chmod ) //@W
COM( _chown_word,        _sys_sii_cb,    "_chown",     &_chmod_word,     &chown ) //@W
COM( _stat_word,         _sys_ss_cb,     "_stat",      &_chown_word,     &stat ) //@W
COM( _fstat_word,        _sys_is_cb,     "_fstat",     &_stat_word,      &fstat ) //@W
COM( _dup_word,          _sys_i_cb,      "_dup",       &_fstat_word,     &dup ) //@W
COM( _getpid_word,       _sys_cb,        "_getpid",    &_dup_word,       &getpid ) //@W
COM( _getppid_word,      _sys_cb,        "_getppid",   &_getpid_word,    &getppid ) //@W
COM( _getuid_word,       _sys_cb,        "_getuid",    &_getppid_word,   &getuid ) //@W
COM( _umask_word,        _sys_i_cb,      "_umask",     &_getuid_word,    &umask ) //@W
COM( _write_word,        _readwrite_cb,  "_write",     &_umask_word,     &write ) //@W
COM( _execve_word,       _sys_sss_cb,    "_execve",    &_write_word,     &execve ) //@W
COM( _setuid_word,       _sys_i_cb,      "_setuid",    &_execve_word,    &setuid ) //@W
COM( _setgid_word,       _sys_i_cb,      "_setgid",    &_setuid_word,    &setgid ) //@W
COM( _ioctl_word,        _sys_iiv_cb,    "_ioctl",     &_setgid_word,    &ioctl ) //@W
COM( _fork_word,         _sys_cb,        "_fork",      &_ioctl_word,     &fork ) //@W
COM( _mount_word,        _sys_ssi_cb,    "_mount",     &_fork_word,      &mount ) //@W
COM( _umount_word,       _sys_s_cb,      "_umount",    &_mount_word,     &umount ) //@W
COM( _signal_word,       _sys_ss_cb,     "_signal",    &_umount_word,    &signal ) //@W
COM( _dup2_word,         _sys_ii_cb,     "_dup2",      &_signal_word,    &dup2 ) //@W
COM( _pause_word,        _sys_cb,        "_pause",     &_dup2_word,      &pause ) //@W
COM( _alarm_word,        _sys_i_cb,      "_alarm",     &_pause_word,     &alarm ) //@W
COM( _kill_word,         _sys_ii_cb,     "_kill",      &_alarm_word,     &kill ) //@W
COM( _pipe_word,         _sys_si_cb,     "_pipe",      &_kill_word,      &pipe ) //@W
COM( _getgid_word,       _sys_cb,        "_getgid",    &_pipe_word,      &getgid ) //@W
COM( _times_word,        _sys_s_cb,      "_times",     &_getgid_word,    &times ) //@W
COM( _utime_word,        _sys_ss_cb,     "_utime",     &_times_word,     &utime ) //@W
COM( _getegid_word,      _sys_cb,        "_getegid",   &_utime_word,     &getegid ) //@W
COM( _geteuid_word,      _sys_cb,        "_geteuid",   &_getegid_word,   &geteuid ) //@W
COM( _chroot_word,       _sys_s_cb,      "_chroot",    &_geteuid_word,   &chroot ) //@W
COM( _fcntl_word,        _sys_iiv_cb,    "_fnctl",     &_chroot_word,    &fcntl ) //@W
COM( _fchdir_word,       _sys_i_cb,      "_fchdir",    &_fcntl_word,     &fchdir ) //@W
COM( _fchmod_word,       _sys_ii_cb,     "_fchmod",    &_fchdir_word,    &fchmod ) //@W
COM( _fchown_word,       _sys_ii_cb,     "_fchown",    &_fchmod_word,    &fchown ) //@W
COM( _mkdir_word,        _sys_si_cb,     "_mkdir",     &_fchown_word,    &mkdir ) //@W
COM( _rmdir_word,        _sys_s_cb,      "_rmdir",     &_mkdir_word,     &rmdir ) //@W
COM( _setpgrp_word,      _sys_ii_cb,     "_setpgrp",   &_rmdir_word,     &setpgrp ) //@W
COM( _getpgrp_word,      _sys_i_cb,      "_getpgrp",   &_setpgrp_word,   &getpgrp ) //@W
COM( _uname_word,        _sys_s_cb,      "_uname",     &_getpgrp_word,   &uname ) //@W
COM( _wait_word,         _sys_s_cb,      "_wait",      &_uname_word,     &wait ) //@W
COM( _nice_word,         _sys_i_cb,      "_nice",      &_wait_word,      &nice ) //@W
COM( _flock_word,        _sys_ii_cb,     "_flock",     &_nice_word,      &flock ) //@W
COM( _stderr_word,       rvarword,       "_stderr",    &_flock_word,     (void*)2 ) //@W
COM( _stdin_word,        rvarword,       "_stdin",     &_stderr_word,    (void*)0 ) //@W
COM( _stdout_word,       rvarword,       "_stdout",    &_stdin_word,     (void*)1 ) //@W
COM( a_number_word,      a_number_cb,    ">NUMBER",    &_stdout_word,    ) //@W
COM( abort_word,         abort_cb,       "ABORT",      &a_number_word,   ) //@W
COM( abs_word,           abs_cb,         "ABS",        &abort_word,      ) //@W
COM( add_one_word,       increment_cb,   "1+",         &abs_word,        (void*)1 ) //@W
COM( add_word,           add_cb,         "+",          &add_one_word,    ) //@W
COM( align_word,         align_cb,       "ALIGN",      &add_word,        ) //@W
COM( allot_word,         allot_cb,       "ALLOT",      &align_word,      ) //@W
COM( and_word,           and_cb,         "AND",        &allot_word,      ) //@W
COM( argc_word,          rvarword,       "ARGC",       &and_word,        &global_argc ) //@W
COM( argv_word,          rvarword,       "ARGV",       &argc_word,       &global_argv ) //@W
COM( arrow_r_word,       arrow_r_cb,     ">R",         &argv_word,       ) //@W
COM( at_word,            at_cb,          "@",          &arrow_r_word,    ) //@W
COM( base_word,          rvarword,       "BASE",       &at_word,         &base ) //@W
COM( branch0_word,       branchif_cb,    "0BRANCH",    &base_word,       (void*)0 ) //@W
COM( branch_word,        branch_cb,      "BRANCH",     &branch0_word,    ) //@W
COM( c_at_word,          c_at_cb,        "C@",         &branch_word,     ) //@W
COM( c_pling_word,       c_pling_cb,     "C!",         &c_at_word,       ) //@W
COM( cell_word,          rvarword,       "CELL",       &c_pling_word,    (void*)CELL ) //@W
COM( ccount_word,        ccount_cb,      "CCOUNT",     &cell_word,       ) //@W
COM( close_sq_word,      close_sq_cb,    "]",          &ccount_word,     ) //@W
COM( dabs_word,          dabs_cb,        "DABS",       &close_sq_word,   ) //@W
COM( drop_word,          dadjust_cb,     "DROP",       &dabs_word,       (void*)-1 ) //@W
COM( dup_word,           peekcon_cb,     "DUP",        &drop_word,       (void*)0 ) //@W
COM( equals0_word,       equals0_cb,     "0=",         &dup_word,        ) //@W
COM( equals_word,        equals_cb,      "=",          &equals0_word,    ) //@W
COM( execute_word,       execute_cb,     "EXECUTE",    &equals_word,     ) //@W
COM( exit_word,          exit_cb,        "EXIT",       &execute_word,    ) //@W
COM( fill_word,          fill_cb,        "FILL",       &exit_word,       ) //@W
COM( find_word,          find_cb,        "FIND",       &fill_word,       ) //@W
COM( fm_mod_word,        fm_mod_cb,      "FM/MOD",     &find_word,       ) //@W
COM( ge_word,            ge_cb,          ">=",         &fm_mod_word,     ) //@W
COM( gt_word,            gt_cb,          ">",          &ge_word,         ) //@W
COM( h_pad,              rvarword,       "#PAD",       &gt_word,         (void*)PAD_SIZE ) //@W
COM( here_word,          rivarword,      "HERE",       &h_pad,           &here ) //@W
COM( in_arrow_word,      rvarword,       ">IN",        &here_word,       &in_arrow ) //@W
COM( inlen_arrow_word,   rvarword,       ">INLEN",     &in_arrow_word,   &in_len ) //@W
COM( inbase_arrow_word,  rvarword,       ">INBASE",    &inlen_arrow_word, &in_base ) //@W
COM( invert_word,        invert_cb,      "INVERT",     &inbase_arrow_word, ) //@W
COM( latest_word,        rvarword,       "LATEST",     &invert_word,     &latest ) //@W
COM( le_word,            le_cb,          "<=",         &latest_word,     ) //@W
COM( less0_word,         less0_cb,       "0<",         &le_word,         ) //@W
COM( lit_word,           lit_cb,         "LIT",        &less0_word,      ) //@W
COM( lshift_word,        lshift_cb,      "LSHIFT",     &lit_word,        ) //@W
COM( lt_word,            lt_cb,          "<",          &lshift_word,     ) //@W
COM( m_one_word,         rvarword,       "-1",         &lt_word,         (void*)-1 ) //@W
COM( m_star_word,        m_star_cb,      "M*",         &m_one_word,      ) //@W
COM( more0_word,         more0_cb,       "0>",         &m_star_word,     ) //@W
COM( move_word,          move_cb,        "MOVE",       &more0_word,      ) //@W
COM( mul_word,           mul_cb,         "*",          &move_word,       ) //@W
COM( not_equals_word,    not_equals_cb,  "<>",         &mul_word,        ) //@W
COM( notequals0_word,    notequals0_cb,  "0<>",        &not_equals_word, ) //@W
COM( one_word,           rvarword,       "1",          &notequals0_word, (void*)1 ) //@W
COM( or_word,            or_cb,          "OR",         &one_word,        ) //@W
COM( over_word,          peekcon_cb,     "OVER",       &or_word,         (void*)1 ) //@W
COM( pad_word,           rivarword,      "PAD",        &over_word,       &here ) //@W
COM( pick_word,          pick_cb,        "PICK",       &pad_word,        ) //@W
COM( pling_word,         pling_cb,       "!",          &pick_word,       ) //@W
COM( q_dup_word,         q_dup_cb,       "?DUP",       &pling_word,      ) //@W
COM( r_arrow_word,       r_arrow_cb,     "R>",         &q_dup_word,      ) //@W
COM( rdrop_word,         radjust_cb,     "RDROP",      &r_arrow_word,    (void*)-1 ) //@W
COM( rot_word,           rot_cb,         "ROT",        &rdrop_word,      ) //@W
COM( rpick_word,         rpick_cb,       "RPICK",      &rot_word,        ) //@W
COM( rshift_word,        rshift_cb,      "RSHIFT",     &rpick_word,      ) //@W
COM( rsp0_word,          rvarword,       "RSP0",       &rshift_word,     rstack+RSTACKSIZE ) //@W
COM( rsp_at_word,        rivarword,      "RSP@",       &rsp0_word,       &rsp ) //@W
COM( rsp_pling_word,     wivarword,      "RSP!",       &rsp_at_word,     &rsp ) //@W
COM( rsshift_word,       rsshift_cb,     "2/",         &rsp_pling_word,  ) //@W
COM( smudge_word,        smudge_cb,      "SMUDGE",     &rsshift_word,    ) //@W
COM( source_word,        source_cb,      "SOURCE",     &smudge_word,     ) //@W
COM( sp0_word,           rvarword,       "SP0",        &source_word,     dstack+DSTACKSIZE ) //@W
COM( sp_at_word,         rivarword,      "SP@",        &sp0_word,        &dsp ) //@W
COM( sp_pling_word,      wivarword,      "SP!",        &sp_at_word,      &dsp ) //@W
COM( state_word,         rvarword,       "STATE",      &sp_pling_word,   &state ) //@W
COM( sub_one_word,       increment_cb,   "1-",         &state_word,      (void*)-1 ) //@W
COM( sub_word,           sub_cb,         "-",          &sub_one_word,    ) //@W
COM( swap_word,          swap_cb,        "SWAP",       &sub_word,        ) //@W
COM( t_drop_word,        dadjust_cb,     "2DROP",      &swap_word,       (void*)-2 ) //@W
COM( t_dup_word,         peekcon2_cb,    "2DUP",       &t_drop_word,     (void*)1 ) //@W
COM( t_over_word,        peekcon2_cb,    "2OVER",      &t_dup_word,      (void*)3 ) //@W
COM( t_swap_word,        t_swap_cb,      "2SWAP",      &t_over_word,     ) //@W
COM( tuck_word,          tuck_cb,        "TUCK",       &t_swap_word,     ) //@W
COM( two_word,           rvarword,       "2",          &tuck_word,       (void*)2 ) //@W
COM( u_lt_word,          u_lt_cb,        "U<",         &two_word,        ) //@W
COM( u_m_star_word,      u_m_star_cb,    "UM*",        &u_lt_word,       ) //@W
COM( ud_mod_word,        ud_mod_cb,      "UD/MOD",     &u_m_star_word,   ) //@W
COM( word_word,          word_cb,        "WORD",       &ud_mod_word,     ) //@W
COM( xor_word,           xor_cb,         "XOR",        &word_word,       ) //@W
COM( zero_word,          rvarword,       "0",          &xor_word,        (void*)0 ) //@W
IMM( immediate_word,     immediate_cb,   "IMMEDIATE",  &zero_word,       ) //@W
IMM( open_sq_word,       open_sq_cb,     "[",          &immediate_word,  ) //@W

//@C \ IMMEDIATE
//   10 WORD DROP
IMM( _5c__word, codeword, "\\", &open_sq_word, (void*)&lit_word, (void*)10, (void*)&word_word, (void*)&drop_word, (void*)&exit_word )

//@C CELLS
//  CELL *
COM( cells_word, codeword, "CELLS", &_5c__word, (void*)&cell_word, (void*)&mul_word, (void*)&exit_word )

//@C CELL+
//  CELL +
COM( cell_2b__word, codeword, "CELL+", &cells_word, (void*)&cell_word, (void*)&add_word, (void*)&exit_word )

//@C CELL-
//  CELL -
COM( cell_2d__word, codeword, "CELL-", &cell_2b__word, (void*)&cell_word, (void*)&sub_word, (void*)&exit_word )

//@C CHAR+
//  1+
COM( char_2b__word, codeword, "CHAR+", &cell_2d__word, (void*)&add_one_word, (void*)&exit_word )

//@C CHARS
// \ nop!
COM( chars_word, codeword, "CHARS", &char_2b__word, (void*)&exit_word )

//@C ALIGNED
// \ addr -- aligned-addr
//   DUP                               \ -- end-r-addr end-r-addr
//   CELL SWAP -
//   CELL 1- AND                       \ -- end-r-addr offset
//   +                                 \ -- aligned-end-r-addr
COM( aligned_word, codeword, "ALIGNED", &chars_word, (void*)&dup_word, (void*)&cell_word, (void*)&swap_word, (void*)&sub_word, (void*)&cell_word, (void*)&sub_one_word, (void*)&and_word, (void*)&add_word, (void*)&exit_word )

//@C -ROT
//   ROT ROT
COM( _2d_rot_word, codeword, "-ROT", &aligned_word, (void*)&rot_word, (void*)&rot_word, (void*)&exit_word )

//@C +!@
// \ n addr -- newval
//   DUP @                             \ -- n addr val
//   ROT                               \ -- addr val n
//   +                                 \ -- addr new-val
//   DUP ROT                           \ -- new-val addr addr
//   !
COM( _2b__21__40__word, codeword, "+!@", &_2d_rot_word, (void*)&dup_word, (void*)&at_word, (void*)&rot_word, (void*)&add_word, (void*)&dup_word, (void*)&rot_word, (void*)&pling_word, (void*)&exit_word )

//@C +!
// \ n addr --
//   +!@ DROP
COM( _2b__21__word, codeword, "+!", &_2b__21__40__word, (void*)&_2b__21__40__word, (void*)&drop_word, (void*)&exit_word )

//@C ,
//  HERE !
//  CELL ALLOT
COM( _2c__word, codeword, ",", &_2b__21__word, (void*)&here_word, (void*)&pling_word, (void*)&cell_word, (void*)&allot_word, (void*)&exit_word )

//@C C,
//  HERE C!
//  1 ALLOT
COM( c_2c__word, codeword, "C,", &_2c__word, (void*)&here_word, (void*)&c_pling_word, (void*)&one_word, (void*)&allot_word, (void*)&exit_word )

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
//   HERE C!
//   _stdout HERE 1 _write DROP
COM( emit_word, codeword, "EMIT", &create_word, (void*)&here_word, (void*)&c_pling_word, (void*)&_stdout_word, (void*)&here_word, (void*)&one_word, (void*)&_write_word, (void*)&drop_word, (void*)&exit_word )

//@C TYPE
// \ ( addr n -- )
//   _stdout -ROT _write DROP
COM( type_word, codeword, "TYPE", &emit_word, (void*)&_stdout_word, (void*)&_2d_rot_word, (void*)&_write_word, (void*)&drop_word, (void*)&exit_word )

//@C CR
//   10 EMIT
COM( cr_word, codeword, "CR", &type_word, (void*)&lit_word, (void*)10, (void*)&emit_word, (void*)&exit_word )

//@C BL
//   32
COM( bl_word, codeword, "BL", &cr_word, (void*)&lit_word, (void*)32, (void*)&exit_word )

//@C SPACE
//   BL EMIT
COM( space_word, codeword, "SPACE", &bl_word, (void*)&bl_word, (void*)&emit_word, (void*)&exit_word )

//@C SPACES
// \ n --
//   BEGIN
//     DUP 0>
//   WHILE
//     SPACE 1-
//   REPEAT
//   DROP
COM( spaces_word, codeword, "SPACES", &space_word, (void*)&dup_word, (void*)&more0_word, (void*)&branch0_word, (void*)(&spaces_word.payload[0] + 8), (void*)&space_word, (void*)&sub_one_word, (void*)&branch_word, (void*)(&spaces_word.payload[0] + 0), (void*)&drop_word, (void*)&exit_word )

//@C NEGATE
//   0 SWAP -
COM( negate_word, codeword, "NEGATE", &spaces_word, (void*)&zero_word, (void*)&swap_word, (void*)&sub_word, (void*)&exit_word )

//@C TRUE
//   1
COM( true_word, codeword, "TRUE", &negate_word, (void*)&one_word, (void*)&exit_word )

//@C FALSE
//   0
COM( false_word, codeword, "FALSE", &true_word, (void*)&zero_word, (void*)&exit_word )

//@C BYE
//   0 _exit
COM( bye_word, codeword, "BYE", &false_word, (void*)&zero_word, (void*)&_exit_word, (void*)&exit_word )

//@C NIP
// \ x y -- y
//  SWAP DROP
COM( nip_word, codeword, "NIP", &bye_word, (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C KEY
// \ Read one character from the terminal. -1 is returned on error.
// \ -- key
//  0 SP@                    \ -- X addr
//  _input_fd @ OVER 1 _read \ -- X addr result
//  1 <> IF
//    2DROP -1
//  ELSE
//    C@ NIP
//  THEN
COM( key_word, codeword, "KEY", &nip_word, (void*)&zero_word, (void*)&sp_at_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&over_word, (void*)&one_word, (void*)&_read_word, (void*)&one_word, (void*)&not_equals_word, (void*)&branch0_word, (void*)(&key_word.payload[0] + 15), (void*)&t_drop_word, (void*)&m_one_word, (void*)&branch_word, (void*)(&key_word.payload[0] + 17), (void*)&c_at_word, (void*)&nip_word, (void*)&exit_word )

//@C ACCEPT
// \ Read characters from the terminal.
// \ addr max -- count
//  OVER + OVER              \ -- addr max-addr addr
//
//  \ Read first key.
//  KEY                      \ -- addr max-addr ptr
//
//  \ If this fails, report failure.
//  DUP -1 = IF              \ -- addr max-addr ptr key
//    DROP 2DROP -1 EXIT
//  THEN
//
//  BEGIN                    \ -- addr max-addr ptr key
//    DUP 10 <>              \ -- addr max-addr-ptr key not-cr
//    2SWAP 2DUP <> >R 2SWAP R>
//    AND
//  WHILE                    \ -- addr max-addr ptr key
//    OVER C! 1+             \ -- addr max-addr ptr+1
//    KEY                    \ -- addr max-addr ptr+1 key
//    DUP -1 = IF
//      \ EOF. Pretend it's a newline.
//      DROP 10
//    THEN
//  REPEAT
//  DROP NIP SWAP            \ -- ptr+1 addr
//  -                        \ -- count
COM( accept_word, codeword, "ACCEPT", &key_word, (void*)&over_word, (void*)&add_word, (void*)&over_word, (void*)&key_word, (void*)&dup_word, (void*)&m_one_word, (void*)&equals_word, (void*)&branch0_word, (void*)(&accept_word.payload[0] + 13), (void*)&drop_word, (void*)&t_drop_word, (void*)&m_one_word, (void*)&exit_word, (void*)&dup_word, (void*)&lit_word, (void*)10, (void*)&not_equals_word, (void*)&t_swap_word, (void*)&t_dup_word, (void*)&not_equals_word, (void*)&arrow_r_word, (void*)&t_swap_word, (void*)&r_arrow_word, (void*)&and_word, (void*)&branch0_word, (void*)(&accept_word.payload[0] + 40), (void*)&over_word, (void*)&c_pling_word, (void*)&add_one_word, (void*)&key_word, (void*)&dup_word, (void*)&m_one_word, (void*)&equals_word, (void*)&branch0_word, (void*)(&accept_word.payload[0] + 38), (void*)&drop_word, (void*)&lit_word, (void*)10, (void*)&branch_word, (void*)(&accept_word.payload[0] + 13), (void*)&drop_word, (void*)&nip_word, (void*)&swap_word, (void*)&sub_word, (void*)&exit_word )

//@C REFILL
//  \ Read a line from the terminal.
//  [&lit_word] [input_buffer]
//  [&lit_word] [MAX_LINE_LENGTH] 
//  ACCEPT                   \ -- len
//
//  \ Is this the end?
//  DUP 0< IF
//    DROP 0 EXIT
//  THEN
//
//  \ Set up in the input buffer variables.
//  >INLEN !
//  [&lit_word] [input_buffer] >INBASE !
//  0 >IN !
//
//  \ We must succeed!
//  1
COM( refill_word, codeword, "REFILL", &accept_word, (void*)(&lit_word), (void*)(input_buffer), (void*)(&lit_word), (void*)(MAX_LINE_LENGTH), (void*)&accept_word, (void*)&dup_word, (void*)&less0_word, (void*)&branch0_word, (void*)(&refill_word.payload[0] + 12), (void*)&drop_word, (void*)&zero_word, (void*)&exit_word, (void*)&inlen_arrow_word, (void*)&pling_word, (void*)(&lit_word), (void*)(input_buffer), (void*)&inbase_arrow_word, (void*)&pling_word, (void*)&zero_word, (void*)&in_arrow_word, (void*)&pling_word, (void*)&one_word, (void*)&exit_word )

//@C COUNT
// \ ( c-addr -- addr len )
//   DUP C@ SWAP 1+ SWAP
COM( count_word, codeword, "COUNT", &refill_word, (void*)&dup_word, (void*)&c_at_word, (void*)&swap_word, (void*)&add_one_word, (void*)&swap_word, (void*)&exit_word )

//@C ( IMMEDIATE
//   41 WORD DROP
IMM( _28__word, codeword, "(", &count_word, (void*)&lit_word, (void*)41, (void*)&word_word, (void*)&drop_word, (void*)&exit_word )

//@C number HIDDEN
// \ val addr len -- val addr len
//   0 -ROT                            \ -- val 0 addr len
//   >NUMBER                           \ -- val 0 addr len
//   ROT DROP                          \ -- val 0 addr
COM( number_word, codeword, "", &_28__word, (void*)&zero_word, (void*)&_2d_rot_word, (void*)&a_number_word, (void*)&rot_word, (void*)&drop_word, (void*)&exit_word )

//@C numberthenneg HIDDEN
// \ val addr len -- val addr len
// \ As number, but negates the result.
//   number
//   ROT NEGATE -ROT
COM( numberthenneg_word, codeword, "", &number_word, (void*)&number_word, (void*)&rot_word, (void*)&negate_word, (void*)&_2d_rot_word, (void*)&exit_word )

//@C snumber HIDDEN
// \ val addr len -- val addr len
// \ As >NUMBER, but copes with a leading -.
//
//   SWAP DUP C@                       \ -- val len addr byte
//   ROT SWAP                          \ -- val addr len byte
//   45 = IF
//     1- SWAP 1+ SWAP
//     numberthenneg
//   ELSE
//     number
//   THEN
COM( snumber_word, codeword, "", &numberthenneg_word, (void*)&swap_word, (void*)&dup_word, (void*)&c_at_word, (void*)&rot_word, (void*)&swap_word, (void*)&lit_word, (void*)45, (void*)&equals_word, (void*)&branch0_word, (void*)(&snumber_word.payload[0] + 17), (void*)&sub_one_word, (void*)&swap_word, (void*)&add_one_word, (void*)&swap_word, (void*)&numberthenneg_word, (void*)&branch_word, (void*)(&snumber_word.payload[0] + 18), (void*)&number_word, (void*)&exit_word )

//@C LITERAL IMMEDIATE
//   [&lit_word] [&lit_word] , ,
IMM( literal_word, codeword, "LITERAL", &snumber_word, (void*)(&lit_word), (void*)(&lit_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&exit_word )

CONST_STRING(unrecognised_word_msg, "panic: unrecognised word: ");
//@C E_enoent HIDDEN
// \ c-addr --
//   [&lit_word] [unrecognised_word_msg]
//   [&lit_word] [sizeof(unrecognised_word_msg)]
//   TYPE
//
//   DROP DROP                         \ -- addr
//   COUNT                             \ -- c-addr len
//   TYPE                              \ --
//
//   CR
//   ABORT
COM( e_enoent_word, codeword, "", &literal_word, (void*)(&lit_word), (void*)(unrecognised_word_msg), (void*)(&lit_word), (void*)(sizeof(unrecognised_word_msg)), (void*)&type_word, (void*)&drop_word, (void*)&drop_word, (void*)&count_word, (void*)&type_word, (void*)&cr_word, (void*)&abort_word, (void*)&exit_word )

CONST_STRING(end_of_line_msg, "panic: unexpected end of line");
//@C E_eol HIDDEN
// \ --
//   [&lit_word] [unrecognised_word_msg]
//   [&lit_word] [sizeof(unrecognised_word_msg)]
//   TYPE
//   CR
//   ABORT
COM( e_eol_word, codeword, "", &e_enoent_word, (void*)(&lit_word), (void*)(unrecognised_word_msg), (void*)(&lit_word), (void*)(sizeof(unrecognised_word_msg)), (void*)&type_word, (void*)&cr_word, (void*)&abort_word, (void*)&exit_word )

//@C INTERPRET_NUM HIDDEN
// \ Evaluates a number, or perish in the attempt.
// \ ( c-addr -- value )
//   DUP
//
//   \ Get the length of the input string.
//   DUP C@                            \ -- addr addr len
//
//   \ The address we've got is a counted string; we want the address of the data.
//   SWAP 1+                           \ -- addr len addr+1
//
//   \ Initialise the accumulator.
//   0 SWAP ROT                        \ -- addr 0 addr+1 len
//
//   \ Parse!
//   snumber                           \ -- addr val addr+1 len
//
//   \ We must consume all bytes to succeed.
//   IF E_enoent THEN
//
//   \ Huzzah!                         \ -- addr val addr+1
//   DROP SWAP DROP                    \ -- val
COM( interpret_num_word, codeword, "", &e_eol_word, (void*)&dup_word, (void*)&dup_word, (void*)&c_at_word, (void*)&swap_word, (void*)&add_one_word, (void*)&zero_word, (void*)&swap_word, (void*)&rot_word, (void*)&snumber_word, (void*)&branch0_word, (void*)(&interpret_num_word.payload[0] + 12), (void*)&e_enoent_word, (void*)&drop_word, (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C COMPILE_NUM HIDDEN
// \ Compiles a number (or at least, a word we don't recognise).
// \ ( c-addr -- )
//   \ The interpreter does the heavy lifting for us!
//   INTERPRET_NUM LITERAL
COM( compile_num_word, codeword, "", &interpret_num_word, (void*)&interpret_num_word, (void*)&literal_word, (void*)&exit_word )

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
//     \ End of the buffer? (WORD returns a zero-length result.) If so, return.
//     C@ 0= IF EXIT THEN
//
//     \ Look up the word.
//     HERE FIND                     \ -- addr kind
//
//     \ What is it? Calculate an offset into the lookup table.
//     1+ CELLS
//     STATE @ 3 CELLS *
//     +                               \ -- addr offset
//
//     \ Look up the right word and run it.
//     [&lit_word] [interpreter_table] + @ EXECUTE \ -- addr
//   AGAIN
COM( interpret_word, codeword, "INTERPRET", &compile_num_word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&c_at_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&interpret_word.payload[0] + 8), (void*)&exit_word, (void*)&here_word, (void*)&find_word, (void*)&add_one_word, (void*)&cells_word, (void*)&state_word, (void*)&at_word, (void*)&lit_word, (void*)3, (void*)&cells_word, (void*)&mul_word, (void*)&add_word, (void*)(&lit_word), (void*)(interpreter_table), (void*)&add_word, (void*)&at_word, (void*)&execute_word, (void*)&branch_word, (void*)(&interpret_word.payload[0] + 0), (void*)&exit_word )

//@C EVALUATE
// \ Parses a user-specified string and executes the words therein.
// \ c-addr len --
//
//  \ Save the old variables (on the return stack so they don't interfere
//  \ with the EVALUATEd return code).
//  >INLEN @ >R              \ c-addr len -- 
//  >INBASE @ >R             \ c-addr len --
//  >IN @ >R                 \ c-addr len --
//
//  \ Set up the new ones.
//  >INLEN !                 \ c-addr --
//  >INBASE !                \ --
//  0 >IN !
//
//  \ Run the code.
//  INTERPRET
//
//  \ Now put the variables back again.
//  R> >IN !                 \ --
//  R> >INBASE !             \ --
//  R> >INLEN !              \ --
COM( evaluate_word, codeword, "EVALUATE", &interpret_word, (void*)&inlen_arrow_word, (void*)&at_word, (void*)&arrow_r_word, (void*)&inbase_arrow_word, (void*)&at_word, (void*)&arrow_r_word, (void*)&in_arrow_word, (void*)&at_word, (void*)&arrow_r_word, (void*)&inlen_arrow_word, (void*)&pling_word, (void*)&inbase_arrow_word, (void*)&pling_word, (void*)&zero_word, (void*)&in_arrow_word, (void*)&pling_word, (void*)&interpret_word, (void*)&r_arrow_word, (void*)&in_arrow_word, (void*)&pling_word, (void*)&r_arrow_word, (void*)&inbase_arrow_word, (void*)&pling_word, (void*)&r_arrow_word, (void*)&inlen_arrow_word, (void*)&pling_word, (void*)&exit_word )

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
COM( interact_word, codeword, "INTERACT", &evaluate_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&_stdin_word, (void*)&equals_word, (void*)&branch0_word, (void*)(&interact_word.payload[0] + 11), (void*)(&lit_word), (void*)(prompt_msg), (void*)&lit_word, (void*)4, (void*)&type_word, (void*)&refill_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&interact_word.payload[0] + 16), (void*)&exit_word, (void*)&interpret_word, (void*)&branch_word, (void*)(&interact_word.payload[0] + 0), (void*)&exit_word )

//@C QUIT
//  SP0 SP!
//  RSP0 RSP!
//  0 STATE !
//  INTERACT BYE
COM( quit_word, codeword, "QUIT", &interact_word, (void*)&sp0_word, (void*)&sp_pling_word, (void*)&rsp0_word, (void*)&rsp_pling_word, (void*)&zero_word, (void*)&state_word, (void*)&pling_word, (void*)&interact_word, (void*)&bye_word, (void*)&exit_word )

//@C READ-FILE
//   \ Read the filename.
//   32 WORD
//
//   \ Turn it into a C string.
//   DUP C@ + 1+ 0 SWAP C!
//
//   \ Open the new file.
//   HERE 1+ O_RDONLY _open
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
COM( read_2d_file_word, codeword, "READ-FILE", &quit_word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&dup_word, (void*)&c_at_word, (void*)&add_word, (void*)&add_one_word, (void*)&zero_word, (void*)&swap_word, (void*)&c_pling_word, (void*)&here_word, (void*)&add_one_word, (void*)&_O_RDONLY_word, (void*)&_open_word, (void*)&dup_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&read_2d_file_word.payload[0] + 19), (void*)&E_fnf_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&swap_word, (void*)&_input_fd_word, (void*)&pling_word, (void*)&interact_word, (void*)&_input_fd_word, (void*)&at_word, (void*)&_close_word, (void*)&drop_word, (void*)&_input_fd_word, (void*)&pling_word, (void*)&exit_word )

//@C :
//  \ Create the word itself.
//  CREATE
//
//  \ It musn't be findable until compilation end.
//  SMUDGE
//
//  \ Turn it into a runnable word.
//  [&lit_word] [codeword] LATEST @ !
//
//  \ Switch to compilation mode.
//  ]
COM( _3a__word, codeword, ":", &read_2d_file_word, (void*)&create_word, (void*)&smudge_word, (void*)(&lit_word), (void*)(codeword), (void*)&latest_word, (void*)&at_word, (void*)&pling_word, (void*)&close_sq_word, (void*)&exit_word )

//@C DOES>
// \ Turns a simple CREATE word into a code word.
//   \ Get the address of name field of the CREATEd word.
//   LATEST @ CELL+                        \ addr --
//
//   \ Fetch it.
//   DUP @                                 \ addr name --
//
//   \ Set the field to null (we don't want the old word being looked up).
//   0                                     \ addr name 0 --
//   ROT                                   \ name 0 addr --
//   !                                     \ name --
//
//   \ Remember what the last word was.
//   LATEST @ SWAP                         \ oldword name --
//
//   \ Ensure alignment, and create the new low level header.
//   ALIGN [&_create_word]                 \ oldword --
//
//   \ Turn it into a runnable word.
//   [&lit_word] [codeword] LATEST @ !     \ oldword
//
//   \ The first thing it does is call the old word (to push the data area
//   \ defined with CREATE).
//   ,
//   
//   \ Then it's going to jump to the bytecode in the word containing the
//   \ DOES>. (That's the one we're being called from.)
//   [&lit_word] [&branch_word] ,
//   R> ,
//
//   \ Meanwhile, the word we're being called from is going to simply exit.
//   \ (So, not running the remainder of our bytecode.) As we've popped
//   \ DOES>'s return address, exiting from here will do that automatically.
COM( does_3e__word, codeword, "DOES>", &_3a__word, (void*)&latest_word, (void*)&at_word, (void*)&cell_2b__word, (void*)&dup_word, (void*)&at_word, (void*)&zero_word, (void*)&rot_word, (void*)&pling_word, (void*)&latest_word, (void*)&at_word, (void*)&swap_word, (void*)&align_word, (void*)(&_create_word), (void*)(&lit_word), (void*)(codeword), (void*)&latest_word, (void*)&at_word, (void*)&pling_word, (void*)&_2c__word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&r_arrow_word, (void*)&_2c__word, (void*)&exit_word )

//@C ; IMMEDIATE
//  [&lit_word] [&exit_word] ,
//  SMUDGE
//  [
IMM( _3b__word, codeword, ";", &does_3e__word, (void*)(&lit_word), (void*)(&exit_word), (void*)&_2c__word, (void*)&smudge_word, (void*)&open_sq_word, (void*)&exit_word )

//@C >BODY
//  3 CELLS +
COM( _3e_body_word, codeword, ">BODY", &_3b__word, (void*)&lit_word, (void*)3, (void*)&cells_word, (void*)&add_word, (void*)&exit_word )

//@C CONSTANT
// \ ( value -- )
//  CREATE
//  [&lit_word] [rvarword] LATEST @ !
//  ,
COM( constant_word, codeword, "CONSTANT", &_3e_body_word, (void*)&create_word, (void*)(&lit_word), (void*)(rvarword), (void*)&latest_word, (void*)&at_word, (void*)&pling_word, (void*)&_2c__word, (void*)&exit_word )

//@C VARIABLE
//  CREATE 0 ,
COM( variable_word, codeword, "VARIABLE", &constant_word, (void*)&create_word, (void*)&zero_word, (void*)&_2c__word, (void*)&exit_word )

//@C 'andkind HIDDEN
// \ Picks up a word from the input stream and returns its xt and type.
// \ Aborts on error.
// \ -- xt kind
//   32 WORD                           \ -- c-addr
//
//   \ End of the buffer? If so, panic..
//   C@ 0= IF E_eol THEN               \ --
//
//   \ Look up the word.
//   HERE FIND                       \ -- addr kind
//
//   \ Not found?
//   DUP 0= IF E_enoent THEN               \ -- addr
//
COM( _27_andkind_word, codeword, "", &variable_word, (void*)&lit_word, (void*)32, (void*)&word_word, (void*)&c_at_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&_27_andkind_word.payload[0] + 8), (void*)&e_eol_word, (void*)&here_word, (void*)&find_word, (void*)&dup_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&_27_andkind_word.payload[0] + 15), (void*)&e_enoent_word, (void*)&exit_word )

//@C '
//   'andkind DROP
COM( _27__word, codeword, "'", &_27_andkind_word, (void*)&_27_andkind_word, (void*)&drop_word, (void*)&exit_word )

//@C ['] IMMEDIATE
//   ' LITERAL
IMM( _5b__27__5d__word, codeword, "[']", &_27__word, (void*)&_27__word, (void*)&literal_word, (void*)&exit_word )

//@C POSTPONE IMMEDIATE
//   'andkind                          \ -- xt kind
//   -1 = IF                           \ -- xt
//     \ Normal word --- generate code to compile it.
//     LITERAL [&lit_word] [&_2c__word] ,
//   ELSE
//     \ Immediate word --- generate code to run it.
//     ,
//   THEN
IMM( postpone_word, codeword, "POSTPONE", &_5b__27__5d__word, (void*)&_27_andkind_word, (void*)&m_one_word, (void*)&equals_word, (void*)&branch0_word, (void*)(&postpone_word.payload[0] + 11), (void*)&literal_word, (void*)(&lit_word), (void*)(&_2c__word), (void*)&_2c__word, (void*)&branch_word, (void*)(&postpone_word.payload[0] + 12), (void*)&_2c__word, (void*)&exit_word )

//@C IF IMMEDIATE
// \ -- addr
//  [&lit_word] [&branch0_word] ,
//  HERE
//  0 ,
IMM( if_word, codeword, "IF", &postpone_word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&here_word, (void*)&zero_word, (void*)&_2c__word, (void*)&exit_word )

//@C THEN IMMEDIATE
// \ addr --
//  HERE SWAP !
IMM( then_word, codeword, "THEN", &if_word, (void*)&here_word, (void*)&swap_word, (void*)&pling_word, (void*)&exit_word )

//@C ELSE IMMEDIATE
// \ if-addr -- else-addr
//  \ Emit a branch over the false part.
//  [&lit_word] [&branch_word] ,      \ -- if-addr
//
//  \ Remember where the branch label is for patching later.
//  HERE 0 ,                         \ -- if-addr else-addr
//
//  \ Patch the *old* branch label (from the condition) to the current address.
//  SWAP                               \ -- else-addr if-addr
//  [&then_word]
IMM( else_word, codeword, "ELSE", &then_word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&here_word, (void*)&zero_word, (void*)&_2c__word, (void*)&swap_word, (void*)(&then_word), (void*)&exit_word )

//@C BEGIN IMMEDIATE
// \ -- start-addr
//  HERE
IMM( begin_word, codeword, "BEGIN", &else_word, (void*)&here_word, (void*)&exit_word )

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
// \ start-addr -- while-target-addr start-addr
//   [&lit_word] [&branch0_word] ,
//   HERE
//   0 ,
//   SWAP
IMM( while_word, codeword, "WHILE", &until_word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&here_word, (void*)&zero_word, (void*)&_2c__word, (void*)&swap_word, (void*)&exit_word )

//@C REPEAT IMMEDIATE
// \ while-target-addr start-addr --
//   [&lit_word] [&branch_word] , ,
//
//   HERE SWAP !
IMM( repeat_word, codeword, "REPEAT", &while_word, (void*)(&lit_word), (void*)(&branch_word), (void*)&_2c__word, (void*)&_2c__word, (void*)&here_word, (void*)&swap_word, (void*)&pling_word, (void*)&exit_word )

//@C DO IMMEDIATE
// \ C: -- &leave-addr start-addr
// \ R: -- leave-addr index max
// \    max index --
//   \ Save the loop exit address; this will be patched by LOOP.
//   [&lit_word] [&lit_word] ,
//   HERE 0 ,
//   [&lit_word] [&arrow_r_word] ,
//
//   \ Save loop start address onto the compiler's stack.
//   HERE
//
//   \ Push the index and max values onto the return stack.
//   [&lit_word] [&arrow_r_word] ,
//   [&lit_word] [&arrow_r_word] ,
IMM( do_word, codeword, "DO", &repeat_word, (void*)(&lit_word), (void*)(&lit_word), (void*)&_2c__word, (void*)&here_word, (void*)&zero_word, (void*)&_2c__word, (void*)(&lit_word), (void*)(&arrow_r_word), (void*)&_2c__word, (void*)&here_word, (void*)(&lit_word), (void*)(&arrow_r_word), (void*)&_2c__word, (void*)(&lit_word), (void*)(&arrow_r_word), (void*)&_2c__word, (void*)&exit_word )

//@C loophelper HIDDEN
// \ Contains the actual logic for loop.
// \ R: leave-addr index max r-addr -- leave-addr r-addr
// \    incr -- max index flag
//   \ Fetch data from return stack.
//   R> SWAP R> R> ROT                 \ r-addr max index incr  R: leave-addr
//   DUP ROT                           \ r-addr max incr incr index
//   + SWAP                            \ r-addr max index' incr 
//   0< IF
//     \ Counting down!
//     2DUP >                          \ r-addr max index' flag
//   ELSE
//     \ Counting up.
//     2DUP <=                         \ r-addr max index' flag
//   THEN
//     
//   \ Put the return address back!
//   >R                                \ r-addr max index'
//   ROT                               \ max index' r-addr
//   R>                                \ max index' r-addr flag
//   SWAP                              \ max index' flag r-addr
//   >R                                \ max index' flag
COM( loophelper_word, codeword, "", &do_word, (void*)&r_arrow_word, (void*)&swap_word, (void*)&r_arrow_word, (void*)&r_arrow_word, (void*)&rot_word, (void*)&dup_word, (void*)&rot_word, (void*)&add_word, (void*)&swap_word, (void*)&less0_word, (void*)&branch0_word, (void*)(&loophelper_word.payload[0] + 16), (void*)&t_dup_word, (void*)&gt_word, (void*)&branch_word, (void*)(&loophelper_word.payload[0] + 18), (void*)&t_dup_word, (void*)&le_word, (void*)&arrow_r_word, (void*)&rot_word, (void*)&r_arrow_word, (void*)&swap_word, (void*)&arrow_r_word, (void*)&exit_word )

//@C +LOOP IMMEDIATE
// \    incr --
// \ R: leave-addr index max --
// \ C: &leave-addr start-addr --
//   [&lit_word] [&loophelper_word] ,
//   [&lit_word] [&branch0_word] , ,
//   [&lit_word] [&t_drop_word] ,
//   [&lit_word] [&rdrop_word] ,
//
//   \ Patch the leave address to contain the loop exit address.
//   HERE SWAP !
IMM( _2b_loop_word, codeword, "+LOOP", &loophelper_word, (void*)(&lit_word), (void*)(&loophelper_word), (void*)&_2c__word, (void*)(&lit_word), (void*)(&branch0_word), (void*)&_2c__word, (void*)&_2c__word, (void*)(&lit_word), (void*)(&t_drop_word), (void*)&_2c__word, (void*)(&lit_word), (void*)(&rdrop_word), (void*)&_2c__word, (void*)&here_word, (void*)&swap_word, (void*)&pling_word, (void*)&exit_word )

//@C LOOP IMMEDIATE
//   1 LITERAL +LOOP
IMM( loop_word, codeword, "LOOP", &_2b_loop_word, (void*)&one_word, (void*)&literal_word, (void*)&_2b_loop_word, (void*)&exit_word )

//@C LEAVE
// \ R: leave-addr index max
//   \ Remove LEAVE's return address.
//   RDROP
//
//   \ ...and the two control words.
//   RDROP RDROP
//
//   \ All that's left is the loop exit address, and EXIT
//   \ will consume that.
COM( leave_word, codeword, "LEAVE", &loop_word, (void*)&rdrop_word, (void*)&rdrop_word, (void*)&rdrop_word, (void*)&exit_word )

//@C UNLOOP
// \ R: leave-addr index max
//   R> RDROP RDROP RDROP >R
COM( unloop_word, codeword, "UNLOOP", &leave_word, (void*)&r_arrow_word, (void*)&rdrop_word, (void*)&rdrop_word, (void*)&rdrop_word, (void*)&arrow_r_word, (void*)&exit_word )

//@C I
// \ R: leave-addr index max -- leave-addr index max
// \    -- index
//  2 RPICK
COM( i_word, codeword, "I", &unloop_word, (void*)&two_word, (void*)&rpick_word, (void*)&exit_word )

//@C J
// \ R: leave-addr index max -- leave-addr index max
// \    -- index
//  5 RPICK
COM( j_word, codeword, "J", &i_word, (void*)&lit_word, (void*)5, (void*)&rpick_word, (void*)&exit_word )

//@C HEX
//  16 BASE !
COM( hex_word, codeword, "HEX", &j_word, (void*)&lit_word, (void*)16, (void*)&base_word, (void*)&pling_word, (void*)&exit_word )

//@C DECIMAL
//  10 BASE !
COM( decimal_word, codeword, "DECIMAL", &hex_word, (void*)&lit_word, (void*)10, (void*)&base_word, (void*)&pling_word, (void*)&exit_word )

//@C S>D
//   DUP 0< IF -1 ELSE 0 THEN 
COM( s_3e_d_word, codeword, "S>D", &decimal_word, (void*)&dup_word, (void*)&less0_word, (void*)&branch0_word, (void*)(&s_3e_d_word.payload[0] + 7), (void*)&m_one_word, (void*)&branch_word, (void*)(&s_3e_d_word.payload[0] + 8), (void*)&zero_word, (void*)&exit_word )

//@C R@
//   1 RPICK
COM( r_40__word, codeword, "R@", &s_3e_d_word, (void*)&one_word, (void*)&rpick_word, (void*)&exit_word )

//@C +- HIDDEN
//   0< IF NEGATE THEN
COM( _2b__2d__word, codeword, "", &r_40__word, (void*)&less0_word, (void*)&branch0_word, (void*)(&_2b__2d__word.payload[0] + 4), (void*)&negate_word, (void*)&exit_word )

//@C UM/MOD
// \ x.d y -- rem quot
//   0 UD/MOD                \ rem.d quot.d
//   ROT 2DROP
COM( um_2f_mod_word, codeword, "UM/MOD", &_2b__2d__word, (void*)&zero_word, (void*)&ud_mod_word, (void*)&rot_word, (void*)&t_drop_word, (void*)&exit_word )

//@C SM/REM
//   OVER                              \ low high quot high
//   >R >R
//   DABS R@ ABS
//   UM/MOD
//   R> R@ XOR
//   +-
//   SWAP R>
//   +-
//   SWAP
COM( sm_2f_rem_word, codeword, "SM/REM", &um_2f_mod_word, (void*)&over_word, (void*)&arrow_r_word, (void*)&arrow_r_word, (void*)&dabs_word, (void*)&r_40__word, (void*)&abs_word, (void*)&um_2f_mod_word, (void*)&r_arrow_word, (void*)&r_40__word, (void*)&xor_word, (void*)&_2b__2d__word, (void*)&swap_word, (void*)&r_arrow_word, (void*)&_2b__2d__word, (void*)&swap_word, (void*)&exit_word )

//@C /
//  >R S>D R> FM/MOD SWAP DROP
COM( _2f__word, codeword, "/", &sm_2f_rem_word, (void*)&arrow_r_word, (void*)&s_3e_d_word, (void*)&r_arrow_word, (void*)&fm_mod_word, (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C /MOD
//  >R S>D R> FM/MOD
COM( _2f_mod_word, codeword, "/MOD", &_2f__word, (void*)&arrow_r_word, (void*)&s_3e_d_word, (void*)&r_arrow_word, (void*)&fm_mod_word, (void*)&exit_word )

//@C MOD
//  >R S>D R> FM/MOD DROP
COM( mod_word, codeword, "MOD", &_2f_mod_word, (void*)&arrow_r_word, (void*)&s_3e_d_word, (void*)&r_arrow_word, (void*)&fm_mod_word, (void*)&drop_word, (void*)&exit_word )

//@C */
//  >R M* R> FM/MOD SWAP DROP
COM( _2a__2f__word, codeword, "*/", &mod_word, (void*)&arrow_r_word, (void*)&m_star_word, (void*)&r_arrow_word, (void*)&fm_mod_word, (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C */MOD
//  >R M* R> FM/MOD
COM( _2a__2f_mod_word, codeword, "*/MOD", &_2a__2f__word, (void*)&arrow_r_word, (void*)&m_star_word, (void*)&r_arrow_word, (void*)&fm_mod_word, (void*)&exit_word )

//@C DEPTH
//  SP0 SP@ - CELL / 1-
COM( depth_word, codeword, "DEPTH", &_2a__2f_mod_word, (void*)&sp0_word, (void*)&sp_at_word, (void*)&sub_word, (void*)&cell_word, (void*)&_2f__word, (void*)&sub_one_word, (void*)&exit_word )

//@C s"helper HIDDEN
// \ -- addr count
//  \ The return address points at a counted string.
//  R> DUP                             \ -- r-addr r-addr
//
//  \ Move it to point after the string.
//  DUP C@ + 1+                        \ -- r-addr end-r-addr
//
//  \ Align it!
//  ALIGNED
//
//  \ Store it back as the return address.
//  >R
//
//  \ ...and decode the counted string.
//  COUNT
COM( s_22_helper_word, codeword, "", &depth_word, (void*)&r_arrow_word, (void*)&dup_word, (void*)&dup_word, (void*)&c_at_word, (void*)&add_word, (void*)&add_one_word, (void*)&aligned_word, (void*)&arrow_r_word, (void*)&count_word, (void*)&exit_word )

//@C S" IMMEDIATE
// \ -- addr count
//  \ Emit the helper.
//  [&lit_word] [&s_22_helper_word] ,
//
//  \ Emit the text itself as a counted string.
//  34 WORD
//
//  \ Advance over the text.
//  C@ 1+ ALLOT
//
//  \ Make sure the workspace pointer is aligned!
//  ALIGN
IMM( s_22__word, codeword, "S\"", &s_22_helper_word, (void*)(&lit_word), (void*)(&s_22_helper_word), (void*)&_2c__word, (void*)&lit_word, (void*)34, (void*)&word_word, (void*)&c_at_word, (void*)&add_one_word, (void*)&allot_word, (void*)&align_word, (void*)&exit_word )

//@C 2*
//  1 LSHIFT
COM( _32__2a__word, codeword, "2*", &s_22__word, (void*)&one_word, (void*)&lshift_word, (void*)&exit_word )

//@C MIN
// \ x1 x2 -- x3
//   2DUP > IF SWAP THEN DROP
COM( min_word, codeword, "MIN", &_32__2a__word, (void*)&t_dup_word, (void*)&gt_word, (void*)&branch0_word, (void*)(&min_word.payload[0] + 5), (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C MAX
// \ x1 x2 -- x3
//   2DUP < IF SWAP THEN DROP
COM( max_word, codeword, "MAX", &min_word, (void*)&t_dup_word, (void*)&lt_word, (void*)&branch0_word, (void*)(&max_word.payload[0] + 5), (void*)&swap_word, (void*)&drop_word, (void*)&exit_word )

//@C RECURSE IMMEDIATE
//   LATEST @ ,
IMM( recurse_word, codeword, "RECURSE", &max_word, (void*)&latest_word, (void*)&at_word, (void*)&_2c__word, (void*)&exit_word )

//@C u.nospace HIDDEN
// \ u --
//   0 BASE @ UM/MOD         \ rem quot
//   ?DUP IF
//     RECURSE
//   THEN
//
//   DUP 10 < IF
//     48
//   ELSE
//     10 -
//     65
//   THEN
//   + EMIT
COM( u_2e_nospace_word, codeword, "", &recurse_word, (void*)&zero_word, (void*)&base_word, (void*)&at_word, (void*)&um_2f_mod_word, (void*)&q_dup_word, (void*)&branch0_word, (void*)(&u_2e_nospace_word.payload[0] + 8), (void*)&u_2e_nospace_word, (void*)&dup_word, (void*)&lit_word, (void*)10, (void*)&lt_word, (void*)&branch0_word, (void*)(&u_2e_nospace_word.payload[0] + 18), (void*)&lit_word, (void*)48, (void*)&branch_word, (void*)(&u_2e_nospace_word.payload[0] + 23), (void*)&lit_word, (void*)10, (void*)&sub_word, (void*)&lit_word, (void*)65, (void*)&add_word, (void*)&emit_word, (void*)&exit_word )

//@C U/ 
// \ Unsigned ordinary division.
// \ x y -- quot
//   0 SWAP
//   UM/MOD
//   NIP
COM( u_2f__word, codeword, "U/", &u_2e_nospace_word, (void*)&zero_word, (void*)&swap_word, (void*)&um_2f_mod_word, (void*)&nip_word, (void*)&exit_word )

//@C uwidth HIDDEN
// \ This word returns the width (in characters) of an unsigned number in the current base.
// \ u -- width
//   BASE @ U/
//   ?DUP IF
//     RECURSE 1+
//   ELSE
//     1
//   THEN
COM( uwidth_word, codeword, "", &u_2f__word, (void*)&base_word, (void*)&at_word, (void*)&u_2f__word, (void*)&q_dup_word, (void*)&branch0_word, (void*)(&uwidth_word.payload[0] + 10), (void*)&uwidth_word, (void*)&add_one_word, (void*)&branch_word, (void*)(&uwidth_word.payload[0] + 11), (void*)&one_word, (void*)&exit_word )

//@C U.R
// \ Prints an unsigned number in a field.
// \ u width --
//   SWAP DUP                          \ width u u
//   uwidth                            \ width u uwidth
//   ROT                               \ u uwidth width
//   SWAP -                            \ u width-uwidth
//
//   SPACES u.nospace
COM( u_2e_r_word, codeword, "U.R", &uwidth_word, (void*)&swap_word, (void*)&dup_word, (void*)&uwidth_word, (void*)&rot_word, (void*)&swap_word, (void*)&sub_word, (void*)&spaces_word, (void*)&u_2e_nospace_word, (void*)&exit_word )

//@C .R
// \ Prints a signed number in a field. We can't just print the sign and call
// \ U.R, because we want the sign to be next to the number...
// \ n width --
//
//   SWAP                              \ width n
//   DUP 0< IF
//     NEGATE                          \ width u
//     1
//     SWAP ROT 1-                     \ 1 u width-1
//   ELSE
//     0
//     SWAP ROT                        \ 0 u width
//   THEN
//
//   SWAP DUP                          \ flag width u u
//   uwidth                            \ flag width u uwidth
//   ROT                               \ flag u uwidth width
//   SWAP -                            \ flag u width-uwidth
//
//   SPACES                            \ flag u
//   SWAP                              \ u flag
//
//   IF 45 EMIT THEN
//
//   u.nospace
COM( _2e_r_word, codeword, ".R", &u_2e_r_word, (void*)&swap_word, (void*)&dup_word, (void*)&less0_word, (void*)&branch0_word, (void*)(&_2e_r_word.payload[0] + 12), (void*)&negate_word, (void*)&one_word, (void*)&swap_word, (void*)&rot_word, (void*)&sub_one_word, (void*)&branch_word, (void*)(&_2e_r_word.payload[0] + 15), (void*)&zero_word, (void*)&swap_word, (void*)&rot_word, (void*)&swap_word, (void*)&dup_word, (void*)&uwidth_word, (void*)&rot_word, (void*)&swap_word, (void*)&sub_word, (void*)&spaces_word, (void*)&swap_word, (void*)&branch0_word, (void*)(&_2e_r_word.payload[0] + 28), (void*)&lit_word, (void*)45, (void*)&emit_word, (void*)&u_2e_nospace_word, (void*)&exit_word )

//@C .
//   0 .R SPACE
COM( _2e__word, codeword, ".", &_2e_r_word, (void*)&zero_word, (void*)&_2e_r_word, (void*)&space_word, (void*)&exit_word )

//@C U.
//   0 U.R SPACE
COM( u_2e__word, codeword, "U.", &_2e__word, (void*)&zero_word, (void*)&u_2e_r_word, (void*)&space_word, (void*)&exit_word )

//@C showstack HIDDEN
// \ Dumps the contents of a stack.
// \ ( SP@ SP0 -- )
//   BEGIN
//     2DUP <>
//   WHILE
//     CELL-
//     DUP @ .
//   REPEAT
//   2DROP
//   CR
COM( showstack_word, codeword, "", &u_2e__word, (void*)&t_dup_word, (void*)&not_equals_word, (void*)&branch0_word, (void*)(&showstack_word.payload[0] + 10), (void*)&cell_2d__word, (void*)&dup_word, (void*)&at_word, (void*)&_2e__word, (void*)&branch_word, (void*)(&showstack_word.payload[0] + 0), (void*)&t_drop_word, (void*)&cr_word, (void*)&exit_word )

//@C .S
//   SP@ SP0 showstack
COM( _2e_s_word, codeword, ".S", &showstack_word, (void*)&sp_at_word, (void*)&sp0_word, (void*)&showstack_word, (void*)&exit_word )

//@C .RS
//   RSP@ CELL+ RSP0 showstack
COM( _2e_rs_word, codeword, ".RS", &_2e_s_word, (void*)&rsp_at_word, (void*)&cell_2b__word, (void*)&rsp0_word, (void*)&showstack_word, (void*)&exit_word )

//@C CHAR
//   BL WORD
//   DUP C@ 0= IF E_eol THEN
//   1+ C@
COM( char_word, codeword, "CHAR", &_2e_rs_word, (void*)&bl_word, (void*)&word_word, (void*)&dup_word, (void*)&c_at_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&char_word.payload[0] + 8), (void*)&e_eol_word, (void*)&add_one_word, (void*)&c_at_word, (void*)&exit_word )

//@C [CHAR] IMMEDIATE
//   CHAR LITERAL
IMM( _5b_char_5d__word, codeword, "[CHAR]", &char_word, (void*)&char_word, (void*)&literal_word, (void*)&exit_word )

//@C ." IMMEDIATE
//   \ Load string literal.
//   S\"
//
//   \ Print it.
//   [&lit_word] [&type_word] ,
IMM( _2e__22__word, codeword, ".\"", &_5b_char_5d__word, (void*)&s_22__word, (void*)(&lit_word), (void*)(&type_word), (void*)&_2c__word, (void*)&exit_word )

//@C 2@
//   DUP CELL+ @ SWAP @
COM( _32__40__word, codeword, "2@", &_2e__22__word, (void*)&dup_word, (void*)&cell_2b__word, (void*)&at_word, (void*)&swap_word, (void*)&at_word, (void*)&exit_word )

//@C 2!
//   SWAP OVER ! CELL+ !
COM( _32__21__word, codeword, "2!", &_32__40__word, (void*)&swap_word, (void*)&over_word, (void*)&pling_word, (void*)&cell_2b__word, (void*)&pling_word, (void*)&exit_word )

//@C padend HIDDEN
//   PAD #PAD +
COM( padend_word, codeword, "", &_32__21__word, (void*)&pad_word, (void*)&h_pad, (void*)&add_word, (void*)&exit_word )

//@C <#
// \ ( -- )
//   padend [&lit_word] [&holdptr] !
COM( _3c__23__word, codeword, "<#", &padend_word, (void*)&padend_word, (void*)(&lit_word), (void*)(&holdptr), (void*)&pling_word, (void*)&exit_word )

//@C #>
// \ ( u.d -- addr len )
//   2DROP                        \ --
//   [&lit_word] [&holdptr] @     \ addr
//   padend OVER                  \ addr end addr
//   -                            \ addr len
COM( _23__3e__word, codeword, "#>", &_3c__23__word, (void*)&t_drop_word, (void*)(&lit_word), (void*)(&holdptr), (void*)&at_word, (void*)&padend_word, (void*)&over_word, (void*)&sub_word, (void*)&exit_word )

//@C HOLD
// \ ( c -- )
//   -1 [&lit_word] [&holdptr] +!@     \ c ptr-1
//   C!
COM( hold_word, codeword, "HOLD", &_23__3e__word, (void*)&m_one_word, (void*)(&lit_word), (void*)(&holdptr), (void*)&_2b__21__40__word, (void*)&c_pling_word, (void*)&exit_word )

//@C #
// \ ( u.d -- u.d )
//   BASE @ 0                \ u.d base.d
//   UD/MOD                  \ rem.d quot.d
//   2SWAP DROP              \ quot.d rem
//   9 OVER                  \ quot.d rem 9 rem
//   < IF                    \ quot.d rem
//     [&lit_word] [(65-57-1)] +
//   THEN
//   48 + HOLD
COM( _23__word, codeword, "#", &hold_word, (void*)&base_word, (void*)&at_word, (void*)&zero_word, (void*)&ud_mod_word, (void*)&t_swap_word, (void*)&drop_word, (void*)&lit_word, (void*)9, (void*)&over_word, (void*)&lt_word, (void*)&branch0_word, (void*)(&_23__word.payload[0] + 15), (void*)(&lit_word), (void*)((65-57-1)), (void*)&add_word, (void*)&lit_word, (void*)48, (void*)&add_word, (void*)&hold_word, (void*)&exit_word )

//@C SIGN
// \ ( n -- )
//   0< IF
//     45 HOLD
//   THEN
COM( sign_word, codeword, "SIGN", &_23__word, (void*)&less0_word, (void*)&branch0_word, (void*)(&sign_word.payload[0] + 6), (void*)&lit_word, (void*)45, (void*)&hold_word, (void*)&exit_word )

//@C #S
// \ ( u.d -- 0.d )
//   BEGIN
//     #
//     2DUP OR 0=
//   UNTIL
COM( _23_s_word, codeword, "#S", &sign_word, (void*)&_23__word, (void*)&t_dup_word, (void*)&or_word, (void*)&equals0_word, (void*)&branch0_word, (void*)(&_23_s_word.payload[0] + 0), (void*)&exit_word )

//@C SHIFT-ARGS
// \ Discards the first argument in the argv array.
// \ ( -- )
//   ARGC @ IF
//     -1 ARGC +!
//     CELL ARGV +!
//   THEN
COM( shift_2d_args_word, codeword, "SHIFT-ARGS", &_23_s_word, (void*)&argc_word, (void*)&at_word, (void*)&branch0_word, (void*)(&shift_2d_args_word.payload[0] + 10), (void*)&m_one_word, (void*)&argc_word, (void*)&_2b__21__word, (void*)&cell_word, (void*)&argv_word, (void*)&_2b__21__word, (void*)&exit_word )

//@C ARG
// \ Fetches an argument from the argv array.
// \ ( n -- addr count )
//   ARGV @ + @              \ c-ptr
//   CCOUNT                  \ addr count
COM( arg_word, codeword, "ARG", &shift_2d_args_word, (void*)&argv_word, (void*)&at_word, (void*)&add_word, (void*)&at_word, (void*)&ccount_word, (void*)&exit_word )

//@C NEXT-ARG
// \ Fetches the next argument (or 0 0 if none).
// \ ( -- addr count )
//   ARGC @ IF
//     0 ARG
//     SHIFT-ARGS
//   ELSE
//     0 0
//   THEN
COM( next_2d_arg_word, codeword, "NEXT-ARG", &arg_word, (void*)&argc_word, (void*)&at_word, (void*)&branch0_word, (void*)(&next_2d_arg_word.payload[0] + 9), (void*)&zero_word, (void*)&arg_word, (void*)&shift_2d_args_word, (void*)&branch_word, (void*)(&next_2d_arg_word.payload[0] + 11), (void*)&zero_word, (void*)&zero_word, (void*)&exit_word )

static cdefn_t* last = (defn_t*) &next_2d_arg_word; //@E
static defn_t* latest = (defn_t*) &next_2d_arg_word; //@E

int main(int argc, const char* argv[])
{
	here = here_top = sbrk(0);
	claim_workspace(0);

	setjmp(onerror);
	input_fd = 0;

	if (argc > 1)
	{
		input_fd = open(argv[1], O_RDONLY);

		/* Panics when running a script exit, rather than returning to the
		 * REPL. */
		if (setjmp(onerror))
			exit(1);

		if (input_fd == -1)
		{
			strerr("panic: unable to open file: ");
			strerr(argv[1]);
			strerr("\n");
			exit(1);
		}

		argv++;
		argc++;
	}
			
	global_argv = argv;
	global_argc = argc;

	dsp = dstack + DSTACKSIZE;
	rsp = rstack + RSTACKSIZE;

	pc = (defn_t**) &quit_word.payload[0];
	for (;;)
	{
		const struct definition* w = (void*) *pc++;
		/* Uncomment this to trace the current program counter, stack and
		 * word for every bytecode. */
		#if 0
			cell_t* p;
			printf("%p ", pc-1);
			printf("S(");
			for (p = dstack+DSTACKSIZE-1; p >= dsp; p--)
				printf("%lx ", *p);
			printf(") ");
			/* Uncomment this to also trace the return stack. */
			#if 0
				printf("R(");
				for (p = rstack+RSTACKSIZE-1; p >= rsp; p--)
					printf("%lx ", *p);
				printf(") ");
			#endif
			printf("[");
			if (w->name)
				fwrite(&w->name->data[0], 1, w->name->len & FL__MASK, stdout);
			else
				printf("(null)");
			putchar(']');
			putchar('\n');
		#endif
		w->code(w);
	}
	return 0;
}
