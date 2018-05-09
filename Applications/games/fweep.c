/*
  Fweeplet -- a Z-machine interpreter for versions 1 to 5.
  This program is license under GNU GPL v3 or later version.
  
  Cut down from 'fweep'
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define IVERSION "fe0.01"

#define VERSION 	3

#if (VERSION == 8)
#define PACKED_SHIFT	3
typedef uint16_t obj_t;

#elif (VERSION > 3)
#define PACKED_SHIFT	2
typedef uin16_t obj_t;

#else				/*  */
#define PACKED_SHIFT	1
typedef uint8_t obj_t;

#endif				/*  */
typedef char boolean;
typedef uint8_t byte;
typedef struct {
	uint32_t pc;
	uint16_t start;
	uint8_t argc;
	boolean stored;
} StackFrame;
const char zscii_conv_1[128] = {
	[155 - 128] =
	'a', 'o', 'u', 'A', 'O', 'U', 's', '>', '<', 'e', 'i', 'y',
	'E', 'I', 'a', 'e', 'i', 'o', 'u', 'y', 'A', 'E', 'I', 'O',
	'U', 'Y', 'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U',
	'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U', 'a', 'A',
	'o', 'O', 'a', 'n', 'o', 'A', 'N', 'O', 'a', 'A', 'c', 'C',
	't', 't', 'T', 'T', 'L', 'o', 'O', '!', '?'
};

/* FIXME: probably smaller as function */
const char zscii_conv_2[128] = {
	[155 - 128] = 'e', 'e', 'e',
	[161 - 128] = 's', '>', '<',
	[211 - 128] = 'e', 'E',
	[215 - 128] = 'h', 'h', 'h', 'h',
	[220 - 128] = 'e', 'E'
};

#if (VERSION == 1)
const char alpha[78] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789.,!?_#'\"/\\<-:()";
#else
const char alpha[78] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ^0123456789.,!?_#'\"/\\-:()";
#endif

char *story_name;
int story = -1;
byte auxname[11];
boolean original = 1;
boolean verified = 0;
boolean tandy = 0;
boolean qtospace = 0;
int sc_rows = 25;
int sc_columns = 80;
uint32_t routine_start;		/* > 16bit */
uint32_t text_start;		/* > 16bit */
uint16_t object_table;
uint16_t dictionary_table;
uint16_t restart_address;
uint16_t synonym_table;
uint16_t alphabet_table;
uint16_t static_start;
uint16_t global_table;
uint8_t memory[0x20000];
uint32_t program_counter;	/* Can be in high memory */

#define STACKSIZE 256
#define FRAMESIZE 32
StackFrame frames[FRAMESIZE];
int framemax;
uint16_t stack[STACKSIZE];
int frameptr;
int stackptr;
int stackmax;
uint16_t stream3addr[16];
uint16_t *stream3ptr = stream3addr - 1;
boolean texting = 1;
boolean window = 0;
boolean buffering = 1;
int cur_row = 2;
int cur_column;
int lmargin = 0;
int rmargin = 0;
uint16_t inst_args[8];

#define inst_sargs ((int16_t*)inst_args)
char text_buffer[128];
int textptr;
uint8_t cur_prop_size;
int zch_shift;
int zch_shiftlock;
int zch_code;

/*
 *	Low level I/O
 */

void error(const char *s)
{
	write(2, s, strlen(s));
}

void panic(const char *s)
{
	error(s);
	exit(1);
}

void writes(const char *s)
{
	write(1, s, strlen(s));
}

void waitcr(void)
{
	uint8_t c;
	while(read(0, &c, 1) == 1) {
		if (c == '\n')
			return;
	}
	exit(0);
}

void input(char *b, uint16_t l)
{
	char *s = b;
	char *e = b + l;
	while(s < e && read(0, s, 1) == 1) {
		if (*s == 13 || *s == 10) {
			*s = 0;
			return;
		}
		if (*s < 32)
			continue;
		s++;
	}
	waitcr();
	*s = 0;
}

static uint8_t iobad;

void xwriteb(int f, uint8_t v)
{
	/* for now */
	if (write(f, &v, 1) != 1)
		iobad = 1;
}

void xwritew(int f, uint16_t v)
{
	/* for now */
	if (write(f, &v, 2) != 2)
		iobad = 1;
}

uint8_t xreadb(int f)
{
	uint8_t v;
	if (read(f, &v, 1) != 1)
		iobad = 1;
	return v;
}

uint16_t xreadw(int f)
{
	uint16_t v;
	if (read(f, &v, 2) != 2)
		iobad = 1;
	return v;
}

void xwrite(int f, void *ptr, uint16_t n, uint16_t size)
{
	n *= size;
	xwritew(f, n);
	if (write(f, ptr, n) != n)
		iobad = 1;
}

int xread(int f, void *ptr, uint16_t n, uint16_t size)
{
	uint16_t r;
	n *= size;
	r = xreadw(f);
	if (r > n) {
		error("Save mismatch ?\n");
		iobad = 1;
		return 0;
	}
	if (read(f, ptr, r) != r)
		iobad = 1;
	return r/size;
}

int xopen(const char *path, int flags, int perm)
{
	iobad = 0;
	return open(path, flags, perm);
}

int xclose(int f)
{
	if (close(f))
		iobad = 1;
	if (iobad)
		return -1;
	return 0;
}

		
/*
 *	Memory management
 */
uint8_t pc(void)
{
	return memory[program_counter++];
}

uint16_t randv = 7;
boolean predictable;
int16_t get_random(int16_t max)
{
	int16_t tmp;
	randv += 0xAA55;
	tmp = randv & 0x7FFF;
	randv = (randv << 8) | (randv >> 8);
	return (tmp & max) + 1;
}

void randomize(uint16_t seed)
{
	if (seed) {
		randv = seed;
		predictable = 1;
	} else {
		predictable = 0;
		randv = time(NULL) ^ get_random(32767);
	}
}

uint16_t read16low(uint16_t address)
{
	return (memory[address] << 8) | memory[address + 1];
}

uint16_t read16(uint32_t address)
{
	return (memory[address] << 8) | memory[address + 1];
}

/* Can be uint16 except when debugging */
void write16(uint16_t address, uint16_t value)
{
	memory[address] = value >> 8;
	memory[address + 1] = value & 255;
}

uint8_t read8low(uint16_t address)
{
	return memory[address];
}

uint8_t read8(uint32_t address)
{
	return memory[address];
}

void write8(uint16_t address, uint8_t value)
{
	memory[address] = value;
}

static void newline_margin(void)
{
	writes("\n");
	cur_row++;
	cur_column = 0;
	while (cur_column < lmargin) {
		writes(" ");
		cur_column++;
	}
}

void text_flush(void)
{
	text_buffer[textptr] = 0;
	if (textptr + cur_column >= sc_columns - rmargin)
		newline_margin();
	if (cur_row >= sc_rows && sc_rows != 255) {
		writes("[MORE]");
		waitcr();
		cur_row = 2;
	}
	writes(text_buffer);
	cur_column += textptr;
	textptr = 0;
}

void char_print(uint8_t zscii)
{
	if (!zscii)
		return;
	if (stream3ptr != stream3addr - 1) {
		uint16_t w = read16low(*stream3ptr);
		write8(*stream3ptr + 2 + w, zscii);
		write16(*stream3ptr, w + 1);
		return;
	}
	if ((read8low(0x11) & 1) && !window) {
		write8(0x10, read8low(0x10) | 4);
	}
	if (texting && !window) {
		if (zscii & 0x80) {
			text_buffer[textptr++] =
			    zscii_conv_1[zscii & 0x7F];
			if (zscii_conv_2[zscii & 0x7F])
				text_buffer[textptr++] =
				    zscii_conv_2[zscii & 0x7F];
		} else if (zscii & 0x6F) {
			text_buffer[textptr++] = zscii;
		}
		if (zscii <= 32 || textptr > 125 || !buffering)
			text_flush();
		if (zscii == 13)
			newline_margin();
	}
}

boolean verify_checksum(void)
{
	return 1;
}

uint32_t text_print(uint32_t address);
void zch_print(int z)
{
	int zsl;
	if (zch_shift == 3) {
		zch_code = z << 5;
		zch_shift = 4;
	} else if (zch_shift == 4) {
		zch_code |= z;
		char_print(zch_code);
		zch_shift = zch_shiftlock;
	} else if (zch_shift >= 5) {
		zsl = zch_shiftlock;
		text_print(read16
			/* FIXME: might need to force cast some of this 32bit */
			   (synonym_table + (z << 1) +
			    ((zch_shift - 5) << 6)) << 1);
		zch_shift = zch_shiftlock = zsl;
	} else if (z == 0) {
		char_print(32);
		zch_shift = zch_shiftlock;
#if (VERSION == 1)		
	} else if (z == 1 && VERSION == 1) {
		char_print(13);
		zch_shift = zch_shiftlock;
#endif		
	} else if (z == 1) {
		zch_shift = 5;
#if (VERSION > 2)		
	} else if ((z == 4 || z == 5) && VERSION > 2
		   && (zch_shift == 1 || zch_shift == 2)) {
		zch_shift = zch_shiftlock = zch_shift & (z - 3);
#endif
#if (VERSION < 3)		
	} else if (z == 4 && VERSION < 3) {
		zch_shift = zch_shiftlock = (zch_shift + 1) % 3;
	} else if (z == 5 && VERSION < 3) {
		zch_shift = zch_shiftlock = (zch_shift + 2) % 3;
#endif		
	} else if ((z == 2 && VERSION < 3) || z == 4) {
		zch_shift = (zch_shift + 1) % 3;
	} else if ((z == 3 && VERSION < 3) || z == 5) {
		zch_shift = (zch_shift + 2) % 3;
	} else if (z == 2) {
		zch_shift = 6;
	} else if (z == 3) {
		zch_shift = 7;
	} else if (z == 6 && zch_shift == 2) {
		zch_shift = 3;
#if (VERSION != 1)		
	} else if (z == 7 && zch_shift == 2 && VERSION != 1) {
		char_print(13);
		zch_shift = zch_shiftlock;
#endif		
	} else {
		if (alphabet_table)
			char_print(read8
				   (alphabet_table + z + (zch_shift * 26) -
				    6));

		char_print(alpha[z + (zch_shift * 26) - 6]);
		zch_shift = zch_shiftlock;
	}
}

uint32_t text_print(uint32_t address)
{
	uint16_t t;
	zch_shift = zch_shiftlock = 0;
	for (;;) {
		t = read16(address);
		address += 2;
		zch_print((t >> 10) & 31);
		zch_print((t >> 5) & 31);
		zch_print(t & 31);
		if (t & 0x8000)
			return address;
	}
}


#if (VERSION > 4)
void make_rectangle(uint32_t addr, int width, int height, int skip)
{
	int old_column = cur_column;
	int w, h;
	for (h = 0; h < height; h++) {
		for (w = 0; w < width; w++)
			char_print(read8(addr++));
		addr += skip;
		if (h != height - 1) {
			char_print(13);
			for (w = 0; w < old_column; w++)
				char_print(32);
		}
	}
	text_flush();
}
#endif				/*  */

uint16_t fetch(uint8_t var)
{
	if (var & 0xF0) {
		return read16low(global_table + ((var - 16) << 1));
	} else if (var) {
		return stack[frames[frameptr].start + var - 1];
	} else {
		return stack[--stackptr];
	}
}

void pushstack(uint16_t value)
{
	if (stackptr == STACKSIZE)
		panic("stack overflow.\n");
	stack[stackptr++] = value;
	if (stackptr > stackmax)
		stackmax = stackptr;
}

void store(uint8_t var, uint16_t value)
{
	if (var & 0xF0) {
		write16(global_table + ((var - 16) << 1), value);
	} else if (var) {
		stack[frames[frameptr].start + var - 1] = value;
	} else {
		pushstack(value);
	}
}

void storei(uint16_t value)
{
	store(pc(), value);
}

void enter_routine(uint32_t address, boolean stored, int argc)
{
	int c = read8(address);
	int i;
	if (frameptr == FRAMESIZE - 1)
		panic("out of frames.\n");

	frames[frameptr].pc = program_counter;
	frames[++frameptr].argc = argc;
	frames[frameptr].start = stackptr;
	frames[frameptr].stored = stored;
	program_counter = address + 1;
	if (frameptr > framemax)
		framemax = frameptr;
	if (VERSION < 5) {
		for (i = 0; i < c; i++) {
			pushstack(read16(program_counter));
			program_counter += 2;
		}
	} else {
		for (i = 0; i < c; i++)
			pushstack(0);
	}
	if (argc > c)
		argc = c;
	for (i = 0; i < argc; i++)
		stack[frames[frameptr].start + i] = inst_args[i + 1];
}

void exit_routine(uint16_t result)
{
	stackptr = frames[frameptr].start;
	program_counter = frames[--frameptr].pc;
	if (frames[frameptr + 1].stored)
		store(read8(program_counter - 1), result);
}

void branch(uint32_t cond)
{
	int v = pc();
	if (!(v & 0x80))
		cond = !cond;
	if (v & 0x40)
		v &= 0x3F;
	else
		v = ((v & 0x3F) << 8) | pc();
	if (cond) {
		if (v == 0 || v == 1)
			exit_routine(v);

		else
			program_counter +=
			    (v & 0x1FFF) - ((v & 0x2000) | 2);
	}
}

void obj_tree_put(obj_t obj, int f, uint16_t v)
{

#if (VERSION > 3)
	write16(object_table + 118 + obj * 14 + f * 2, v);
#else				/*  */
	write8(object_table + 57 + obj * 9 + f, v);
#endif				/*  */
}

obj_t obj_tree_get(obj_t obj, int f)
{

#if (VERSION > 3)
	return read16low(object_table + 118 + obj * 14 + f * 2);
#else				/*  */
	return read8low(object_table + 57 + obj * 9 + f);
#endif				/*  */
}


#define parent(x) obj_tree_get(x,0)
#define sibling(x) obj_tree_get(x,1)
#define child(x) obj_tree_get(x,2)
#define set_parent(x,y) obj_tree_put(x,0,y)
#define set_sibling(x,y) obj_tree_put(x,1,y)
#define set_child(x,y) obj_tree_put(x,2,y)
#define attribute(x) (VERSION>3?object_table+112+(x)*14:object_table+53+(x)*9)
#define obj_prop_addr(o) (read16low(VERSION>3?(object_table+124+(o)*14):(object_table+60+(o)*9)))

/* FIXME: rewrite these directly for the two formats and using z pointers
   as it'll be much shorter */
void insert_object(obj_t obj, uint16_t dest)
{
	obj_t p = parent(obj);
//	obj_t s = sibling(obj);
	obj_t x;
	if (p) {
		x = child(p);
		if (x == obj) {
			set_child(p, sibling(x));
		} else {
			while (sibling(x)) {
				if (sibling(x) == obj) {
					set_sibling(x,
						    sibling(sibling(x)));
					break;
				}
				x = sibling(x);
			}
		}
	}
	if (dest) {

		// Attach object to new parent
		set_sibling(obj, child(dest));
		set_child(dest, obj);
	} else {
		set_sibling(obj, 0);
	}
	set_parent(obj, dest);
}

uint16_t property_address(uint16_t obj, uint8_t p)
{
	uint16_t a = obj_prop_addr(obj);
	uint8_t n = 1;
	a += (read8low(a) << 1) + 1;
	while (read8low(a)) {	/* FIXME save and reuse this value! */
		if (VERSION < 4) {
			n = read8low(a) & 31;
			cur_prop_size = (read8(a) >> 5) + 1;
		} else if (read8low(a) & 0x80) {
			n = read8low(a) & 63;
			cur_prop_size = read8low(++a) & 63;
			if (cur_prop_size == 0)
				cur_prop_size = 64;
		} else {
			n = read8low(a) & 63;
			cur_prop_size = (read8low(a) >> 6) + 1;
		}
		a++;

		//if(n<p) return 0;
		if (n == p)
			return a;
		a += cur_prop_size;
	}
	return 0;
}

uint8_t system_input(char **out)
{
	time_t t;
input_again:
	text_flush();
	cur_row = 2;
	cur_column = 0;
	time(&t);
	input(text_buffer, 128);
	if (!*text_buffer)
		goto input_again;
	if (!predictable)
		randv += time(NULL) - t;
	*out = text_buffer;
	return 13;
}

uint64_t dictionary_get(uint32_t addr)
{
	uint64_t v = 0;
	int c = VERSION > 3 ? 6 : 4;
	while (c--)
		v = (v << 8) | read8(addr++);
	return v;
}

uint64_t dictionary_encode(uint8_t * text, int len)
{
	uint64_t v = 0;
	int c = VERSION > 3 ? 9 : 6;
	int i;

	/* FIXME: memory direct reference still here */
	const uint8_t *al =
	    (alphabet_table ? (const uint8_t *) memory +
	     alphabet_table : (const uint8_t *) alpha);
	while (c && len && *text) {

		// Since line breaks cannot be in an input line of text, and VAR:252 is only available in version 5, line breaks need not be considered here.
		// However, because of VAR:252, spaces still need to be considered.
		if (!(c % 3))
			v <<= 1;
		if (*text == ' ') {
			v <<= 5;
		} else {
			for (i = 0; i < 78; i++) {
				if (*text == al[i] && i != 52 && i != 53) {
					v <<= 5;
					if (i >= 26) {
						v |= i / 26 + (VERSION >
							       2 ? 3 : 1);
						c--;
						if (!c)
							return v | 0x8000;
						if (!(c % 3))
							v <<= 1;
						v <<= 5;
					}
					v |= (i % 26) + 6;
					break;
				}
			}
			if (i == 78) {
				v <<= 5;
				v |= VERSION > 2 ? 5 : 3;
				c--;
				if (!c)
					return v | 0x8000;
				if (!(c % 3))
					v <<= 1;
				v <<= 5;
				v |= 6;
				c--;
				if (!c)
					return v | 0x8000;
				if (!(c % 3))
					v <<= 1;
				v <<= 5;
				v |= *text >> 5;
				c--;
				if (!c)
					return v | 0x8000;
				if (!(c % 3))
					v <<= 1;
				v <<= 5;
				v |= *text & 31;
			}
		}
		c--;
		text++;
		len--;
	}
	while (c) {
		if (!(c % 3))
			v <<= 1;
		v <<= 5;
		v |= 5;
		c--;
	}
	return v | 0x8000;
}

void add_to_parsebuf(uint32_t parsebuf, uint32_t dict, uint8_t * d,
		     int k, int el, int ne, int p, uint16_t flag)
{
	uint64_t v = dictionary_encode(d, k);
	uint64_t g;
	int i;
	for (i = 0; i < ne; i++) {
		g = dictionary_get(dict) | 0x8000;
		if (g == v) {

			/* FIXME tidy re-uses */
			write8(parsebuf + (read8(parsebuf + 1) << 2) +
			       5, p + 1 + (VERSION > 4));
			write8(parsebuf + (read8(parsebuf + 1) << 2) + 4,
			       k);
			write16(parsebuf + (read8(parsebuf + 1) << 2) + 2,
				dict);
			break;
		}
		dict += el;
	}
	if (i == ne && !flag) {

		/* FIXME: tidy reuses */
		write8(parsebuf + (read8(parsebuf + 1) << 2) + 5,
		       p + 1 + (VERSION > 4));
		write8(parsebuf + (read8(parsebuf + 1) << 2) + 4, k);
		write16(parsebuf + (read8(parsebuf + 1) << 2) + 2, 0);
	}
	write8(parsebuf + 1, read8(parsebuf + 1) + 1);
}


#define Add_to_parsebuf() if(k)add_to_parsebuf(parsebuf,dict,d,k,el,ne,p1,flag),k=0;p1=p+1;
void tokenise(uint32_t text, uint32_t dict, uint32_t parsebuf, int len,
	      uint16_t flag)
{
	boolean ws[256];
	uint8_t d[10];
	int i, el, ne, k, p, p1;
	memset(ws, 0, 256 * sizeof(boolean));

	/* Direct memory references plus a big copy we should avoid */
	/* FIXME change algorithms */
	if (!dict) {
		for (i = 1; i <= memory[dictionary_table]; i++)
			ws[memory[dictionary_table + i]] = 1;
		dict = dictionary_table;
	}
	for (i = 1; i <= memory[dict]; i++)
		ws[memory[dict + i]] = 1;
	memory[parsebuf + 1] = 0;
	k = p = p1 = 0;
	el = read8low(dict + read8(dict) + 1);
	ne = read16low(dict + read8(dict) + 2);
	if (ne < 0)
		ne *= -1;	// Currently, it won't care about the order; it doesn't use binary search.
	dict += memory[dict] + 4;
	while (p < len && read8(text + p)
	       && read8(parsebuf + 1) < read8(parsebuf)) {
		i = memory[text + p];
		if (i >= 'A' && i <= 'Z')
			i += 'a' - 'A';
		if (i == '?' && qtospace)
			i = ' ';
		if (i == ' ') {
			Add_to_parsebuf();
		} else if (ws[i]) {
			Add_to_parsebuf();
			*d = i;
			k = 1;
			Add_to_parsebuf();
		} else if (k < 10) {
			d[k++] = i;
		} else {
			k++;
		}
		p++;
	}
	Add_to_parsebuf();
}


#undef Add_to_parsebuf
uint8_t line_input(void)
{
	char *ptr;
	char *p;
	int c, cmax;		/* FIXME: uint16_t surely ? */
	uint8_t res;
	res = system_input(&ptr);

	/* ? is there another copy of this FIXME */
	if (read8low(0x11) & 1)
		write8(0x10, read8low(0x10) | 4);
	p = ptr;
	while (*p) {
		if (*p >= 'A' && *p <= 'Z')
			*p |= 0x20;
		p++;
	}
	p = ptr;
	c = 0;
	cmax = read8low(inst_args[0]);
	if (VERSION > 4) {

		// "Left over" characters are not implemented.
		while (*p && c < cmax) {
			write8(inst_args[0] + c + 2, *p++);
			++c;
		}
		write8(inst_args[0] + 1, c);
		if (inst_args[1])
			tokenise(inst_args[0] + 2, 0, inst_args[1], c, 0);
	} else {
		while (*p && c < cmax) {
			write8(inst_args[0] + c + 1, *p++);
			++c;
		}
		write8(c + 1, 0);
		tokenise(inst_args[0] + 1, 0, inst_args[1], c, 0);
	}
	return res;
}

uint8_t char_input(void)
{
	char *ptr;
	uint8_t res;
	res = system_input(&ptr);
	if (res == 13 && *ptr)
		res = *ptr;
	return res;
}

void game_restart(void)
{
	stackptr = frameptr = 0;
	program_counter = restart_address;
	lseek(story, 64, SEEK_SET);
	if (read(story, memory + 64, sizeof(memory)) < 1024)
		panic("invalid story file.\n");
}

void game_save(uint8_t storage)
{
	char filename[64];
	int f;
	uint8_t c;
	uint16_t o, q;
	writes("\n*** Save? ");
	input(filename, 64);
	if (*filename == '.' && !filename[1]) {
		/* FIXME: strlcpy/cat */
		strcpy(filename, story_name);
		strcat(filename, ".sav");
	}
	cur_column = 0;
	if (!*filename) {
		if (VERSION < 4)
			branch(0);
		else
			store(storage, 0);
		return;
	} else if (*filename == '*') {
		if (VERSION < 4)
			branch(1);
		else
			store(storage, strtol(filename + 1, 0, 0));
		return;
	}
	f = xopen(filename, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if (f == -1)
		goto bad;
		
	if (VERSION < 4)
		branch(1);
	else
		store(storage, 2);
	/* Fweep has a nice endian safe blah blah de blah byte by byte
	   Save/Restore. We don't bother. Saved games are platfor specific
	   Deal with it! */
	frames[frameptr].pc = program_counter;
	frames[frameptr + 1].start = stackptr;
	
	xwrite(f, frames, frameptr + 1, sizeof(StackFrame));
	xwrite(f, stack, stackptr, 2);

	xwriteb(f, 0xAA);
	lseek(story, o = 0x38, SEEK_SET);
	q = 0;
	while (o < static_start) {
		read(story, &c, 1);
		if (read8low(o) == c)
			q++;
		else {
			if (q) {
				xwriteb(f, 0);
				xwritew(f, q);
				q = 0;
			}
			xwriteb(f, read8low(o) ^ c);
		}
		o++;
	}
	xwriteb(f, 0);
	xwritew(f, 0);
	if (xclose(f) == -1)
		goto bad;
	if (VERSION < 4)
		return;
	fetch(storage);
	store(storage, 1);
	return;
bad:
	writes("BAD\n");
	/* This seems to fail for V3 games.. investigate */
	if (VERSION < 4)
		branch(0);
	else
		store(storage, 0);

}

void game_restore(void)
{
	char filename[64];
	int f;
	uint8_t d;
	uint16_t o, c;
	writes("\n*** Restore? ");
	input(filename, 64);
	if (*filename == '.' && !filename[1]) {
		/* strlcpy */
		strcpy(filename, story_name);
		strcat(filename, ".sav");
	}
	cur_column = 0;
	if (!*filename)
		return;
	f = xopen(filename, O_RDONLY, 0600);
	if (f == -1)
		return;
	frameptr = xread(f, frames, FRAMESIZE, sizeof(StackFrame));
	if (frameptr == -1)
		goto bad;
	frameptr--;
	stackptr = xread(f, stack, STACKSIZE, 2);
	
	if (xreadb(f) != 0xAA)
		goto bad;

	lseek(story, o = 0x38, SEEK_SET);
	/* FIXME: buffering - but this will look different anyway once
	   we have the virtual management done */
	while (o < static_start) {
		d = xreadb(f);
		/* We xor and save different so a 0 never occurs in a block.
		   It indicates a skip followed by a literal byte xor which
		   may be zero... */
		if (d == 0) {
			c = xreadw(f);
			if (c == 0)
				break;	/* EOF */
			while (c-- > 0)
				write8(o++, xreadb(story));
		}
		else 
			write8(o++, xreadb(story) ^ d);
	}
	if (xclose(f) == -1)
		goto bad;

	while (o < static_start)
		write8(o++, xreadb(story));
	program_counter = frames[frameptr].pc;
	return;
	
bad:
	writes("Read error\n");
	game_restart();
}

void switch_output(int st)
{
	switch (st) {
	case 1:
		texting = 1;
		break;
	case 2:
		write8(0x11, read8low(0x11) | 1);
		break;
	case 3:
		if (stream3ptr != stream3addr + 15) {
			*++stream3ptr = inst_args[1];
			write16(inst_args[1], 0);
		}
		break;
	case 4:
		break;
	case -1:
		texting = 0;
		break;
	case -2:
		write8(0x11, read8low(0x11) & ~1);
		break;
	case -3:
		if (stream3ptr != stream3addr - 1)
			stream3ptr--;
		break;
	case -4:
		break;
	}
}

void execute_instruction(void)
{
	uint8_t in = pc();
	uint16_t at;
	int16_t n;
	uint32_t u;
	int argc;
	if (!predictable)
		randv -= 0x0200;
	if (in & 0x80) {
		if (in >= 0xC0 || in == 0xBE) {

			// variable
			if (in == 0xBE)
				in = pc();
			at = pc() << 8;
			if (in == 0xEC || in == 0xFA)
				at |= pc();
			else
				at |= 0x00FF;
			if ((at & 0xC000) == 0xC000)
				argc = 0;

			else if ((at & 0x3000) == 0x3000)
				argc = 1;

			else if ((at & 0x0C00) == 0x0C00)
				argc = 2;

			else if ((at & 0x0300) == 0x0300)
				argc = 3;

			else if ((at & 0x00C0) == 0x00C0)
				argc = 4;

			else if ((at & 0x0030) == 0x0030)
				argc = 5;

			else if ((at & 0x000C) == 0x000C)
				argc = 6;

			else if ((at & 0x0003) == 0x0003)
				argc = 7;

			else
				argc = 8;
		} else {

			// short
			at = (in << 10) | 0x3FFF;
			argc = (in < 0xB0);
			if (argc)
				in &= 0x8F;
		}
	} else {

		// long
		at = 0x5FFF;
		if (in & 0x20)
			at ^= 0x3000;
		if (in & 0x40)
			at ^= 0xC000;
		in &= 0x1F;
		in |= 0xC0;
		argc = 2;
	}
	for (n = 0; n < 8; n++) {
		switch ((at >> (14 - n * 2)) & 3) {
		case 0:	// large
			inst_args[n] = pc() << 8;
			inst_args[n] |= pc();
			break;
		case 1:	// small
			inst_args[n] = pc();
			break;
		case 2:	// variable
			inst_args[n] = fetch(pc());
			break;
		case 3:	// omit
			inst_args[n] = 0;
			break;
		}
	}
	switch (in) {

#if (VERSION > 4)
	case 0x00:		// Save game or auxiliary file
		if (argc)
			storei(0);

		else
			game_save(pc());
		break;
	case 0x01:		// Restore game or auxiliary file
		storei(0);
		if (!argc)
			game_restore();
		break;
	case 0x02:		// Logical shift
		if (inst_sargs[1] > 0)
			storei(inst_args[0] << inst_args[1]);

		else
			storei(inst_args[0] >> -inst_args[1]);
		break;
	case 0x03:		// Arithmetic shift
		if (inst_sargs[1] > 0)
			storei(inst_sargs[0] << inst_sargs[1]);

		else
			storei(inst_sargs[0] >> -inst_sargs[1]);
		break;
	case 0x04:		// Set font
		text_flush();
		storei((*inst_args == 1 || *inst_args == 4) ? 4 : 0);
		if (!tandy)
			write(1, inst_args == 3 ? 14 : 15, 1);
		break;
	case 0x08:		// Set margins
		if (!window) {
			lmargin = inst_args[0];
			rmargin = inst_args[1];
			if (VERSION == 5)
				write16(40, inst_args[0]);
			if (VERSION == 5)
				write16(41, inst_args[1]);
			while (cur_column < *inst_args) {
				writes(" ");
				cur_column++;
			}
		}
		break;
	case 0x09:		// Save undo buffer
		storei(-1);
		break;
	case 0x0A:		// Restore undo buffer
		storei(-1);
		break;
	case 0x0B:		// Call byte address
		program_counter++;
		enter_routine(*inst_args, 1, argc - 1);
		break;
	case 0x0C:		// Get reference to stack or local variables
		if (*inst_args)
			storei(stackptr - 1);

		else
			storei(frames[frameptr].start + *inst_args - 1);
		break;
	case 0x0D:		// Read through stack/locals reference
		storei(stack[*inst_args]);
		break;
	case 0x0E:		// Write through stack/locals reference
		if (*inst_args < 1024)
			stack[*inst_args] = inst_args[1];
		break;
	case 0x0F:		// Read byte from long property
		u = property_address(inst_args[0], inst_args[1]);
		storei(read8low(u));
		break;
	case 0x1D:		// Read word from long property
		u = property_address(inst_args[0], inst_args[1]);
		storei(read16low(u));
		break;

#endif				/*  */
	case 0x80:		// Jump if zero
		branch(!*inst_args);
		break;
	case 0x81:		// Sibling
		storei(sibling(*inst_args));
		branch(sibling(*inst_args));
		break;
	case 0x82:		// Child
		storei(child(*inst_args));
		branch(child(*inst_args));
		break;
	case 0x83:		// Parent
		storei(parent(*inst_args));
		break;
	case 0x84:		// Property length
		in = read8low(*inst_args - 1);
		storei(VERSION <
		       4 ? (in >> 5) +
		       1 : in & 0x80 ? (in & 63 ? (in & 63) : 64) : (in >>
								     6) +
		       1);
		break;
	case 0x85:		// Increment
		store(*inst_args, fetch(*inst_args) + 1);
		break;
	case 0x86:		// Decrement
		store(*inst_args, fetch(*inst_args) - 1);
		break;
	case 0x87:		// Print by byte address
		text_print(*inst_args);
		break;
	case 0x88:		// Call routine
		if (*inst_args) {
			program_counter++;
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 1, argc - 1);
		} else {
			storei(0);
		}
		break;
	case 0x89:		// Remove object
		insert_object(*inst_args, 0);
		break;
	case 0x8A:		// Print short name of object
		text_print(obj_prop_addr(*inst_args) + 1);
		break;
	case 0x8B:		// Return
		exit_routine(*inst_args);
		break;
	case 0x8C:		// Unconditional jump
		program_counter += *inst_sargs - 2;
		break;
	case 0x8D:		// Print by packed address
		text_print((*inst_args << PACKED_SHIFT) + text_start);
		break;
	case 0x8E:		// Load variable
		at = fetch(*inst_args);
		store(*inst_args, at);	// if it popped from the stack, please put it back on
		storei(at);
		break;
	case 0x8F:		// Not // Call routine and discard result
		if (VERSION > 4) {
			if (*inst_args)
				enter_routine((*inst_args << PACKED_SHIFT)
					      + routine_start, 0,
					      argc - 1);
		} else {
			storei(~*inst_args);
		}
		break;
	case 0xB0:		// Return 1
		exit_routine(1);
		break;
	case 0xB1:		// Return 0
		exit_routine(0);
		break;
	case 0xB2:		// Print literal
		program_counter = text_print(program_counter);
		break;
	case 0xB3:		// Print literal and return
		program_counter = text_print(program_counter);
		char_print(13);
		exit_routine(1);
		break;
	case 0xB4:		// No operation
		//NOP
		break;
	case 0xB5:		// Save
		if (VERSION > 3)
			game_save(pc());

		else
			game_save(0);
		break;
	case 0xB6:		// Restore
		if (VERSION > 3)
			storei(0);
		else
			branch(0);
		game_restore();
		break;
	case 0xB7:		// Restart
		game_restart();
		break;
	case 0xB8:		// Return from stack
		exit_routine(stack[stackptr - 1]);
		break;
	case 0xB9:		// Discard from stack // Catch
		if (VERSION > 4)
			storei(frameptr);

		else
			stackptr--;
		break;
	case 0xBA:		// Quit
		text_flush();
#ifdef DEBUG	
		fprintf(stderr, "stackmax %d framemax %d\n", stackmax,
			framemax);
#endif			
		exit(0);
		break;
	case 0xBB:		// Line break
		char_print(13);
		break;
	case 0xBC:		// Show status
		//NOP
		break;
	case 0xBD:		// Verify checksum
		branch(verify_checksum());
		break;
	case 0xBF:		// Check if game disc is original
		branch(original);
		break;
	case 0xC1:		// Branch if equal
		for (n = 1; n < argc; n++) {
			if (*inst_args == inst_args[n]) {
				branch(1);
				break;
			}
		}
		if (n == argc)
			branch(0);
		break;
	case 0xC2:		// Jump if less
		branch(inst_sargs[0] < inst_sargs[1]);
		break;
	case 0xC3:		// Jump if greater
		branch(inst_sargs[0] > inst_sargs[1]);
		break;
	case 0xC4:		// Decrement and branch if less
		store(*inst_args, n = fetch(*inst_args) - 1);
		branch(n < inst_sargs[1]);
		break;
	case 0xC5:		// Increment and branch if greater
		store(*inst_args, n = fetch(*inst_args) + 1);
		branch(n > inst_sargs[1]);
		break;
	case 0xC6:		// Check if one object is the parent of the other
		branch(parent(inst_args[0]) == inst_args[1]);
		break;
	case 0xC7:		// Test bitmap
		branch((inst_args[0] & inst_args[1]) == inst_args[1]);
		break;
	case 0xC8:		// Bitwise OR
		storei(inst_args[0] | inst_args[1]);
		break;
	case 0xC9:		// Bitwise AND
		storei(inst_args[0] & inst_args[1]);
		break;
	case 0xCA:		// Test attributes
		branch(read8low(attribute(*inst_args) + (inst_args[1] >> 3)) &
		       (0x80 >> (inst_args[1] & 7)));
		break;
	case 0xCB:		// Set attribute
		memory[attribute(*inst_args) + (inst_args[1] >> 3)] |=
		    0x80 >> (inst_args[1] & 7);
		break;
	case 0xCC:		// Clear attribute
		memory[attribute(*inst_args) + (inst_args[1] >> 3)] &=
		    ~(0x80 >> (inst_args[1] & 7));
		break;
	case 0xCD:		// Store to variable
		fetch(inst_args[0]);
		store(inst_args[0], inst_args[1]);
		break;
	case 0xCE:		// Insert object
		insert_object(inst_args[0], inst_args[1]);
		break;
	case 0xCF:		// Read 16-bit number from RAM/ROM
		storei(read16(inst_args[0] + (inst_sargs[1] << 1)));
		break;
	case 0xD0:		// Read 8-bit number from RAM/ROM
		storei(read8(inst_args[0] + inst_sargs[1]));
		break;
	case 0xD1:		// Read property
		if ((u = property_address(inst_args[0], inst_args[1])) != 0)
			storei(cur_prop_size == 1 ? read8low(u) : read16low(u));

		else
			storei(read16
			       (object_table + (inst_args[1] << 1) - 2));
		break;
	case 0xD2:		// Get address of property
		storei(property_address(inst_args[0], inst_args[1]));
		break;
	case 0xD3:		// Find next property
		if (inst_args[1]) {
			u = property_address(inst_args[0], inst_args[1]);
			u += cur_prop_size;
			storei(read8low(u) & (VERSION > 3 ? 63 : 31));
		} else {
			u = obj_prop_addr(inst_args[0]);
			u += (read8low(u) << 1) + 1;
			storei(read8low(u) & (VERSION > 3 ? 63 : 31));
		}
		break;
	case 0xD4:		// Addition
		storei(inst_sargs[0] + inst_sargs[1]);
		break;
	case 0xD5:		// Subtraction
		storei(inst_sargs[0] - inst_sargs[1]);
		break;
	case 0xD6:		// Multiplication
		storei(inst_sargs[0] * inst_sargs[1]);
		break;
	case 0xD7:		// Division
		if (inst_args[1])
			n = inst_sargs[0] / inst_sargs[1];

		else
			panic("\n*** Division by zero\n");
		storei(n);
		break;
	case 0xD8:		// Modulo
		if (inst_args[1])
			n = inst_sargs[0] % inst_sargs[1];

		else
			panic("\n*** Division by zero\n");
		storei(n);
		break;

#if (VERSION > 3)
	case 0xD9:		// Call routine
		if (*inst_args) {
			program_counter++;
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 1, argc - 1);
		} else {
			storei(0);
		}
		break;

#endif				/*  */
#if (VERSION > 4)
	case 0xDA:		// Call routine and discard result
		if (*inst_args)
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 0, argc - 1);
		break;
	case 0xDB:		// Set colors
		//NOP
		break;
	case 0xDC:		// Throw
		frameptr = inst_args[1];
		exit_routine(*inst_args);
		break;
	case 0xDD:		// Bitwise XOR
		storei(inst_args[0] ^ inst_args[1]);
		break;

#endif				/*  */
	case 0xE0:		// Call routine (FIXME for v1)
		if (*inst_args) {
			program_counter++;
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 1, argc - 1);
		} else {
			storei(0);
		}
		break;
	case 0xE1:		// Write 16-bit number to RAM
		write16(inst_args[0] + (inst_sargs[1] << 1), inst_args[2]);
		break;
	case 0xE2:		// Write 8-bit number to RAM
		write8(inst_args[0] + inst_sargs[1], inst_args[2]);
		break;
	case 0xE3:		// Write property
		u = property_address(inst_args[0], inst_args[1]);
		if (cur_prop_size == 1)
			write8(u, inst_args[2]);

		else
			write16(u, inst_args[2]);
		break;
	case 0xE4:		// Read line of input
		n = line_input();
		if (VERSION > 4)
			storei(n);
		break;
	case 0xE5:		// Print character
		char_print(*inst_args);
		break;
	case 0xE6:		// Print number
		{
			static uint8_t nbuf[5];
			n = *inst_sargs;
			if (n == -32768) {
				char_print('-');
				char_print('3');
				char_print('2');
				char_print('7');
				char_print('6');
				char_print('8');
			} else {
				nbuf[0] = nbuf[1] = nbuf[2] = nbuf[3] =
				    nbuf[4] = 0;
				if (n < 0) {
					char_print('-');
					n *= -1;
				}
				nbuf[4] = (n % 10) | '0';
				if (n /= 10)
					nbuf[3] = (n % 10) | '0';
				if (n /= 10)
					nbuf[2] = (n % 10) | '0';
				if (n /= 10)
					nbuf[1] = (n % 10) | '0';
				if (n /= 10)
					nbuf[0] = (n % 10) | '0';
				char_print(nbuf[0]);
				char_print(nbuf[1]);
				char_print(nbuf[2]);
				char_print(nbuf[3]);
				char_print(nbuf[4]);
			}
			break;
		}
	case 0xE7:		// Random number generator
		if (*inst_sargs > 0)
			storei(get_random(*inst_sargs));

		else
			randomize(-*inst_sargs), storei(0);
		break;
	case 0xE8:		// Push to stack
		pushstack(*inst_args);
		break;
	case 0xE9:		// Pop from stack (different in v6 FIXME)
		if (*inst_args)
			store(*inst_args, stack[--stackptr]);

		else
			stack[stackptr - 2] =
			    stack[stackptr - 1], stackptr--;
		break;
	case 0xEA:		// Split window
		//NOP
		break;
	case 0xEB:		// Set active window
		window = *inst_args;
		break;

#if (VERSION > 3)
	case 0xEC:		// Call routine
		if (*inst_args) {
			program_counter++;
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 1, argc - 1);
		} else {
			storei(0);
		}
		break;
	case 0xED:		// Clear window
		if (*inst_args != 1) {
			cur_row = 1;	/* Will be bumped to 2 in call */
			textptr = 0;
			newline_margin();
		}
		break;
	case 0xEE:		// Erase line
		//NOP
		break;
	case 0xEF:		// Set cursor position
		//NOP
		break;
	case 0xF0:		// Get cursor position
		if (window) {
			write8(*inst_args, sc_rows);
			write8(*inst_args + 1, cur_column + 1);
		} else {
			write16(*inst_args, 0);
		}
		break;
	case 0xF1:		// Set text style
		//NOP
		break;
	case 0xF2:		// Buffer mode
		buffering = *inst_args;
		break;

#endif				/*  */
	case 0xF3:		// Select output stream
		switch_output(*inst_sargs);
		break;
	case 0xF4:		// Select input stream
		break;
	case 0xF5:		// Sound effects
		writes("\007");
		break;

#if (VERSION > 3)
	case 0xF6:		// Read a single character
		n = char_input();
		storei(n);
		break;
	case 0xF7:		// Scan a table
		if (argc < 4)
			inst_args[3] = 0x82;
		u = inst_args[1];
		while (inst_args[2]) {
			if (*inst_args ==
			    (inst_args[3] & 0x80 ? read16low(u) : read8low(u)))
				break;
			u += inst_args[3] & 0x7F;
			inst_args[2]--;
		}
		storei(inst_args[2] ? u : 0);
		branch(inst_args[2]);
		break;

#endif				/*  */
#if (VERSION > 4)
	case 0xF8:		// Not
		storei(~*inst_args);
		break;
	case 0xF9:		// Call routine and discard results
		if (*inst_args) {
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 0, argc - 1);
		}
		break;
	case 0xFA:		// Call routine and discard results
		if (*inst_args)
			enter_routine((*inst_args << PACKED_SHIFT) +
				      routine_start, 0, argc - 1);
		break;
	case 0xFB:		// Tokenise text
		if (argc < 4)
			inst_args[3] = 0;
		if (argc < 3)
			inst_args[2] = 0;
		tokenise(inst_args[0] + 2, inst_args[2], inst_args[1],
			 read8(inst_args[0] + 1), inst_args[3]);
		break;
	case 0xFC:		// Encode text in dictionary format
		{
			uint64_t y;

			/* FIXME memory ... */
			y = dictionary_encode(memory + inst_args[0] +
					      inst_args[2], inst_args[1]);
			write16(inst_args[3], y >> 16);
			write16(inst_args[3] + 2, y >> 8);
			write16(inst_args[3] + 4, y);
			break;
		}
	case 0xFD:		// Copy a table
		if (!inst_args[1]) {

			// zero!
			while (inst_args[2])
				write8(inst_args[0] + --inst_args[2], 0);
		} else if (inst_sargs[2] > 0
			   && inst_args[1] > inst_args[0]) {

			// backward!
			m = inst_sargs[2];
			while (m--)
				write8(inst_args[1] + m,
				       read8low(inst_args[0] + m));
		} else {

			// forward!
			if (inst_sargs[2] < 0)
				inst_sargs[2] *= -1;
			m = 0;
			while (m < inst_sargs[2])
				write8(inst_args[1] + m,
				       read8low(inst_args[0] + m), m);
			m++;
		}
		break;
	case 0xFE:		// Print a rectangle of text
		make_rectangle(inst_args[0], inst_args[1],
			       argc > 2 ? inst_args[2] : 1,
			       argc > 3 ? inst_sargs[3] : 0);

		// (I assume the skip is signed, since many other things are, and +32768 isn't useful anyways.)
		break;
	case 0xFF:		// Check argument count
		branch(frames[frameptr].argc >= *inst_args);
		break;

#endif				/*  */
	default:
#ifdef DEBUG
		fprintf(stderr,
			"\n*** Invalid instruction: %02X (near %06X)\n",
			in, program_counter);
		exit(1);
#else
		panic("illegal");
#endif				
		break;
	}
}

void game_begin(void)
{
	if (story == -1)
		story = open(story_name, O_RDONLY);
	if (story == -1) {
		error("\n*** Unable to load story file: ");
		error(story_name);
		panic("\n");
		exit(1);
	}
	lseek(story, 0L, SEEK_SET);
	read(story, memory, 64);
	if (read8low(0) != VERSION) {
		panic("\n*** Unsupported Z-machine version.\n");
		exit(1);
	}
	switch (VERSION) {
	case 1:
	case 2:
		write8(0x01, 0x10);
		break;
	case 3:
		write8(0x01,
			  (read8low(0x01) & 0x8F) | 0x10 | (tandy << 3));
		break;
	case 4:
		write8(0x01, 0x00);
		break;
	case 5:
		alphabet_table = read16low(0x34);
		break;
	case 7:
		routine_start = read16low(0x28) << 3;
		text_start = read16low(0x2A) << 3;
		alphabet_table = read16low(0x34);
		break;
	case 8:
		alphabet_table = read16low(0x34);
		break;
	}
	restart_address = read16low(0x06);
	dictionary_table = read16low(0x08);
	object_table = read16low(0x0A);
	global_table = read16low(0x0C);
	static_start = read16low(0x0E);
#ifdef DEBUG
	fprintf(stderr, "[%d blocks dynamic]\n", static_start >> 9);
#endif	
	write8(0x11, read8low(0x11) & 0x53);
	if (VERSION > 1)
		synonym_table = read16low(0x18);
	if (VERSION > 3) {
		write8(0x1E, tandy ? 11 : 1);
		write8(0x20, sc_rows);
		write8(0x21, sc_columns);
	}
	if (VERSION > 4) {
		write8(0x01, 0x10);
		write8(0x23, sc_columns);
		write8(0x25, sc_rows);
		write8(0x26, 1);
		write8(0x27, 1);
		write8(0x2C, 2);
		write8(0x2D, 9);
	}
	if (!(read8low(2) & 128))
		write16(0x02, 0x0802);
	write8(0x11, read8low(0x11) & 0x43);
	cur_row = 2;
	cur_column = 0;
	randomize(0);
	writes("\n");
}

static void usage(void)
{
	panic("fweep [-t] [-p] [-q] storyfile\n");
}

int main(int argc, char **argv)
{
	srand(getpid() ^ time(NULL));
	while(*++argv && *argv[0] == '-') {
		switch(*argv[1]) {
			case 't':
				tandy = 1;
				break;
			case 'p':
				original = 0;
				break;
			case 'q':
				qtospace = 0;
				break;
			default:
				usage();
		}
	}
	story_name = *argv++;
	if (!story_name || *argv)
		usage();
	game_begin();
	game_restart();
	for (;;)
		execute_instruction();
}
