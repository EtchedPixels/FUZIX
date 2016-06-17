/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

#include <stdint.h>

extern int toupper(int __c);
extern int tolower(int __c);

extern int isalnum(int __c);
extern int isalpha(int __c);
extern int isascii(int __c);
extern int isblank(int __c);
extern int iscntrl(int __c);
extern int isdigit(int __c);
extern int isgraph(int __c);
extern int islower(int __c);
extern int isprint(int __c);
extern int ispunct(int __c);
extern int isspace(int __c);
extern int isupper(int __c);
extern int isxdigit(int __c);

#define isascii(c) (!((uint8_t)(c) & 0x80))
#define toascii(c) ((c) & 0x7f)

#endif /* __CTYPE_H */
