/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

#include <stdint.h>

#if !defined(__CC65__)
	#if !defined HAVE_STATIC_INLINE
		#define HAVE_STATIC_INLINE 1
	#endif
#endif

#if HAVE_STATIC_INLINE

static inline int isalpha(int c)
{ return (((uint8_t)c|0x20) >= 'a') && (((uint8_t)c|0x20) <= 'z'); }

static inline int isblank(int c)
{ return ((uint8_t)c == ' ') || ((uint8_t)c == '\t'); }

static inline int isascii(int c)
{ return ((uint8_t)c >= 0) && ((uint8_t)c <= 127); }

static inline int iscntrl(int c)
{ return (((uint8_t)c >= 0) && ((uint8_t)c <= 31)) || ((uint8_t)c == 127); }

static inline int isdigit(int c)
{ return ((uint8_t)c >= '0') && ((uint8_t)c <= '9'); }

static inline int isgraph(int c)
{ return ((uint8_t)c >= 33) && ((uint8_t)c <= 126); }

static inline int islower(int c)
{ return ((uint8_t)c >= 'a') && ((uint8_t)c <= 'z'); }

static inline int isprint(int c)
{ return ((uint8_t)c >= 32) && ((uint8_t)c <= 126); }

static inline int isspace(int c)
{ return ((uint8_t)c == ' ') || ((uint8_t)c == '\t') ||
         ((uint8_t)c == '\n') || ((uint8_t)c == '\r') ||
         ((uint8_t)c == '\v') || ((uint8_t)c == '\f'); }

static inline int isupper(int c)
{ return ((uint8_t)c >= 'A') && ((uint8_t)c <= 'Z'); }

static inline int isalnum(int c)
{ return isdigit(c) || isalpha(c); }

static inline int ispunct(int c)
{ return isascii(c) && !iscntrl(c) && !isalnum(c) && !isspace(c); }

static inline int isxdigit(int c)
{ return isdigit(c) || ((((uint8_t)c|0x20) >= 'a') && (((uint8_t)c|0x20) <= 'f')); }

static inline int tolower(int c)
{
	if (isupper(c))
		return (uint8_t)c ^ 0x20;
	return c;
}

static inline int toupper(int c)
{
	if (islower(c))
		return (uint8_t)c ^ 0x20;
	return c;
}

#define isdecimal isdigit

#else

extern int isalnum(int c);
extern int isalpha(int c);
extern int isascii(int c);
extern int isblank(int c);
extern int iscntrl(int c);
extern int isdigit(int c);
extern int isgraph(int c);
extern int islower(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);
extern int tolower(int c);
extern int toupper(int c);

#define isdecimal(c) isdigit(c)

#endif

#define _tolower(c) tolower(c)
#define _toupper(c) toupper(c)

#endif /* __CTYPE_H */
