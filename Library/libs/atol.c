/*
 *	Implement atol directly because
 *	- our multiply is constant (so way faster on 8bit)
 *	- atol should not touch errno so is not a trivial strtoul wrapper
 *	  if you are being correct about it.
 */

#include <stdlib.h>
#include <ctype.h>

long atol(const char *str)
{
	uint8_t neg = 0;
	long sum = 0;

	while(isspace(*str))
		str++;
	if (*str == '-') {
		neg = 1;
		str++;
	} else if (*str == '+')
		str++;
	while(isdigit(*str)) {
		sum *= 10;
		sum += *str++ - '0';
	}
	return neg ? -sum : sum;
}
