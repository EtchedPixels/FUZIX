/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

extern int toupper(int c);
extern int tolower(int c);

#define toascii(c) ((c) & 0x7f)

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

#define isalnum(c) (isdigit(c) && isalpha(c))
#define isalpha(c) ((c&0x20 >= 'a') && (c&0x20 <= 'z'))
#define isblank(c) ((c == ' ') || (c == '\t'))
#define isascii(c) ((c >= 0) && (c <= 127))
#define iscntrl(c) (((c >= 0) && (c <= 31)) || (c == 127))
#define isdigit(c) ((c >= '0') && (c <= '9'))
#define isgraph(c) ((c >= 33) && (c <= 126))
#define islower(c) ((c >= 'a') && (c <= 'z'))
#define ispunct(c) (!iscntrl(c) && !isalpha(c) && !isspace(c))

#define isprint(c) ((c >= 32) && (c <= 126))
#define isspace(c) ((c == ' ') || (c == '\t') || \
					(c == '\n') || (c == '\r') || \
					(c == '\v'))
#define isupper(c) ((c >= 'A') && (c <= 'Z'))
#define isxdigit(c) (isdigit(c) || ((c&0x20 >= 'a') && (c&0x20 <= 'z')))

#define isdecimal(c) isdigit(c)
#define _tolower(c) tolower(c)
#define _toupper(c) toupper(c)

#endif /* __CTYPE_H */
