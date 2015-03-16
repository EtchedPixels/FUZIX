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
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);
extern int isoctal(int c);

#define isdecimal isdigit
#define isprint(c) (!iscntrl(c))
#define _tolower tolower
#define _toupper toupper

#endif /* __CTYPE_H */
