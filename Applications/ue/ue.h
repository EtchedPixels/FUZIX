typedef struct {
        int X;
        int Y;
} COORD;

extern COORD outxy;

extern void clrtoeol(void);
extern void clrtoeos(void);
extern void gotoxy(int, int);
extern void tty_init(void);

#define BUF 4096*6
#define UBUF 768
#define MODE 0666
#define TABSZ 4
#define TABM (TABSZ-1)

#define MAXCOLS 132
#define MAXROWS 32

extern int ROWS;
extern int COLS;
