#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

/*
 *	Read and match against the token stream. Need to move from stdio
 *	eventually
 */

#define NO_TOKEN	0xFFFF		/* An unused value */


/*
 *	Simple block buffer read. We use 128 byte records so we can make
 *	this work in CP/M. For CP/M we'll also need to write some kind of
 *	'end of file' token
 */

static unsigned char inbuf[128];
static unsigned char *inptr;
static int inlen;

/* Read the next block. Hopefully this is the only routine we need to
   swap for CP/M etc */
static int in_record(void)
{
	inptr = inbuf;
	return read(0, inbuf, 128);
}

static int in_byte(void)
{
	if (inlen == 0)
		inlen = in_record();
	if (inlen--)
		return *inptr++;
	inlen = 0;
	return EOF;
}

static unsigned char outbuf[128];
static unsigned char *outptr = outbuf;
static unsigned int outlen;
static unsigned int outrecord = 0;

void out_write(void)
{
	if (lseek(1, outrecord * 128L, SEEK_SET) < 0)
		fatal("seek error");
	if (outlen && write(1, outbuf, outlen) != outlen)
		fatal("write error");
	outlen = 0;
	outptr = outbuf;
}

/* Again try and isolate the block I/O into two tiny routines */
void out_flush(void)
{
	out_write();
	outrecord++;
}

/* Read a record. We use this in situations where we need to rewind and
   update headers */
static void out_record_read(unsigned record)
{
	if (lseek(1, record * 128L, SEEK_SET) < 0)
		fatal("seek error");
	if (read(1, outbuf, 128) < 0)
		fatal("read error");
	outrecord = record;
}

/* Report the current record/offset */
unsigned long out_tell(void)
{
	return (outrecord << 8) | outlen;
}

/* Go to a given record/offset from before */
void out_seek(unsigned long pos)
{
	out_write();
	out_record_read(pos >> 8);
	outlen = pos & 0xFF;
	outptr = outbuf + outlen;
}

/* Add bytes at the current position */
void out_byte(unsigned char c)
{
	if (outlen == 128)
		out_flush();
	*outptr++ = c;
	outlen++;
}

void out_block(void *pv, unsigned len)
{
	unsigned char *p = pv;
	while(len) {
		unsigned n;

		/* Flush any full record */
		if (outlen == 128)
			out_flush();
		/* Fill up what we can */
		n = 128 - outlen;
		if (n > len)
			n = len;
		memcpy(outptr, p, n);
		outptr += n;
		outlen += n;
		p += n;
		len -= n;
	}
}

char filename[16];

unsigned line_num;

unsigned long token_value;
unsigned token;
unsigned last_token = NO_TOKEN;

unsigned tokbyte(void)
{
	unsigned c = in_byte();
	if (c == EOF) {
		error("corrupt stream");
		exit(1);
	}
	return c;
}

void next_token(void)
{
	int c;

	/* Handle pushed back tokens */
	if (last_token != NO_TOKEN) {
		token = last_token;
		last_token = NO_TOKEN;
		return;
	}

	c = in_byte();
	if (c == EOF) {
		token = T_EOF;
//        printf("*** EOF\n");
		return;
	}
	token = c;
	c = in_byte();
	if (c == EOF) {
		token = T_EOF;
		return;
	}
	token |= (c << 8);

	if (token == T_LINE) {
		char *p = filename;

		line_num = tokbyte();
		line_num |= tokbyte() << 8;

		if (line_num & 0x8000) {
			line_num &= 0x7FFF;
			for (c = 0; c < 16; c++) {
				*p = tokbyte();
				if (*p == 0)
					break;
				p++;
			}
		}
		next_token();
		return;
	}

	if (token == T_INTVAL || token == T_LONGVAL || token == T_UINTVAL
	    || token == T_ULONGVAL || token == T_FLOATVAL) {
		token_value = tokbyte();
		token_value |= tokbyte() << 8;
		token_value |= tokbyte() << 16;
		token_value |= tokbyte() << 24;
	}
}

/*
 * You can only push back one token and it must not have attached data. This
 * works out fine because we only ever need to push back a name when processing
 *  labels
 */
void push_token(unsigned t)
{
	last_token = token;
	token = t;
}

/*
 *	Try and move on a bit so that we don't generate a wall of errors for
 *	a single mistake
 */
void junk(void)
{
	while (token != T_EOF && token != T_SEMICOLON)
		next_token();
	next_token();
}

/*
 *	If the token is the one expected then consume it and return 1, if not
 *	do not consume it and leave 0. This lets us write things like
 *
 *	if (match(T_STAR)) { ... }
 */
unsigned match(unsigned t)
{
	if (t == token) {
		next_token();
		return 1;
	}
	return 0;
}

void need_semicolon(void)
{
	if (!match(T_SEMICOLON)) {
		error("missing semicolon");
		junk();
	}
}

/* This can only be used if the token is a single character token. That turns
   out to be sufficient for C so there is no need for anything fancy here */
void require(unsigned t)
{
	if (!match(t))
		errorc(t, "expected");
}

unsigned symname(void)
{
	unsigned t;
	if (token < T_SYMBOL)
		return 0;
	t = token;
	next_token();
	return t;
}

/*
 *	This is ugly and we need to review how we handle it
 */

static unsigned char pad_zero[2] = { 0xFF, 0xFE };

unsigned copy_string(unsigned label, unsigned maxlen, unsigned pad, unsigned lit)
{
	unsigned c;
	unsigned l = 0;

	header(H_STRING, label, lit);

	/* Copy the encoding string as is */
	while((c = tokbyte()) != 0) {
		if (l < maxlen) {
			out_byte(c);
			/* Quoted FFFF FFFE pairs count as one byte */
			if (c == 0xFF)
				out_byte(tokbyte());
			l++;
		}
	} while(c);

	/* No write any padding bytes */
	if (pad) {
		while(l++ < maxlen)
			out_block(&pad_zero, 2);
	}
	/* Write the end marker */
	out_byte(0);
	footer(H_STRING, label, l);

	next_token();
	if (token != T_STRING_END)
		error("bad token stream");
	next_token();
	return l;
}


unsigned label_tag;

unsigned quoted_string(int *len)
{
	unsigned l = 0;
	unsigned label = ++label_tag;

	if (token != T_STRING)
		return 0;

	l = copy_string(label, ~0, 0, 1);

	if (len)
		*len = l;

	return label;
}
