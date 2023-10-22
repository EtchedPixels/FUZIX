/*
 *	Tokenizer
 *
 *	It might be nicer to switch to an algorithm with less meta-data
 *	but we have to balance code size/data size/speed
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "symtab.h"
#include "token.h"
#include "target.h"

#if defined(__linux__)
/* _itoa */
static char buf[7];

char *_uitoa(unsigned int i)
{
	char *p = buf + sizeof(buf);
	int c;

	*--p = '\0';
	do {
		c = i % 10;
		i /= 10;
		*--p = '0' + c;
	} while (i);
	return p;
}

char *_itoa(int i)
{
	char *p;
	if (i >= 0)
		return _uitoa(i);
	p = _uitoa(-i);
	*--p = '-';
	return p;
}

#endif


static unsigned char filename[33] = { "<stdin>" };

static unsigned filechange = 1;

static int isoctal(unsigned char c)
{
	if (c >= '0' && c <= '7')
		return 1;
	return 0;
}

static int iscsymbol(unsigned char c)
{
	if (c == '_' || isalnum(c))
		return 1;
	return 0;
}

static int iscsymstart(unsigned char c)
{
	if (c == '_' || isalpha(c))
		return 1;
	return 0;
}

/*
 *	Glue for now
 */

static unsigned err;
static unsigned line = 1;
static unsigned oldline = 0;

static void colonspace(void)
{
	write(2, ": ", 2);
}

static void writes(const char *p)
{
	unsigned len = strlen(p);
	write(2, p, len);
}

static void report(char code, const char *p)
{
	writes((const char *) filename);
	colonspace();
	writes(_itoa(line));
	colonspace();
	write(2, &code, 1);
	colonspace();
	writes(p);
	write(2, "\n", 1);
}

void error(const char *p)
{
	report('E', p);
	err++;
}

void warning(const char *p)
{
	report('W', p);
}

void fatal(const char *p)
{
	error(p);
	exit(1);
}

#define BLOCK 512

static uint8_t buffer[BLOCK];	/* 128 for CPM */
static uint8_t *bufptr = buffer + BLOCK;
static uint16_t bufleft = 0;

/* Pull the input stream in blocks and optimize for our case as this
   is of course a very hot path. This design allows for future running
   on things like CP/M and with the right block size is also optimal for
   Fuzix */

static unsigned bgetc(void)
{
	if (bufleft == 0) {
		bufleft = read(0, buffer, BLOCK);
		if (bufleft == 0)
			return EOF;
		bufptr = buffer;
	}
	bufleft--;
	return *bufptr++;
}

static unsigned pushback;
static unsigned pbstack[2];
static unsigned isnl = 1;
static unsigned lastbslash;

static void directive(void);

unsigned get(void)
{
	int c;
	if (pushback) {
		c = pbstack[--pushback];
		pushback = 0;
		if (c == '\n') {
			isnl = 1;
			line++;
		}
		return c;
	}
	c = bgetc();
	while (c == '#' && isnl) {
		directive();
		c = bgetc();
	}
	isnl = 0;
	if (c == '\n') {
		line++;
		isnl = 1;
	}
	/* backslash newline continuation */
	if (lastbslash && c == '\n')
		c = bgetc();

	if (c == '\\')
		lastbslash = 1;
	else
		lastbslash = 0;

	if (c == EOF)
		return 0;
	return c;
}

unsigned get_nb(void)
{
	unsigned c;
	do {
		c = get();
	} while (c && isspace(c));
	return c;
}

void unget(unsigned c)
{
	if (pushback > 2)
		fatal("double pushback");
	pbstack[pushback++] = c;
	if (c == '\n')
		line--;
}

void required(unsigned cr)
{
	unsigned c = get();
	if (c != cr) {
		error("expected quote");
		unget(c);
	}
}

/* # directive from cpp # line file - # line "file" */
/* TODO file name saving */
static void directive(void)
{
	unsigned char *p = filename;
	unsigned c;

	line = 0;

	do {
		c = bgetc();
	} while (isspace(c));

	while (isdigit(c)) {
		line = 10 * line + c - '0';
		c = bgetc();
	}
	if (c == '\n')
		return;

	/* Should be a quote next */
	c = bgetc();
	if (c == '"') {
		while ((c = bgetc()) != EOF && c != '"') {
			/* Skip magic names */
			if (p == filename && c == '<')
				p = filename + 32;
			if (c == '/')
				p = filename;
			else if (p < filename + 32)
				*p++ = c;
		}
		filechange = 1;
	}
	*p = 0;
	while ((c = bgetc()) != EOF) {
		if (c == '\n')
			return;
	}
	fatal("bad cpp");
}


#define NHASH	64

/* We could infer the symbol number from the table position in theory */

static struct name symbols[MAXNAME];
static struct name *nextsym = symbols;
static struct name *symbase;	/* Base of post keyword symbols */
static struct name *symhash[NHASH];
/* Start of symbol range */
static unsigned symnum = T_SYMBOL;

/*
 *	Add a symbol to our symbol tables as we discover it. Log the
 *	fact if tracing.
 */
static struct name *new_symbol(const char *name, unsigned hash, unsigned id)
{
	struct name *s;
	if (nextsym == symbols + MAXNAME)
		fatal("too many sybmols");
	s = nextsym++;
	strncpy(s->name, name, NAMELEN);
	s->next = symhash[hash];
	s->id = id;
	symhash[hash] = s;
	return s;
}

/*
 *	Find a symbol in a given has table	
 */
static struct name *find_symbol(const char *name, unsigned hash)
{
	struct name *s = symhash[hash];
	while (s) {
		if (strncmp(s->name, name, NAMELEN) == 0)
			return s;
		s = s->next;
	}
	return NULL;
}

/*
 *	A simple but adequate hashing algorithm. A better one would
 *	be worth it for performance.
 */
static unsigned hash_symbol(const char *name)
{
	int hash = 0;
	uint8_t n = 0;

	while (*name && n++ < NAMELEN)
		hash += *name++;
	return (hash & (NHASH - 1));
}

static void write_symbol_table(void)
{
	unsigned len = (uint8_t *) nextsym - (uint8_t *) symbase;
	uint8_t n[2];

	/* FIXME: proper temporary file! */
	int fd = open(".symtmp", O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1) {
		perror(".symtmp");
		exit(1);
	}
	n[0] = len;
	n[1] = len >> 8;
	if (write(fd, n, 2) != 2 || write(fd, symbase, len) != len)
		error("symbol I/O");
	close(fd);
}

/*
 *	Token stream writing. We have a single special case to handle
 *	which is strings.
 */

static uint8_t outbuf[BLOCK];
static uint8_t *outptr = outbuf;

static void outbyte(unsigned char c)
{
	*outptr++ = c;
	if (outptr == outbuf + BLOCK) {
		outptr = outbuf;
		if (write(1, outbuf, BLOCK) != BLOCK)
			error("I/O");
	}
}

static void outflush(void)
{
	unsigned len = outptr - outbuf;
	if (len && write(1, outbuf, len) != len)
		error("I/O");
}

static void outbyte_quoted(unsigned char c)
{
	if (c == 0 || c == 0xFF)
		outbyte(0xFF);
	if (c == 0)
		outbyte(0xFE);
	else
		outbyte(c);
}

static unsigned char tokdata[8];
static unsigned char *tokptr = tokdata;

static void encode_byte(unsigned c)
{
	*tokptr++ = c;
}

static void write_token(unsigned c)
{
	unsigned char *tp;
	unsigned n = 0;
	if (oldline != line || filechange) {
		oldline = line;
		outbyte(T_LINE & 0xFF);
		outbyte(T_LINE >> 8);
		outbyte(line);
		if (filechange) {
			outbyte(0x80 | (line >> 8));
			tp = filename;
			while (*tp && n++ < 32)
				outbyte(*tp++);
			outbyte(0);
		} else
			outbyte(line >> 8);
		filechange = 0;
	}
	/* Write the token, then any data for it */
	outbyte(c);
	outbyte(c >> 8);
	tp = tokdata;
	while (tp < tokptr)
		outbyte(*tp++);
	/* Reset the data pointer */
	tokptr = tokdata;
}

/* C keywords, ignoring all the modern crap */

static const char *keytab[] = {
	/* Types */
	"char",
	"double",
	"enum",
	"float",
	"int",
	"long",
	"short",
	"signed",
	"struct",
	"union",
	"unsigned",
	"void",
	/* Storage classes */
	"auto",
	"extern",
	"register",
	"static",
	/* Modifiers */
	"const",
	"volatile",
	/* Then the rest */
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
	"for",
	"goto",
	"if",
	"return",
	"sizeof",
	"switch",
	"typedef",
	"while",
	/* Nonsense */
	"restrict",
	NULL
};

/* Add keywords. These get added first so they head the hash lists */
static void keywords(void)
{
	const char **p = keytab;
	int i = T_KEYWORD;
	while (*p) {
		new_symbol(*p, hash_symbol(*p), i++);
		p++;
	}
	symbase = nextsym;
}

/* Read up to 14 more bytes into the symbol name, plus a terminator */
static void get_symbol_tail(char *p)
{
	unsigned n = 14;
	unsigned c;
	while ((c = get()) != 0) {
		if (!iscsymbol(c))
			break;
		if (n) {
			n--;
			*p++ = c;
		}
	}
	*p = 0;
	unget(c);
}

/* Also does keywords */
static unsigned tokenize_symbol(unsigned c)
{
	char symstr[16];
	unsigned h;
	struct name *s;
	*symstr = c;
	get_symbol_tail(symstr + 1);
	/* We can't do cunning tricks to spot labels in this pass because
	   foo: is ambiguous between a label and a ?: */
	h = hash_symbol(symstr);
	s = find_symbol(symstr, h);
	if (s)
		return s->id;
	return new_symbol(symstr, h, symnum++)->id;
}

/*
 *	Software float encoding. This is a bit long winded because
 *	we want it to work on an 8bit micro on a compiler that
 *	has no floating point so that you can bootstrap an FP
 *	compiler with an integer only one.
 */

/*
 *	This does all the clever stuff and is about the only
 *	IEEE754 dependent part of the operation except negate.
 *
 *	We are passed a 32.32 fixed point value in sum/frac along
 *	with a binary exponent (exp) from building the bits, and a
 *	decimal exponent from the user.
 *
 *	We turn this into a 28bit mantissa for ease of manipulation
 *	and then exponentiate for the passed exponent adjusting the
 *	binary exponent as we go to keep the bits. Finally we normalize it
 *	to 24bit manitissa and assemble an unsigned IEEE 754 float.
 *
 *	This is not paritcularly fast. It's adequate for the compiler
 *	but it's not clear there is any gain from using an FP version
 *	for FP capable compilers.
 */

static uint16_t rtype;
static uint32_t result;

static void overflow(void)
{
	error("overflow");
}

static void exp_overflow(void)
{
	warning("exponent under/overflow");
}

static void convert_fix32(int exp, uint32_t sum, uint32_t frac, int uexp)
{
	rtype = T_FLOATVAL;
	if (sum == 0 && frac == 0) {
		result = 0;
		return;
	}

	/* Start by getting it down to 28bits */
	while (!(sum & 0x08000000)) {
		sum <<= 1;
		if (frac & 0x80000000)
			sum |= 1;
		frac <<= 1;
		exp--;
	}
	/* A 28bit number will never overflow when multiplied by ten */
	while (uexp > 0) {
		sum *= 10;
		/* Shift it down to keep it fitting */
		while (sum & 0xF0000000) {
			sum >>= 1;
			exp++;
		}
		uexp--;
	}
	/* Fix up the exponent in the other direction */
	while (uexp < 0) {
		sum /= 10;
		while (!(sum & 0x08000000)) {
			sum <<= 1;
			exp--;
		}
		uexp++;
	}

	exp += 22;
	/* If the sum has too many bits then shift it down and adjust
	   the exponent */
	while (sum & 0xFF000000) {
		sum >>= 1;
		exp++;
	}
	/* If there are leading zero bits, then shift up and merge in frac */
	while ((sum & 0x00800000) == 0) {
		sum <<= 1;
		if (frac & 0x80000000)
			sum++;
		frac <<= 1;
		exp--;
	}

	exp += 128;

	/* No denormals */
	if (exp < 1) {
		exp_overflow();
		result = 0;	/* Zero */
		return;
	}
	if (exp > 254) {
		exp_overflow();
		result = 0x7F800000;	/* Infinity */
		return;
	}

	/* Assemble result */
	sum &= 0x007FFFFF;
	sum |= (exp << 23) & 0x7F800000UL;
	result = sum;
}

/* After the E or P in a floating point value is a signed decimal exponent.
   Parse this */
static int parse_exponent(void)
{
	uint32_t sum = 0, n;
	int neg = 1;
	unsigned c;

	c = get();
	if (c == '-') {
		neg = -1;
		c = get();
	} else if (c == '+')
		c = get();

	/* Parse integer digits only */
	while (isdigit(c)) {
		c -= '0';
		n = sum * 10 + c;
		if (n < sum)
			overflow();
		sum = n;
		c = get();
	}
	unget(c);
	/* If it is out of the range then error */
	if (sum > 128)
		exp_overflow();
	return sum * neg;
}

/* Decimal float format digits.digitsEdigits. We don't handle
   the 0.0000000000000000000000000000000001 type silly yet */

/* Table of decminal fractions as binary value. We only need 24bit
   precision worst case so this is adequate */
static const uint32_t fraction[10] = {
	/* 0.100000 */ 0x199999A0,
	/* 0.010000 */ 0x28F5C28,
	/* 0.001000 */ 0x418937,
	/* 0.000100 */ 0x68DB8,
	/* 0.000010 */ 0xA7C5,
	/* 0.000001 */ 0x10C6,
	/* 0.000000 */ 0x1AD,
	/* 0.000000 */ 0x2A,
	/* 0.000000 */ 0x4,
	0
};

static void dec_format(unsigned c)
{
	uint32_t sum = 0, frac = 0, n;
	unsigned ex = 0, uex;

	/* Parse digits before . : could be integer or float */
	while (c != 'E' && c != 'e' && c != '.') {
		if (!isdigit(c)) {
			/* Done */
			unget(c);
			if (ex == 0) {
				result = sum;
				rtype = T_INTVAL;
				return;
			}
			/* Oversized integer */
			overflow();
			return;
		}
		c -= '0';
		n = sum * 10 + c;
		/* If we wrap then keep shifting */
		/* There's probably a better way to do this */
		while (n < sum) {
			sum >>= 1;
			ex++;
			n = sum * 10 + c;
		}
		sum = n;
		c = get();
	}
	/* We have done the integer part, and found floaty stuff */
	n = 0;
	/* Parse any fractional part using the fraction table */
	if (c == '.') {
		while (1) {
			c = get();
			if (c == 'E' || c == 'e')
				break;
			if (!isdigit(c)) {
				unget(c);
				convert_fix32(ex, sum, frac, 0);
				return;
			}
			c -= '0';
			if (fraction[n]) {
				frac += c * fraction[n];
				n++;
			}
		}
	}
	/* If we shifted the non fractional part to make it fit then we
	   don't need the frac bits */
	if (ex)
		frac = 0;
	/* Now look for an exponent */
	if (c == 'E' || c == 'e')
		uex = parse_exponent();
	else
		unget(c);
	convert_fix32(ex, sum, frac, uex);
}

/*
 *	Hex format
 *	- parse a hex number
 *	- if we find a P or a . then it's a float
 */

static unsigned unhex(unsigned c)
{
	c = toupper(c);
	c -= '0';
	if (c > 9)
		c -= 7;
	return c;
}

static void hex_format(void)
{
	uint32_t sum = 0, frac = 0, n;
	unsigned c;
	int ct;
	unsigned ex = 0, uex;

	/* Parse digits before . : could be integer or float */
	while (1) {
		c = get();
		if (c == '.' || c == 'P' || c == 'p')
			break;
		if (!isxdigit(c)) {
			/* Done */
			unget(c);
			if (ex == 0) {
				result = sum;
				rtype = T_INTVAL;
				return;
			}
			/* Oversized integer */
			error("range");
			return;
		}
		c = unhex(c);
		n = sum << 4;

		if (n >= sum)
			sum = n + c;
		else
			/* Digits we are parsing are not relevant, but remember
			   the shift */
			ex += 4;
	}
	/* We have done the integer part, and found floaty stuff */
	ct = 28;

	/* If we stopped parsing because we had too many bits then
	   don't add in the fractional part */
	if (ex)
		ct = -1;

	if (c == '.') {
		while (1) {
			c = get();
			if (c == 'P' || c == 'p')
				break;
			if (!isxdigit(c)) {
				unget(c);
				convert_fix32(ex, sum, frac, 0);
				return;
			}
			/* Keep adding on fraction bits that fit */
			c = unhex(c);
			/* Only add relevant fractions */
			if (ct >= 0) {
				frac |= c << n;
				ct -= 4;
			}
		}
	}
	/* Now look for an exponent */
	if (c == 'P' || c == 'p')
		uex = parse_exponent();
	else
		unget(c);
	convert_fix32(ex, sum, frac, uex);
}

/*
 *	We parsed a 0, check for 0x or octal
 */
static void oct_format(void)
{
	uint32_t sum = 0, n;
	unsigned c;

	c = get();
	if (c == 'x' || c == 'X') {
		hex_format();
		return;
	}
	if (c == '.') {
		/* Implied fractional part of decimal */
		dec_format(c);
		return;
	}

	while (c >= '0' && c <= '7') {
		n = (sum << 3) + c - '0';
		if (n < sum)
			overflow();
		sum = n;
		c = get();
	}
	/* Done */
	unget(c);
	result = sum;
	rtype = T_INTVAL;
}

/*
 *	Leading digit
 *	0	octal or hex
 *	other	decimal
 *
 *	Parse a C number. The statics result and rtype
 *	are set up as the bits and the float/int status
 */
static void parse_digits(unsigned c)
{
	if (c == '0')
		oct_format();
	else
		dec_format(c);
}


/*
 *	TODO longlong if we add it to the compiler
 */
static unsigned tokenize_numeric(unsigned c, unsigned neg)
{
	unsigned force_unsigned = 0;
	unsigned force_long = 0;
	unsigned force_float = 0;
	unsigned cup;

	parse_digits(c);

	/* Look for trailing type information */
	while (1) {
		c = get();
		cup = toupper(c);
		if (cup == 'F' && !force_float)
			force_float = 1;
		else if (cup == 'U' && !force_unsigned)
			force_unsigned = 1;
		else if (cup == 'L' && !force_long)
			force_long = 1;
		else {
			unget(c);
			break;
		}
	}
	/* UF is not valid but LF or FL is a double */
	if (force_float && force_unsigned)
		error("invalid type specifiers");

	if (force_float && rtype != T_FLOATVAL) {
		convert_fix32(0, result, 0, 0);
		/* This also sets rtype */
	}
	if (neg) {
		/* Assumes IEEE 754 */
		if (rtype == T_FLOATVAL)
			result ^= 0x80000000UL;
		else
			result = -result;
	}
	if (rtype != T_FLOATVAL) {
		/* Anything can be shoved in a ulong */
		rtype = T_ULONGVAL;
		/* FIXME: this needs review for the -32768 case */
		/* Will it fit in a uint ? */
		if (!force_long && result <= TARGET_MAX_UINT) {
			rtype = T_UINTVAL;
			if (!force_unsigned && result <= TARGET_MAX_INT)
				rtype = T_INTVAL;
		} else if (!force_unsigned) {
			/* Maybe a signed long then ? */
			if (result <= TARGET_MAX_LONG)
				rtype = T_LONGVAL;
			/* Will it fit in a signed integer ? */
			if (!force_long && result <= TARGET_MAX_INT)
				rtype = T_INTVAL;
		}
	}
	if (neg) {
		/* Assumes IEEE754 */
		if (rtype == T_FLOATVAL)
			result ^= 0x80000000UL;
		else
			result = -result;
	}
	/* Order really doesn't matter here so stick to LE. We will worry about 
	   actual byte order in the code generation */
	encode_byte(result);
	encode_byte(result >> 8);
	encode_byte(result >> 16);
	encode_byte(result >> 24);
	return rtype;
}

static unsigned tokenize_number(unsigned c)
{
	return tokenize_numeric(c, 0);
}

static unsigned tokenize_neg(unsigned c)
{
	return tokenize_numeric(c, 1);
}

static unsigned hexpair(void)
{
	unsigned c, c2;
	c = get();
	if (!isxdigit(c)) {
		warning("invalid hexadecimal escape");
		return T_INVALID;
	}
	c2 = get();
	if (!isxdigit(c2)) {
		warning("invalid hexadecimal escape");
		return T_INVALID;
	}
	return (unhex(c2) << 4) | unhex(c);
}

static unsigned octalset(unsigned c)
{
	unsigned int n = c - '0';
	int ct = 1;
	while (ct++ < 3) {
		c = get();
		if (!isoctal(c)) {
			unget(c);
			return n;
		}
		n <<= 3;
		n |= c - '0';
	}
	return n;
}

static unsigned escaped(unsigned c)
{
	/* Simple cases first */
	switch (c) {
	case 'a':
		return 0x07;
	case 'b':
		return 0x08;
	case 'e':
		return 0x1B;	/* Non standard but common */
	case 'f':
		return 0x0C;
	case 'n':
		return 0x0A;
	case 'r':
		return 0x0D;
	case 't':
		return 0x09;
	case 'v':
		return 0x0B;
	case '\\':
		return '\\';
	case '\'':
		return '\'';
	case '"':
		return '"';
	case '?':
		return '?';	/* Not that we suport the trigraph nonsense */
	}
	/* Now the numerics */
	if (c == 'x')
		return hexpair();
	if (isdigit(c))
		return octalset(c);
	warning("invalid escape code");
	return T_INVALID;
}

static unsigned tokenize_char(void)
{
	unsigned c = get();
	unsigned c2;
	if (c != '\\') {
		/* Encode as a value */
		encode_byte(c);
		encode_byte(0);
		encode_byte(0);
		encode_byte(0);
		c = get();
		if (c != '`') {
			unget(c);
			required('\'');
		}
		return T_INTVAL;
	}
	c2 = get();
	c = escaped(c2);
	required('\'');
	if (c == T_INVALID)
		/* Not a valid escape */
		encode_byte(c2);
	else
		encode_byte(c);
	encode_byte(0);
	encode_byte(0);
	encode_byte(0);
	return T_INTVAL;
}

static unsigned tokenize_string(void)
{
	/* We escape any internal \0 or \FF so we can parse this without
	   buffers, and likewise write it to data the other end the same way */
	unsigned c, c2;
	write_token(T_STRING);

	/* This is slightly odd because we do the string catenation here too */
	do {
		while ((c = get()) != '"') {
			if (c != '\\') {
				outbyte_quoted(c);
			} else {
				c2 = get();
				c = escaped(c2);
				if (c == T_INVALID)
					outbyte_quoted(c2);
				else
					outbyte_quoted(c);
			}
		}
		c = get_nb();
	} while (c == '"');
	unget(c);
	outbyte_quoted(0);
	outbyte(0);
	return T_STRING_END;
}

static char *doublesym = "+-=<>|&";
static char *symeq = "+-/*^!|&%<>";
static char *unibyte = "()[]{}&*/%+-?:^<>|~!=;.,";

static unsigned tokenize(void)
{
	unsigned c, c2, c3;
	char *p;

	c = get_nb();
	if (c == 0)
		return T_EOF;
	if (iscsymstart(c))
		return tokenize_symbol(c);
	if (isdigit(c))
		return tokenize_number(c);
	if (c == '\'')
		return tokenize_char();
	if (c == '"')
		return tokenize_string();
	/* Look for things like ++ and the special case of -n for constants */
	c2 = get();
/*	if (c == '-' && isdigit(c2))
		return tokenize_neg(c2); */
	/* Until we fix the negative handling we need to deal with the
	   a = -.1 case specially. When we fix minus parsing this all goes
	   away */
	if (c == '-' && c2 == '.')
		return tokenize_neg(c2);
	/* Funny case - whilst . is a token . followed by a digit is part
	   of a number */
	if (c == '.' && isdigit(c2)) {
		unget(c2);
		return tokenize_number(c);
	}
	if (c2 == c) {
		p = strchr(doublesym, c);
		if (p) {
			if (c == '<' || c == '>') {
				c3 = get();
				if (c3 == '=') {
					if (c == '<')
						return T_SHLEQ;
					return T_SHREQ;
				}
				unget(c3);
			}
			/* Double sym */
			return T_DOUBLESYM + p - doublesym;
		}
	}
	/* Now deal with the other double symbol cases */
	if (c == '-' && c2 == '>')
		return T_POINTSTO;
	if (c == '.' && c2 == '.') {
		c3 = get();
		if (c3 == '.')
			return T_ELLIPSIS;
		unget(c3);
	}
	/* The '=' cases */
	if (c2 == '=') {
		p = strchr(symeq, c);
		if (p)
			return T_SYMEQ + p - symeq;
	}
	unget(c2);
	/* Symbols that only have a 1 byte form */
	p = strchr(unibyte, c);
	if (p)
		return c;	/* Map to self */
	/* Not valid C */
	error("nonsense in C");
	/* I'm a teapot */
	return T_POT;
}

/* Tokenizer as a standalone pass */
int main(int argc, char *argv[])
{
	unsigned t;
	keywords();
	do {
		t = tokenize();
		write_token(t);
	} while (t != T_EOF);
	/* Write the remaining decode */
	outflush();
	write_symbol_table();
	return err;
}
