#ifndef _LINEIN_H
#define _LINEIN_H

extern int tty_event(void);
extern void tty_hide(void);
extern void tty_show(void);
extern int tty_begin(void);
extern int tty_restore(void);
extern int tty_resume(void);
extern void tty_set_buffer(char *base, int promptlen, int totalsize);

extern int tty_width;

#endif
