/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

#if !defined(__CC65__)
	#if !defined HAVE_STATIC_INLINE
		#define HAVE_STATIC_INLINE 1
	#endif
#endif

#if HAVE_STATIC_INLINE

static inline int isalpha(int c)
{ return ((c|0x20) >= 'a') && ((c|0x20) <= 'z'); }

static inline int isblank(int c)
{ return (c == ' ') || (c == '\t'); }

static inline int isascii(int c)
{ return (c >= 0) && (c <= 127); }

static inline int iscntrl(int c)
{ return ((c >= 0) && (c <= 31)) || (c == 127); }

static inline int isdigit(int c)
{ return (c >= '0') && (c <= '9'); }

static inline int isgraph(int c)
{ return (c >= 33) && (c <= 126); }

static inline int islower(int c)
{ return (c >= 'a') && (c <= 'z'); }

static inline int isprint(int c)
{ return (c >= 32) && (c <= 126); }

static inline int isspace(int c)
{ return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ||
         (c == '\v') || (c == '\f'); }

static inline int isupper(int c)
{ return (c >= 'A') && (c <= 'Z'); }

static inline int isalnum(int c)
{ return isdigit(c) || isalpha(c); }

static inline int ispunct(int c)
{ return isascii(c) && !iscntrl(c) && !isalnum(c) && !isspace(c); }

static inline int isxdigit(int c)
{ return isdigit(c) || (((c|0x20) >= 'a') && ((c|0x20) <= 'f')); }

static inline int tolower(int c)
{
	if (isupper(c))
		c ^= 0x20;
	return c;
}

static inline int toupper(int c)
{
	if (islower(c))
		c ^= 0x20;
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
extern int isoctal(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);

#define isdecimal(c) isdigit(c)

#endif

#define _tolower(c) tolower(c)
#define _toupper(c) toupper(c)

#endif /* __CTYPE_H */
