#ifndef __TERMCAP_H
#define __TERMCAP_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern char PC;
extern char *UP;
extern char *BC;
extern int ospeed;

extern int tgetent(char *__bp, const char *__name);
extern int tgetflag(char *__id);
extern int tgetnum(char *__id);
extern char *tgetstr(char *__id, char **__area);

extern int tputs(const char *__str, int __affcnt, int (*__putc)(int ch));
extern char *tgoto(const char *__cap, int __col, int __row);

#endif /* _TERMCAP_H */
