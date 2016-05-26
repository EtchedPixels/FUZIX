/* temporary replacement for a working <sys/tty.h> */

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};
#define TIOCGWINSZ	10
#define VTSIZE		0x22

