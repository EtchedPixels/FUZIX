#ifndef __TERMCAP_H
#define __TERMCAP_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern char PC;
extern char *UP;
extern char *BC;
extern int ospeed;

extern int tgetent(char *, const char *);
extern int tgetflag(char *);
extern int tgetnum(char *);
extern char *tgetstr(char *, char **);

extern int tputs __P((const char *, int, int (*outc)(int)));
extern char *tgoto __P((const char *, int, int));

#endif /* _TERMCAP_H */
