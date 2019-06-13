/*
 *	This module deals with keeping the BASIC program present and tokenized
 *
 *	The code ends with a fake line 65535 which we must ensure the user
 *	never gets to replace as it avoids us ever having to special case
 *	last line.
 *
 *	The interpreter core uses memory above lines_end for data. We don't
 *	do anything to make life easy for it if lines are added.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "basic.h"

uint8_t blob[2048];
uint8_t *lines = blob;
uint8_t *lines_end;
uint8_t *lines_limit = &blob[2047];
uint8_t *execp;
uint16_t run_line;
uint8_t *next_line;
uint8_t running;
uint8_t pending_go;
uint16_t go_target;
uint8_t no_colon;
uint16_t editbase;
uint16_t newest;


uint8_t cursor_x;
uint8_t cursor_y;
uint8_t width = 80;
uint8_t height = 24;

static uint8_t last_ch;

static void print_ch(uint8_t c)
{
	if (c == 13)
		cursor_x = 0;
	else if (c == 10) {
		cursor_x = 0;
		if (cursor_y < height - 1)
			cursor_y++;
	}
	else if (c == 8) {
		if (cursor_x)
			cursor_x--;
		else
			return;
	}
	else if (c == '\t') {
		do {
			print_ch(' ');
		} while(cursor_x % 8);
		return;
	} else {
		cursor_x++;
		if (cursor_x == width - 1) {
			if (cursor_y < height - 1)
				cursor_y++;
			cursor_x = 0;
		}
	}
	putchar(c);
	last_ch = c;
}

static void print_str(char *c)
{
	while(*c)
		print_ch((uint8_t)*c++);
}
	

static void printat(uint8_t y, uint8_t x)
{
	printf("\033[%d;%dH", y, x);
	cursor_x = x;
	cursor_y = y;
}

static void wipe(void)
{
	printat(0,0);
	printf("\033[J");
}

static char toktab[] = {
#include "tokens.h"
	0xFF
};

static uint8_t *outptr;

/* Input is a 7bit basic token with 0x80 on the last byte */
/* Could binary search this as we can always find a marker with 0x80 to
   split the search in halfish */

static uint8_t tokget(uint8_t * c)
{
	uint8_t *ptr = toktab;
	uint8_t *cp = c;
	uint8_t v;
	uint8_t n = TOKEN_BASE;

	while (*ptr != 0xFF) {
		/* keep matching bytes */
		while (*ptr == *cp) {
			/* If we match and 0x80 is set then we matched end bytes
			   and we got a hit */
			if (*ptr++ & 0x80)
				return n;
			/* If we matched top bit clear then keep matching */
			cp++;
		}
		/* We matched until a '.' in the input that indicates a short form
		   except for the case of '.' */
		if (*cp == ('.' | 0x80) && cp != c)
			return n;
		/* When the match fails walk on until the start of the next match
		   in the table */
		while (!(*ptr++ & 0x80));
		cp = c;
		n++;
	}
	return 0;
}

/* TODO - don't tokenize past a REM token */
void tokenize_line(uint8_t * input)
{
	uint8_t *p = input;
	uint8_t t;

	while (*p) {
		/* Strip spaces */
		while (*p && isspace(*p))
			p++;
		if (*p == 0)
			break;
		*p &= 127;
		/* Strings require special handling so we don't tokenize them */
		if (*p == '"') {
			*outptr++ = *p++;
			while (*p != '"') {
				if (*p == 0)
					require('"');
				*outptr++ = *p++;
			}
			*outptr++ = *p++;
		} else {
			uint8_t *f = p++;
			/* Otherwise we keep adding letters and trying to tokenize. This
			   is inefficient but we don't do it often */
			while (*p) {
				*p |= 0x80;
				t = tokget(f);
				if (t == 0)
					*p++ &= 0x7f;
				else {
					*outptr++ = t;
					p++;
					break;
				}
			}
			if (t)
				continue;
			*outptr++ = *f;
			p = f + 1;
		}
	}
	*outptr++ = 0;
}

/* Find the line (or next one). As we have a dummy line 65535 this never
   fails to find something */
uint8_t *find_line(uint16_t line)
{
	uint8_t *ptr = lines;
	/* We pad lines on alignment icky processors */
	while (*(uint16_t *) ptr < line)
		ptr += *(uint16_t *) (ptr + 2);	/* Length in bytes */
	return ptr;
}

/* Currently we linear search. It's possible to do clever things but we don't
   as it's not a performance critical spot. Pass 0 size for a delete */
void insdel_line(uint16_t num, uint8_t * toks, uint16_t size)
{
	uint8_t *ptr = lines;
	int16_t shift = size;
	uint8_t *cptr;

	/* The return stack sits above us so it goes when we add a line */
	clear_return_stack();

	if (size == 1) {
		size = 0;	/* End mark only - delete */
		shift = 0;
	}
	if (size) {
		size += 4;	/* Length, number */
		shift += 4;
	}

	/* This is conservative - we should check for deletion/insertion to see
	   if a replaced line will fit.. but it's close to the line anyway so
	   already deep in trouble */

	/* FIXME: use variable base as a guide instead */
	if (size + lines_end > lines_limit)
		error(ERROR_MEMORY);

	/* We pad lines on alignment icky processors */
	while (*(uint16_t *) ptr < num)
		ptr += *(uint16_t *) (ptr + 2);	/* Length in bytes */

	cptr = ptr;

	/* Our line exists, we will need to shift by the difference in line
	   size, and also we need to move relative to the next line */
	if (*(uint16_t *) ptr == num) {
		/* Find the next line */
		shift -= *(uint16_t *) (ptr + 2);
		cptr = ptr + *(uint16_t *) (ptr + 2);	/* Length in bytes */
	}
	if (shift) {
		memmove(cptr + shift, cptr, lines_end - cptr);
		lines_end += shift;
	}
	if (size) {
		*(uint16_t *)ptr = num;
		*(uint16_t *)(ptr + 2) = size;
		memcpy(ptr + 4, toks, size - 4);
		lines_end += size;
	}
}

uint8_t *detok(uint8_t c)
{
	static uint8_t rv;
	uint8_t *p;

	rv = c | 0x80;
	if (c < TOKEN_BASE)
		return &rv;
	p = toktab;

	c -= TOKEN_BASE;
	while (c--) {
		while (!(*p++ & 0x80));
		if (*p == 0xFF) {
			fprintf(stderr, "badtok\n");
			exit(1);
		}
	}
	return p;
}

void print_line(uint8_t * p)
{
	uint8_t *o;
	while (*p) {
		/* Quoted text */
		if (*p == '"') {
			do {
				print_ch(*p++);
			} while (*p != '"');
			print_ch(*p++);
			/* Symbols */
		} else if (*p < TOKEN_BASE) {
			print_ch(*p);
			p++;
		}
		/* Tokens */
		else {
			if (last_ch != ' ')
				print_ch(' ');
			o = detok(*p++);
			do {
				print_ch(*o & 0x7F);
			} while (!(*o++ & 0x80));
			if (o[-1] != '(')
				print_ch(' ');
		}
	}
}

void find_exec_line(uint16_t l)
{
	uint8_t *ptr = find_line(l);
	run_line = *(uint16_t *)ptr;
	next_line = ptr + *(uint16_t *)(ptr + 2);
}


/* Might also be worth having renumber but that involves parsing each line
   and fixing up anything that's a constant for goto/gosub etc - messy */

void error(int n)
{
	char tmpbuf[64];
	sprintf(tmpbuf, "\nError %d/%d\n", n, run_line);
	print_str(tmpbuf);
	running = 0;
	longjmp(aborted, 1);
}

/* FIXME: merge with makenumber */
uint8_t *linenumber(uint8_t * n, uint16_t * v)
{
	uint16_t vp = 0, vn;
	while (isdigit(*n)) {
		vn = vp * 10 + *n - '0';
		if (vn < vp)
			goto bad;
		vp = vn;
		n++;
	}
	*v = vp;
	return n;

      bad:
	error(ERROR_OVERFLOW);
	return NULL;
}

/*
 *	Wipe and initialize the program area with the single dummy line 0xFFFF
 */

void new_command(void)
{
//    statememt_end();
	lines[0] = 0xFF;
	lines[1] = 0xFF;
	/* FIXME: endianness */
	lines[2] = 0x01;
	lines[3] = 0x00;
	lines[4] = TOK_END;	/* An END marker so we always end */
	lines[5] = 0x00;
	lines_end = lines + 6;
//    variables = lines_end;
//    *variables++ = 0;
}

void cls_command(void)
{
	wipe();
}

void clear_command(void)
{
	clear_variables();
}


/*
 *	Wipe and initialize the program area with the single dummy line 0xFFFF
 */

void new_command(void)
{
//    statememt_end();
	lines[0] = 0xFF;
	lines[1] = 0xFF;
	/* FIXME: endianness */
	lines[2] = 0x01;
	lines[3] = 0x00;
	lines[4] = TOK_END;	/* An END marker so we always end */
	lines[5] = 0x00;
	lines_end = lines + 6;
	clear_command();
}

void do_list_command(uint16_t low, uint16_t high, int must)
{
	char tmp[6];
	uint8_t *p;
	uint16_t n;
	int c = 0;
	uint8_t seen = 0;

retry:
	wipe();

	p = find_line(low);

	for (n = *(uint16_t *) p; n <= high; p += *(uint16_t *) (p + 2)) {
		n = *(uint16_t *)p;
		if (n == must)
			seen = 1;
		/* End marker - never print */
		if (n == 0xFFFF)
			break;
		sprintf(tmp, "%5d ", n);
		print_str(tmp);
		print_line(p + 4);
		print_ch('\n');
		if (must && cursor_y == height - 5) {
			if (seen)
				return;
			editbase += (must - editbase) / 2;
			low = editbase;
			goto retry;
		}
	}
}

void editor_update(void)
{
	wipe();
	do_list_command(editbase, 65534, newest);
}

void list_command(void)
{
	do_list_command(0,65534,0);
}

void run_command(void)
{
	find_exec_line(0);
	execute_line_content(execp);
}

void go_statement(void) {
	uint8_t *c = *execp++;
	if (c == TOK_SUB)
		stack_frame();
	else if (c != TOK_TO) {
		error(SYNTAX_ERROR);
		return;
	}
	if (int_expr(&l) == 0)
		return;
	find_exec_line(l);
}

void let_statement(void) {
	struct variable *v;
	struct value r;
	v = find_variable();
	if (*execp++ != '=')
		require('=');
	else {
		expression(&r);
		variable_assign(v, r);
	}
}

void if_statement(void) {
	uint8_t b = intexpression();
	if (*execp++ != TOK_THEN)
		require(TOK_THEN);
	if (b == 0 && run_line)
		find_exec_line(run_line + 1);
	else
		no_colon = 1;
}
	
void return_command(void)
{
	if (unstack_frame() == 0)
		error(ERROR_RET_UNDERFLOW);
}

void run_command(void)
{
	clear_command();
	find_exec_line(0);
	execute_line_content(execp);
}

void do_print_input(uint8_t in)
{
	uint8_t need_punc = 0;
	while(ch = *execp++) {
		if (c == '"' && !need_punc) {
			while((c = *execp++) != '"') {
				if (c == 0) {
					error(MISSING_QUOTE);
					return;
				}
				print_ch(c);
			}
			need_punc = 1;
		}
		/* TODO: AT TAB and SPC(x) */
		if (c == ',') {
			print_ch('\t');
			need_punc = 0;
			continue;
		}
		if (c == ';') {
			need_punc  0;
			continue;
		}
		if (need_punc)
			error(SYNTAX_ERROR);
		if (in) {
			struct variable v;
			variable_name(&v);
			do_input(&v);
		} else
			print_expression();
	}
}
	
void execute_statement(void)
{
	switch(*execp++) {
	case '?':
	case TOK_PRINT:
		print_command();
		break;
	case TOK_IF:
		if_command();
		break;
	case TOK_GO:
		go_command();
		break;
	case TOK_LET:
		let_command();
		break;
	case TOK_INPUT:
		input_command();
		break;
	case TOK_RETURN:
		return_command();
		break;
	case TOK_CLEAR:
		clear_command();
		break;
	case TOK_LIST:
		list_command();
		break;
	case TOK_RUN:
		run_command();
		break;
	case TOK_END:
		running = 0;
		return;
	default:
		execp--;
		let_command();
		break;
	}
}

void execute_line_content(uint8_t *l)
{
	uint8_t c;
	execp = l;
	running = 1;
	do {
		execute_statement();
		if (!running)
			return;
		if (no_colon)
			continue;
		if(pending_go) {
			find_exec_line(go_target);
			continue;
		}
		c = *execp++;
		if (c == 0)
			return;
	} while(c == ':');
	error(SYNTAX_ERROR);
}

void parse_line(uint8_t * l, int s)
{
	uint16_t n;
	if (isdigit(*l)) {
		uint8_t *dp = linenumber(l, &n);
		if (l == NULL)
			return;
		newest = n;
		insdel_line(n, dp, s - (dp - l));
		editor_update();
		printat(height-4, 0);
		return;
	}
	run_line = 0;
	execute_line_content(l);
}

int main(int argc, char *argv[])
{
	char buf[256];
	uint8_t o[256];
	uint8_t *p;

	cls_command();
	printf("Fuzix Basic 0.1\n");
	printf("%d bytes free.\n", lines_limit - lines);

	new_command();

	while (1) {
		fgets(buf, 256, stdin);
		outptr = o;
		tokenize_line(buf);
		parse_line(o, outptr - o);
	}
}
