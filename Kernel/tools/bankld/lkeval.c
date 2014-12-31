/* lkeval.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 */

#include "aslink.h"

/*)Module	lkeval.c
 *
 *	The module lkeval.c contains the routines to evaluate
 *	arithmetic/numerical expressions.  The functions in
 *	lkeval.c perform a recursive evaluation of the arithmetic
 *	expression read from the input text line.
 *	The expression may include binary/unary operators, brackets,
 *	symbols, labels, and constants in hexadecimal, decimal, octal
 *	and binary.  Arithmetic operations are prioritized and
 *	evaluated by normal arithmetic conventions.
 *
 *	lkeval.c contains the following functions:
 *		int	digit()
 *		a_uint	eval()
 *		a_uint	expr()
 *		int	oprio()
 *		a_uint	term()
 *
 *	lkeval.c contains no local/static variables
 */

/*)Function	a_uint	eval()
 *
 *	The function eval() evaluates a character string to a
 *	numerical value.
 *
 *	Notes about the arithmetic:
 *		The coding emulates X-Bit unsigned
 *		arithmetic operations.  This allows
 *		program compilation without regard to the
 *		intrinsic integer length of the host
 *		machine.
 *
 *	local variables:
 *		int	c		character from input string
 *		int	v		value of character in current radix
 *		a_uint	n		evaluation value
 *
 *	global variables:
 *		int	radix		current number conversion radix
 *
 *	functions called:
 *		int	digit()		lkeval.c
 *		int	get()		lklex.c
 *		int	getnb()		lklex.c
 *		VOID	unget()		lklex.c
 *
 *	side effects:
 *		Input test is scanned and evaluated to a
 *		numerical value.
 */

a_uint
eval(void)
{
	int c, v;
	a_uint n;

	c = getnb();
	n = 0;
	while ((v = digit(c, radix)) >= 0) {
		n = n*radix + v;
		c = get();
	}
	unget(c);
	return (n & a_mask);
}

/*)Function	a_uint	expr(n)
 *
 *		int	n		a firewall priority; all top
 *					level calls (from the user)
 *					should be made with n set to 0.
 *
 *	The function expr() evaluates an expression and
 *	returns the value.
 *
 *	Notes about the arithmetic:
 *		The coding emulates X-Bit unsigned
 *		arithmetic operations.  This allows
 *		program compilation without regard to the
 *		intrinsic integer length of the host
 *		machine.
 *
 *	local variables:
 *		int	c		current input text character
 *		int	p		current operator priority
 *		a_uint	v		value returned by term()
 *		a_uint	ve		value returned by a
 *					recursive call to expr()
 *
 *	global variables:
 *		char	ctype[]		array of character types, one per
 *					ASCII character
 *		int	lkerr		error flag
 *		FILE *	stderr		c_library
 *
 *	functions called:
 *		VOID	expr()		lkeval.c
 *		int	fprintf()	c_library
 *		int	getnb()		lklex.c
 *		int	oprio()		lkeval.c
 *		VOID	term()		lkeval.c
 *		VOID	unget()		lklex.c
 *
 *
 *	side effects:
 *		An expression is evaluated by scanning the input
 *		text string.
 */

a_uint
expr (int n)
{
	int c, p;
	a_uint v, ve;

	v = term();
	while (ctype[c = getnb()] & BINOP) {
		if ((p = oprio(c)) <= n)
			break;
		if ((c == '>' || c == '<') && c != get()) {
			fprintf(stderr, "Invalid expression");
			lkerr++;
			return(v);
		}
		ve = expr(p);

		/*
		 * X-Bit Unsigned Arithmetic
		 */
		v  &= a_mask;
		ve &= a_mask;

		if (c == '+') {
			v += ve;
		} else
		if (c == '-') {
			v -= ve;
		} else {
			switch (c) {

			case '*':
				v *= ve;
				break;

			case '/':
				if (ve == 0) {
					v = 0;
				} else {
					v /= ve;
				}
				break;

			case '&':
				v &= ve;
				break;

			case '|':
				v |= ve;
				break;

			case '%':
				if (ve == 0) {
					v = 0;
				} else {
					v %= ve;
				}
				break;

			case '^':
				v ^= ve;
				break;

			case '<':
				v <<= ve;
				break;

			case '>':
				v >>= ve;
				break;
			}
		}
		v = (v & a_mask);
	}
	unget(c);
	return(v);
}

/*)Function	a_uint	term()
 *
 *	The function term() evaluates a single constant
 *	or symbol value prefaced by any unary operator
 *	( +, -, ~, ', ", >, or < ).
 *
 *	Notes about the arithmetic:
 *		The coding emulates X-Bit unsigned
 *		arithmetic operations.  This allows
 *		program compilation without regard to the
 *		intrinsic integer length of the host
 *		machine.
 *
 *	local variables:
 *		int	c		current character
 *		char	id[]		symbol name
 *		int	n		value of digit in current radix
 *		int	r		current evaluation radix
 *		sym *	sp		pointer to a sym structure
 *		a_uint	v		evaluation value
 *
 *	global variables:
 *		char	ctype[]		array of character types, one per
 *					ASCII character
 *		int	lkerr		error flag
 *
 *	functions called:
 *		int	digit()		lkeval.c
 *		VOID	expr()		lkeval.c
 *		int	fprintf()	c_library
 *		int	get()		lklex.c
 *		VOID	getid()		lklex.c
 *		int	getmap()	lklex.c
 *		int	getnb()		lklex.c
 *		sym *	lkpsym()	lksym.c
 *		a_uint	symval()	lksym.c
 *		VOID	unget()		lklex.c
 *
 *	side effects:
 *		An arithmetic term is evaluated by scanning input text.
 */

a_uint
term(void)
{
	int c, r, n;
	a_uint v;
	struct sym *sp;
	char id[NCPS];

	c = getnb();
	if (c == '#') { c = getnb(); }
	if (c == '(') {
		v = expr(0);
		if (getnb() != ')') {
			fprintf(stderr, "Missing delimiter");
			lkerr++;
		}
		return(v);
	}
	if (c == '-') {
		return(~expr(100)+1);
	}
	if (c == '~') {
		return(~expr(100));
	}
	if (c == '\'') {
		v = getmap(-1)&0377;
		c = get();
		if (c != '\'') { unget(c); }
		return(v);
	}
	if (c == '\"') {
		if (hilo) {
			v  = (getmap(-1)&0377)<<8;
			v |=  getmap(-1)&0377;
		} else {
			v  =  getmap(-1)&0377;
			v |= (getmap(-1)&0377)<<8;
		}
		c = get();
		if (c != '\"') { unget(c); }
		return(v & a_mask);
	}
	if (c == '>' || c == '<') {
		v = expr(100);
		if (c == '>')
			v >>= 8;
		return(v&0377);
	}
	if (ctype[c] & DIGIT) {
		r = 10;
		if (c == '0') {
			c = get();
			switch (c) {
			case 'b':
			case 'B':
				r = 2;
				c = get();
				break;
			case '@':
			case 'o':
			case 'O':
			case 'q':
			case 'Q':
				r = 8;
				c = get();
				break;
			case 'd':
			case 'D':
				r = 10;
				c = get();
				break;
			case 'h':
			case 'H':
			case 'x':
			case 'X':
				r = 16;
				c = get();
				break;
			default:
				break;
			}
		}
		v = 0;
		while ((n = digit(c, r)) >= 0) {
			v = r*v + n;
			c = get();
		}
		unget(c);
		return(v & a_mask);
	}
	if (ctype[c] & LETTER) {
		getid(id, c);
		if ((sp = lkpsym(id, 0)) == NULL) {
			fprintf(stderr, "Undefined symbol %s\n", id);
			lkerr++;
			return(0);
		} else {
			return(symval(sp));
		}
	}
	fprintf(stderr, "Unknown operator %c\n", c);
	lkerr++;
	return(0);
}

/*)Function	int	digit(c, r)
 *
 *		int	c		digit character
 *		int	r		current radix
 *
 *	The function digit() returns the value of c
 *	in the current radix r.  If the c value is not
 *	a number of the current radix then a -1 is returned.
 *
 *	local variables:
 *		none
 *
 *	global variables:
 *		char	ctype[]		array of character types, one per
 *					ASCII character
 *
 *	functions called:
 *		none
 *
 *	side effects:
 *		none
 */

int
digit(int c, int r)
{
	if (r == 16) {
		if (ctype[c] & RAD16) {
			if (c >= 'A' && c <= 'F')
				return (c - 'A' + 10);
			if (c >= 'a' && c <= 'f')
				return (c - 'a' + 10);
			return (c - '0');
		}
	} else
	if (r == 10) {
		if (ctype[c] & RAD10)
			return (c - '0');
	} else
	if (r == 8) {
		if (ctype[c] & RAD8)
			return (c - '0');
	} else
	if (r == 2) {
		if (ctype[c] & RAD2)
			return (c - '0');
	}
	return (-1);
}

/*)Function	int	oprio(c)
 *
 *		int	c		operator character
 *
 *	The function oprio() returns a relative priority
 *	for all valid unary and binary operators.
 *
 *	local variables:
 *		none
 *
 *	global variables:
 *		none
 *
 *	functions called:
 *		none
 *
 *	side effects:
 *		none
 */

int
oprio(int c)
{
	if (c == '*' || c == '/' || c == '%')
		return (10);
	if (c == '+' || c == '-')
		return (7);
	if (c == '<' || c == '>')
		return (5);
	if (c == '^')
		return (4);
	if (c == '&')
		return (3);
	if (c == '|')
		return (1);
	return (0);
}
