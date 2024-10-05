#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler.h"

/* Eventually remove stdio usage */

static char buf[20];

static char *uitoa(unsigned int i)
{
	char *p = buf + sizeof(buf);
	int c;

	*--p = '\0';
	do {
		c = i % 10;
		i /= 10;
		*--p = '0' + c;
	} while(i);
	return p;
}

unsigned errors;

static void writes(const char *p)
{
	write(2, p, strlen(p));
}

static void writec(const char c)
{
	write(2, &c, 1);
}

static void writeval(unsigned n)
{
	writes(uitoa(n));
}

void format_error(unsigned line, const char *p, const unsigned c)
{
	writes(filename);
	writec(':');
	writeval(line);
	writes(" - ");
	writes(p);
	if (c) {
		writes(" \"");
		writec(c);
		writec('"');
	}
	writec('\n');
}

void warningline(unsigned line, const char *p)
{
	format_error(line, p, 0);
}

void warning(const char *p)
{
	format_error(line_num, p, 0);
}

void errorline(unsigned line, const char *p)
{
	format_error(line, p, 0);
	errors++;
}

void error(const char *p)
{
	format_error(line_num, p, 0);
	errors++;
}

void fatal(const char *p)
{
	error(p);
	exit(255);
}

void errorc(const unsigned c, const char *p)
{
	format_error(line_num, p, c);
	errors++;
}

void needlval(void)
{
	error("lvalue required");
}

void badtype(void)
{
	error("bad type");
}


void indirections(void)
{
	error("too many indirections");
}

void typemismatch(void)
{
	error("type mismatch");
}

void invalidtype(void)
{
	error("invalid type");
}

void divzero(void)
{
	warning("division by zero");
}

void notconst(void)
{
	error("must evaluate to a constant");
}
