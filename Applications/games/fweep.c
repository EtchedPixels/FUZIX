/*
  Fweeplet -- a Z-machine interpreter for versions 1 to 5 and 8
  This program is license under GNU GPL v3 or later version.
  
  Cut down from 'fweep'

  V6 was mostly used for the graphical games, and V7 is a bit of a rarity
  so neither are obviously important. For V6 the packed shift is 4 but the
  routine calls bias it by 8* the routine offset, and strings by 8 * the
  string offset.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define IVERSION "fe0.01"

#if defined(__m68k__)
#define MACHINE_BIG
#endif

#define WORD(x)		((uint16_t)(x))

#ifndef VERSION
#define VERSION 	3
#endif

#if (VERSION == 8)

#define PACKED_SHIFT	3
typedef uint16_t obj_t;
/* Some V8 games need a lot of call frames which sucks */
#define STACKSIZE 512
#define FRAMESIZE 256
#define routine_start	0
#define text_start	0

#elif (VERSION == 7)

#define PACKED_SHIFT	3
typedef uint16_t obj_t;
#define STACKSIZE 512
#define FRAMESIZE 128
uint32_t routine_start;		/* > 16bit */
uint32_t text_start;		/* > 16bit */

#elif (VERSION == 6)

#define PACKED_SHIFT	3
typedef uint16_t obj_t;
#define STACKSIZE 512
#define FRAMESIZE 128
uint32_t routine_start;		/* > 16bit */
uint32_t text_start;		/* > 16bit */

#elif (VERSION > 3)

#define PACKED_SHIFT	2
typedef uint16_t obj_t;
#define STACKSIZE 512
#define FRAMESIZE 64
#define routine_start	0
#define text_start	0

#else				/*  */

#define PACKED_SHIFT	1
typedef uint8_t obj_t;
#define STACKSIZE 256
#define FRAMESIZE 32
#define routine_start	0
#define text_start	0

#endif				/*  */

#define UNPACK32(x)	(((uint32_t)x) << PACKED_SHIFT)

#if VERSION > 2
#define SHIFT1		4
#define SHIFT2		5
#else
#define	SHIFT1		2
#define SHIFT2		3
#endif

typedef char boolean;
typedef uint8_t byte;
typedef struct {
	uint32_t pc;
	uint16_t start;
	uint8_t argc;
	boolean stored;
} StackFrame;
const char zscii_conv_1[128] = {
	/* 128 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 136 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 144 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 152 */	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	/* 155... is the table */
	'a', 'o', 'u', 'A', 'O', 'U', 's', '>', '<', 'e', 'i', 'y',
	'E', 'I', 'a', 'e', 'i', 'o', 'u', 'y', 'A', 'E', 'I', 'O',
	'U', 'Y', 'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U',
	'a', 'e', 'i', 'o', 'u', 'A', 'E', 'I', 'O', 'U', 'a', 'A',
	'o', 'O', 'a', 'n', 'o', 'A', 'N', 'O', 'a', 'A', 'c', 'C',
	't', 't', 'T', 'T', 'L', 'o', 'O', '!', '?'
};

/* FIXME: probably smaller as function */
const char zscii_conv_2[128] = {
	/* 128 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 136 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 144 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 152 */	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	/* 155... is the table */
	'e', 'e', 'e', 0, 0, 0,
	's', '>', '<', 0, 0, 0, 0, 0, 0, 0, 0,
	/* 172 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 180 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 188 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 196 */	0, 0, 0, 0, 0, 0, 0, 0,
	/* 204 */	0, 0, 0, 0, 0, 0, 0,
	/* 211... */
	'e', 'E', 0, 0, 'h', 'h', 'h', 'h', 0, 'e', 'E'
};

#if (VERSION == 1)
uint8_t alpha[78] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789.,!?_#'\"/\\<-:()";
#else
uint8_t alpha[78] =
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
uint16_t object_table;
uint16_t dictionary_table;
uint16_t restart_address;
uint16_t synonym_table;
uint16_t alphabet_table;
uint16_t static_start;
uint16_t global_table;
uint32_t program_counter;	/* Can be in high memory */

StackFrame frames[FRAMESIZE];
StackFrame *framemax;
uint16_t stack[STACKSIZE];
StackFrame *frameptr;
int stackptr;
int stackmax;
uint16_t stream3addr[16];
uint16_t *stream3ptr;
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
uint8_t zch_shift;
uint8_t zch_shiftlock;
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

#ifndef LOAD_ALL

#ifdef MACHINE_BIG
#define ZBUF_MAX	64
#if (VERSION > 3)
#define ZBUF_SIZE	512
#define ZBUF_SHIFT	9
#else
#define ZBUF_SIZE	256
#define ZBUF_SHIFT	8
#endif
#else
#define ZBUF_MAX	32
#define ZBUF_SIZE	256
#define ZBUF_SHIFT	8
#endif

#define ZBUF_MASK (ZBUF_SIZE - 1)

/* Based on an idea by Staffan Vilcans: Skip the fseek() if it isn't needed */
static uint32_t last_addr;
static uint8_t last_count;
static uint8_t *last_ptr;
static uint16_t last_page;
static uint8_t zbuf_num = ZBUF_MAX;

static int pagefile = -1;

/* FIXME: dynamic for zbuf */
static uint8_t zbuf[ZBUF_MAX][ZBUF_SIZE];
static uint16_t zbuf_page[ZBUF_MAX];
static uint8_t zbuf_pri[ZBUF_MAX];	/* 0 = unused , 1+ is use count */
static uint8_t zbuf_dirty[ZBUF_MAX];

static uint16_t membreak;

uint8_t memory[64];

static uint8_t zbuf_alloc(void)
{
	uint8_t low = 255;
	uint8_t i, lnum = 0;
	for (i = 0; i < zbuf_num; i++) {
		if (zbuf_pri[i] == 0)
			return i;
		if (zbuf_pri[i] < low) {
			lnum = i;
			low = zbuf_pri[i];
		}
	}
	return lnum;
}

static void zbuf_sweep(void)
{
	uint8_t i;
	for (i = 0; i < zbuf_num; i++)
		if (zbuf_pri[i] > 1)
			zbuf_pri[i] /= 2;
}

static void zbuf_load(uint8_t slot, uint16_t page)
{
	int f = story;
	/* Invalidate any fast page pointer into this page */
	if (page == last_page)
		last_count = 0;
	zbuf_page[slot] = page;
	zbuf_pri[slot] = 0x80;
	if (page < membreak)
		f = pagefile;
//	fprintf(stderr, "Loading page %d int slot %d from %d\n", page, slot, f);
	if (lseek(f, ((off_t)page) << ZBUF_SHIFT, SEEK_SET) < 0 ||
	    read(f, zbuf[slot], ZBUF_SIZE) != ZBUF_SIZE) {
		perror(story_name);
		exit(1);
	}
#ifdef FAKE_DISK_DELAY
	usleep(20000);
#endif
}

static void zbuf_writeback(uint8_t slot)
{
//	fprintf(stderr, "Writing back slot %d (page %d)\n", slot,
//		zbuf_page[slot]);
	if (lseek(pagefile, ((off_t)zbuf_page[slot]) << ZBUF_SHIFT, SEEK_SET) < 0 ||
	    write(pagefile, zbuf[slot], ZBUF_SIZE) != ZBUF_SIZE) {
	    	perror("pagefile");
	    	exit(1);
	}
#ifdef FAKE_DISK_DELAY
	usleep(20000);
#endif
	zbuf_dirty[slot] = 0;
}

static uint8_t zbuf_find(uint16_t page)
{
	uint8_t i;
	for (i = 0; i < zbuf_num; i++) {
		if (zbuf_page[i] == page) {
			zbuf_pri[i] |= 0x80;
			return i;
		}
	}
	zbuf_sweep();
	i = zbuf_alloc();
	if (zbuf_dirty[i])
		zbuf_writeback(i);
	zbuf_load(i, page);
	return i;
}

static uint8_t zmem(uint32_t addr)
{
	uint8_t c;
	uint16_t page;
	uint16_t off;

	/* Fast path - current buffer */
	if (last_count && addr == last_addr + 1) {
		last_addr++;
		last_count--;
		return *++last_ptr;
	}

	page = addr >> ZBUF_SHIFT;
	c = zbuf_find(page);
	last_addr = addr;
	last_page = page;
	off  = ((uint16_t)addr) & ZBUF_MASK;
	last_count = ZBUF_SIZE - 1 - off;
	last_ptr = zbuf[c] + off;
	return *last_ptr;
}

void debugme(void)
{
}

static void zwrite(uint16_t addr, uint8_t value)
{
	/* FIXME: optimize */
	uint8_t p = zbuf_find(addr >> 8);
	zbuf[p][addr & ZBUF_MASK] = value;
	zbuf_dirty[p] = 1;
	/* Ugly : fix this better */
	if (addr < 64)
		memory[addr] = value;
}
	
/* Big endian */
static uint16_t zword(uint32_t addr)
{
	uint16_t r = zmem(addr) << 8;
	r |= zmem(addr + 1);
	return r;
}

/*
 *	Memory management
 */
uint8_t pc(void)
{
	return zmem(program_counter++);
}

uint16_t read16low(uint16_t address)
{
	return zword(address);
}

uint16_t read16(uint32_t address)
{
	return zword(address);
}

/* Can be uint16 except when debugging */
void write16(uint16_t address, uint16_t value)
{
	zwrite(address, value >> 8);
	zwrite(address + 1, value);
}

uint8_t read8low(uint16_t address)
{
	return zmem(address);
}

uint8_t read8(uint32_t address)
{
	return zmem(address);
}

void write8(uint16_t address, uint8_t value)
{
	zwrite(address, value);
}

static char tmpstr[] = "/tmp/fweepXXXXXX";
void paging_init(void)
{
	uint8_t i = 0;
	if (pagefile == - 1) {
		pagefile = mkstemp(tmpstr);
		if (pagefile == -1) {
			perror("create pagefile");
			exit(1);
		}
	}
	
	membreak = static_start >> ZBUF_SHIFT;

	lseek(story, 0, SEEK_SET);
	lseek(pagefile, 0, SEEK_SET);
	/* Copy the writable parts of the story into the page file */
	while(i++ < membreak) {
		if (read(story, zbuf[0], ZBUF_SIZE) != ZBUF_SIZE ||
			write(pagefile, zbuf[0], ZBUF_SIZE) != ZBUF_SIZE) {
			perror("copy pagefile");
			exit(1);
		}
	}
	memset(zbuf_page, 0xFF, sizeof(zbuf_page));
	memset(zbuf_dirty, 0, sizeof(zbuf_dirty));
	//lseek(story, 64, SEEK_SET);
	//if (read(story, memory + 64, sizeof(memory)) < 1024)
	//	panic("invalid story file.\n");
	/* Write back the proper header info */
	for (i = 0; i < 64; i++)
		write8(i, memory[i]);
}

void paging_restart(void)
{
	paging_init();
}

#else
		
/*
 *	Memory management: Really only here for debug work
 */

uint8_t memory[0x20000];

static uint8_t mget(uint32_t addr)
{
//	fprintf(stderr, "[%06X:%02X]\n", addr, memory[addr]);
	return memory[addr];
}

static void mput(uint32_t addr, uint8_t value)
{
//	fprintf(stderr, ">%06X:%02X<\n", addr, value);
	memory[addr] = value;
}


uint8_t pc(void)
{
	return mget(program_counter++);
}

uint16_t read16low(uint16_t address)
{
	uint16_t r = mget(address) << 8;
	r |= mget(address + 1);
	return r;
}

uint16_t read16(uint32_t address)
{
	uint16_t r = mget(address) << 8;
	r |= mget(address + 1);
	return r;
}

/* Can be uint16 except when debugging */
void write16(uint16_t address, uint16_t value)
{
	mput(address, value >> 8);
	mput(address + 1, value & 255);
}

uint8_t read8low(uint16_t address)
{
	return mget(address);
}

uint8_t read8(uint32_t address)
{
	return mget(address);
}

void write8(uint16_t address, uint8_t value)
{
	mput(address, value);
}

void paging_init(void)
{
}

void paging_restart(void)
{
	lseek(story, 64, SEEK_SET);
	if (read(story, memory + 64, sizeof(memory)) < 1024)
		panic("invalid story file.\n");
}

#endif

static uint16_t mword(uint8_t address)
{
	return (((uint16_t)memory[address]) << 8) | memory[address + 1];
}

uint16_t randv = 7;
boolean predictable;
int16_t get_random(int16_t max)
{
	/* This is what the official early interpreters appear to use */
	int16_t tmp;
	randv += 0xAA55;
	tmp = randv & 0x7FFF;
	randv = (randv << 8) | (randv >> 8);
#if 0
	/* Do we need more randomness for some later games ? */
	if (!predictable)
		randv ^= rand() >> 2;
#endif
	return (tmp % max) + 1;
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
		/* Report 8 pixels per char */
		write16(0x30, w * 8);
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

/* This is a pain but strictly speaking a game is entitled to change its
   alphabet on the fly! */
static void sync_alphabet(void)
{
#if VERSION >= 5
	uint8_t *p = alpha;
	uint16_t r;
	if ((r = alphabet_table) != 0) {
		while(p != alpha + 78)
			*p++ = read8(r++);
	}
#endif
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
			char_print(read8low
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
		return stack[frameptr->start + var - 1];
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
		stack[frameptr->start + var - 1] = value;
	} else {
		pushstack(value);
	}
}

void storei(uint16_t value)
{
	store(pc(), value);
}

static int depth = 0;

void enter_routine(uint32_t address, boolean stored, int argc)
{
	int c = read8(address);
	int i;

	fflush(stdout);
	if (frameptr == &frames[FRAMESIZE - 1])
		panic("out of frames.\n");

	frameptr->pc = program_counter;
	frameptr++;
	frameptr->argc = argc;
	frameptr->start = stackptr;
	frameptr->stored = stored;
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
		stack[frameptr->start + i] = inst_args[i + 1];
}

void exit_routine(uint16_t result)
{
	stackptr = frameptr->start;
	program_counter = (--frameptr)->pc;
	if (frameptr[1].stored)
		store(read8(program_counter - 1), result);
}

void branch(uint16_t cond)
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

/*
 *	Fetch a dictionary entry of 2 or 3 zwords into the passed
 *	word array
 */
void dictionary_get(uint16_t addr, uint16_t *p)
{
	uint8_t c = VERSION > 3 ? 3 : 2;
	uint16_t w;
	while (c--) {
		w = read8low(addr++);
		*p++ = (w << 8) | read8low(addr++);
	}
}

/*
 *	We implement the encoder as a state machine. If we do it differently
 *	we have to worry about truncation rules everywhere. As a state machine
 *	we can just stop calling it when done and everything just works.
 */

uint8_t wordstate;
#define WORD_END	1
#define WORD_SHIFT	2
#define WORD_LIT_1	3
#define WORD_LIT_2	4
#define WORD_LIT_3	5
#define WORD_BYTE	6
uint8_t *wordptr;
uint8_t *wordend;
uint8_t wordcode;

uint8_t encodesym(void)
{
	uint8_t i;
	switch(wordstate) {
	case WORD_END:
		/* We pad to the end with blanks (5) */
		return 5;
	case WORD_SHIFT:
		/* Second byte of a 2 byte sequence doing an alphabet shift */
		wordstate = WORD_BYTE;
		wordptr++;
		return wordcode;
	case WORD_LIT_1:
		/* Writing out a non encodable symbol, byte 2 is 6 */
		wordstate = WORD_LIT_2;
		return 6;
	case WORD_LIT_2:
		/* Then the 10bit code follows */
		wordstate = WORD_LIT_2;
		return *wordptr >> 5;
	case WORD_LIT_3:
		wordstate = WORD_BYTE;
		return *wordptr++ & 31;
	case WORD_BYTE:
		/* End padding */
		if (wordptr == wordend) {
			wordstate = WORD_END;
			return 5;
		}
		/* See if the symbol is encodable */
		for (i = 0; i < 78; i++) {
			if (*wordptr == alpha[i] && i != 52 && i != 53) {
				wordcode = i % 26 + 6;
				/* Shifted or not ? */
				if (i >= 26)
					return *wordptr / 26 + (VERSION > 2 ? 3 : 1);
				wordptr++;
				return wordcode;
			}
		}
		/* No - in which case start emitting a literal */
		wordstate = WORD_LIT_1;
		return VERSION > 2 ? 5 : 3;
	default:
		write(2,"Parsebad\n", 9);
		exit(1);
	}
}

uint16_t encodeword(void)
{
	uint16_t w;
	w = encodesym();
	w <<= 5;
	w |= encodesym();
	w <<= 5;
	w |= encodesym();
	return w;
}

void dictionary_encode(uint8_t *text, int len, uint16_t *wp)
{
	sync_alphabet();
	wordptr = text;
	wordend = text + len;
	wordstate = WORD_BYTE;
#if (VERSION > 3)
	*wp++ = encodeword();
#endif
	*wp++ = encodeword();
	*wp = encodeword() | 0x8000;
}


/*
 *	Encode a word into the parse buffer
 */
void add_to_parsebuf(uint16_t parsebuf, uint16_t dict, uint8_t * d,
		     int k, int el, int ne, int p, uint16_t flag)
{
	/* Encode the word into zscii */
	int i;
	uint16_t n = parsebuf + (read8(parsebuf + 1) << 2);
	uint16_t vbuf[3];
	uint16_t dbuf[3];

	dictionary_encode(d, k, vbuf);

	/* Hunt for a match */
	for (i = 0; i < ne; i++) {
		/* Get the next word and see if it matches */
		dictionary_get(dict, dbuf);
//		g = dictionary_get(dict) | 0x8000;
		if (memcmp(vbuf, dbuf, (VERSION > 3) ? 6 : 4) == 0) {
			/* It does - add the needed parse info */
			write8(n + 5, p + 1 + (VERSION > 4));
			write8(n + 4, k);
			write16(n + 2, dict);
			break;
		}
		dict += el;
	}
	/* No luck - we may need to write in a failure */
	if (i == ne && !flag) {
		write8(n + 5, p + 1 + (VERSION > 4));
		write8(n + 4, k);
		write16(n + 2, 0);
	}
	/* Finally bump the count */
	write8(parsebuf + 1, read8(parsebuf + 1) + 1);
}


#define Add_to_parsebuf() if(k)add_to_parsebuf(parsebuf,dict,d,k,el,ne,p1,flag),k=0;p1=p+1;

/*
 *	Process a command line input
 */

/* Out of the fn in order to build nicely on SDCC and CC65 - sigh */
static boolean ws[256];

void tokenise(uint16_t text, uint16_t dict, uint16_t parsebuf, int len,
	      uint16_t flag)
{
	uint8_t d[10];
	int i, el, ne, k, p, p1;
	int l;

	memset(ws, 0, 256 * sizeof(boolean));

	/* A big copy we should avoid */
	/* FIXME change algorithms */
	/* Read the table of character codes that count as a word */
	if (!dict) {
		l = read8(dictionary_table);
		for (i = 1; i <= l; i++)
			ws[read8(dictionary_table + i)] = 1;
		dict = dictionary_table;
	}
	l = read8(dict);
	for (i = 1; i <= l; i++)
		ws[read8(dict + i)] = 1;
	/* Parse buf count */
	write8(parsebuf + 1, 0);
	k = p = p1 = 0;
	/* Get the length and number of entries */
	el = read8low(dict + read8(dict) + 1);
	ne = read16low(dict + read8(dict) + 2);
	/* Binary search hint - not used */
	if (ne < 0)
		ne *= -1;	// Currently, it won't care about the order; it doesn't use binary search.
	/* Skip the header */
	dict += read8(dict) + 4;

	/* Walk the input */
	while (p < len && read8(text + p)
	       && read8(parsebuf + 1) < read8(parsebuf)) {
	        /* Get a symbol */
		i = read8(text + p);
		/* Case conversion */
		if (i >= 'A' && i <= 'Z')
			i += 'a' - 'A';
		/* Quiting rules */
		if (i == '?' && qtospace)
			i = ' ';
		/* Spaces break words - send the word to the buffer */
		if (i == ' ') {
			Add_to_parsebuf();
		} else if (ws[i]) {
			/* Symbols go the buffer on their own - queue any
			   pending stuff first, then the symbol */
			Add_to_parsebuf();
			*d = i;
			k = 1;
			Add_to_parsebuf();
		} else if (k < 10) {
			/* Queue more symbol */
			d[k++] = i;
		} else {
			/* Discard extra bytes */
			k++;
		}
		p++;
	}
	/* Add the final entry */
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
	stackptr = 0;
	frameptr = frames;
	program_counter = restart_address;
	paging_restart();
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
	   Save/Restore. We don't bother. Saved games are platform specific
	   Deal with it! */
	frameptr->pc = program_counter;
	frameptr[1].start = stackptr;
	
	xwrite(f, frames, frameptr - frames + 1, sizeof(StackFrame));
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
	int f, n;
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
	n = xread(f, frames, FRAMESIZE, sizeof(StackFrame));
	if (n == -1)
		goto bad;
	frameptr = frames + n - 1;
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
	program_counter = frameptr->pc;
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
	uint16_t u;
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

//	fprintf(stderr, "%02X\n", in);
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
		/* In theory we want shift-in shift-out here */
		if (!tandy) {
			uint8_t c = *inst_args == 3 ? 14 : 15;
			write(1, &c, 1);
		}
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
			storei(frameptr->start + *inst_args - 1);
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
			enter_routine(UNPACK32(*inst_args) +
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
		text_print(UNPACK32(*inst_args) + text_start);
		break;
	case 0x8E:		// Load variable
		at = fetch(*inst_args);
		store(*inst_args, at);	// if it popped from the stack, please put it back on
		storei(at);
		break;
	case 0x8F:		// Not // Call routine and discard result
		if (VERSION > 4) {
			if (*inst_args)
				enter_routine(UNPACK32(*inst_args)
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
			storei(frameptr - frames);
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
		at = attribute(*inst_args) + (inst_args[1] >> 3);
		write8(at, read8(at) | (0x80 >> (inst_args[1] & 7)));
		break;
	case 0xCC:		// Clear attribute
		at = attribute(*inst_args) + (inst_args[1] >> 3);
		write8(at, read8(at) & ~(0x80 >> (inst_args[1] & 7)));
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
			enter_routine(UNPACK32(*inst_args) +
				      routine_start, 1, argc - 1);
		} else {
			storei(0);
		}
		break;

#endif				/*  */
#if (VERSION > 4)
	case 0xDA:		// Call routine and discard result
		if (*inst_args)
			enter_routine(UNPACK32(*inst_args) +
				      routine_start, 0, argc - 1);
		break;
	case 0xDB:		// Set colors
		//NOP
		break;
	case 0xDC:		// Throw
		frameptr = frames + inst_args[1];
		exit_routine(*inst_args);
		break;
	case 0xDD:		// Bitwise XOR
		storei(inst_args[0] ^ inst_args[1]);
		break;

#endif				/*  */
	case 0xE0:		// Call routine (FIXME for v1)
		if (*inst_args) {
			program_counter++;
			enter_routine(UNPACK32(*inst_args) +
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
		else {
			randomize(-*inst_sargs);
			storei(0);
		}
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
			enter_routine(UNPACK32(*inst_args) +
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
			enter_routine(UNPACK32(*inst_args) +
				      routine_start, 0, argc - 1);
		}
		break;
	case 0xFA:		// Call routine and discard results
		if (*inst_args)
			enter_routine(UNPACK32(*inst_args) +
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
			uint8_t blob[10];
			uint16_t word[3];
			uint8_t i;

			for (i = 0; i < 9; i++)
				blob[i] = read8(inst_args[0] + inst_args[2] + i);

			dictionary_encode(blob, inst_args[1], word);
			write16(inst_args[3], word[0]);
			write16(inst_args[3] + 2, word[1]);
			write16(inst_args[3] + 4, word[2]);
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
			uint16_t m = inst_sargs[2];
			while (m--)
				write8(inst_args[1] + m,
				       read8low(inst_args[0] + m));
		} else {

			// forward!
			uint16_t m = 0;
			if (inst_sargs[2] < 0)
				inst_sargs[2] *= -1;
			while (m < inst_sargs[2]) {
				write8(inst_args[1] + m,
				       read8low(inst_args[0] + m));
				m++;
			}
		}
		break;
	case 0xFE:		// Print a rectangle of text
		make_rectangle(inst_args[0], inst_args[1],
			       argc > 2 ? inst_args[2] : 1,
			       argc > 3 ? inst_sargs[3] : 0);

		// (I assume the skip is signed, since many other things are, and +32768 isn't useful anyways.)
		break;
	case 0xFF:		// Check argument count
		branch(frameptr->argc >= *inst_args);
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

void version_init(void)
{
	switch (VERSION) {
	case 1:
	case 2:
		write8(0x01, 0x10);
		break;
	case 3:
		if (tandy)
			write8(0x01, (read8low(0x01) & 0x8F) | 0x18);
		else
			write8(0x01, (read8low(0x01) & 0x8F) | 0x10);
		break;
	case 4:
		write8(0x01, 0x00);
		break;
	case 5:
		alphabet_table = mword(0x34);
		break;
#if (VERSION == 6 || VERSION == 7)
	case 6:
	case 7:
		routine_start = mword(0x28) << 3;
		text_start = mword(0x2A) << 3;
		alphabet_table = mword(0x34);
		break;
#endif
	case 8:
		alphabet_table = mword(0x34);
		break;
	}

	write8(0x11, read8low(0x11) & 0x53);
	if (VERSION > 1)
		synonym_table = mword(0x18);
	if (VERSION > 3) {
		write8(0x1E, tandy ? 11 : 1);
		write8(0x20, sc_rows);
		write8(0x21, sc_columns);
	}
	if (VERSION > 4) {
		write8(0x01, 0x10);
		write16(0x22, sc_columns << 3);
		write16(0x24, sc_rows << 3);
		write8(0x26, 8);
		write8(0x27, 8);
		write8(0x2C, 2);
		write8(0x2D, 9);
	}
	if (!(read8low(2) & 128))
		write16(0x02, 0x0802);
	write8(0x11, read8low(0x11) & 0x43);
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
	if (memory[0] != VERSION) {
		panic("\n*** Unsupported Z-machine version.\n");
		exit(1);
	}
	restart_address = mword(0x06);
	dictionary_table = mword(0x08);
	object_table = mword(0x0A);
	global_table = mword(0x0C);
	static_start = mword(0x0E);
#ifdef DEBUG
	fprintf(stderr, "[%d blocks dynamic]\n", static_start >> 9);
#endif
	paging_init();
	version_init();
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

	/* cc65 isn't smart enough to do this at compile time */
	stream3ptr = stream3addr - 1;

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
