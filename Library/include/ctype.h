/* ctype.h	Character classification and conversion
 */
#ifndef __CTYPE_H
#define __CTYPE_H

#include <features.h>
extern unsigned char __ctype[];

#define __CT_c	0x01		/* control character */
#define __CT_u	0x02		/* upper case */
#define __CT_l	0x04		/* lower case */
#define __CT_d	0x08		/* numeric digit */
#define __CT_s	0x10		/* whitespace */
#define __CT_p	0x20		/* punctuation */
#define __CT_x	0x40		/* hexadecimal */

#define __CT_a	(__CT_u|__CT_l) /* alpha */

/* always functions ! */
extern int toupper __P((int));
extern int tolower __P((int));

#define _toupper(c)	(islower(c) ? (c)^0x20 : (c))
#define _tolower(c)	(isupper(c) ? (c)^0x20 : (c))
#define __toupper(c)	((c)^0x20)
#define __tolower(c)	((c)^0x20)
#define toascii(c)	((c)&0x7F)

#define _CTYPE(c)	(__ctype[(unsigned char)(c)])

/* Note the '!!' is a cast to 'bool' and even BCC deletes it in an if()  */
#define isascii(c)	(!((c)&~0x7F))
#define isalnum(c)	(!!(_CTYPE(c)&(__CT_a|__CT_d)))
#define isalpha(c)	(!!(_CTYPE(c)&__CT_a))
#define iscntrl(c)	(!!(_CTYPE(c)&__CT_c))
#define isdigit(c)	(!!(_CTYPE(c)&__CT_d))
#define isgraph(c)	(! (_CTYPE(c)&(__CT_c|__CT_s)))
#define islower(c)	(!!(_CTYPE(c)&__CT_l))
#define isprint(c)	(! (_CTYPE(c)&__CT_c))
#define ispunct(c)	(!!(_CTYPE(c)&__CT_p))
#define isspace(c)	(!!(_CTYPE(c)&__CT_s))
#define isupper(c)	(!!(_CTYPE(c)&__CT_u))
#define isxdigit(c)	(!!(_CTYPE(c)&__CT_x))

#define isdecimal(c)	isdigit(c)
#define isoctal(c)	((c) >= '0' && (c) <= '7')

#endif /* __CTYPE_H */
