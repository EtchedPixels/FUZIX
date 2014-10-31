#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define TIOCGETP  0
#define TIOCSETP  1
#define TIOCSETN  2
#define TIOCEXCL  3     /** currently not implemented  SN **/
#define UARTSLOW  4     /* Normal interrupt routine (UZI280) */
#define UARTFAST  5     /* Fast interrupt routine for modem usage (UZI280) */
#define TIOCFLUSH 6
#define TIOCGETC  7
#define TIOCSETC  8
              /* UZI280 extensions used by UZI180 in the CP/M 2.2 Emulator */
#define TIOCTLSET 9     /* Don't parse ctrl-chars */
#define TIOCTLRES 10    /* Normal Parse */

#define XTABS   0006000
#define RAW     0000040
#define CRMOD   0000020
#define ECHO    0000010
#define LCASE   0000004
#define CBREAK  0000002
#define COOKED  0000000

#define DFLT_MODE  (XTABS|CRMOD|ECHO|COOKED)

#define CTRL(c)  (c & 0x1f)

/* Character Input Queue size */
#define TTYSIZ 132

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;

    char t_intr;
    char t_quit;
    char t_start;
    char t_stop;
    char t_eof;

    char ctl_char;
};

void tty_echo(uint8_t minor, char c);
int tty_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int tty_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int tty_open(uint8_t minor, uint16_t flag);
int tty_close(uint8_t minor);
int tty_ioctl(uint8_t minor, uint16_t request, char *data);
void tty_putc_wait(uint8_t minor, char c);
void tty_inproc(uint8_t minor, char c);
void tty_outproc(uint8_t minor);
void tty_putc(uint8_t minor, char c);
bool tty_writeready(uint8_t minor);
void tty_pollirq(void);
#endif
