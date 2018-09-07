/*
 *	A small subset of the CC65 tgi API
 */

extern void tgi_init(void);
extern void tgi_done(void);
extern void tgi_setpalette(const unsigned char *palette);
extern void tgi_clear(void);
extern void tgi_setcolor(unsigned char c);
extern void tgi_setpixel(int x, int y);
extern void tgi_bar(int x1, int y1, int x2, int y2);
extern void tgi_line(int x1, int y1, int x2, int y2);
