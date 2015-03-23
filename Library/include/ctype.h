/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

#include <stdint.h>

extern int toupper(int c);
extern int tolower(int c);

extern int isalnum(int c);
extern int isalpha(int c);
extern int isascii(int c);
extern int isblank(int c);
extern int iscntrl(int c);
extern int isdigit(int c);
extern int isgraph(int c);
extern int islower(int c);
extern int isprint(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);

#define isascii(c) (!((uint8_t)c & 0x80))
#define toascii(c) ((c) & 0x7f)

#define isdecimal isdigit
#define _tolower tolower
#define _toupper toupper

#endif /* __CTYPE_H */
