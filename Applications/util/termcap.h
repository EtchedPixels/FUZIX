#ifndef _TERMCAP_H
#define _TERMCAP_H

int  tgetent(char *_bp, char *_name);
int  tgetflag(char *_id);
int  tgetnum(char *_id);
char *tgetstr(char *_id, char **_area);
char *tgoto(char *_cm, int _destcol, int _destline);
int  tputs(char *_cp, int _affcnt, int (*_outc)(int));

#endif /* _TERMCAP_H */
