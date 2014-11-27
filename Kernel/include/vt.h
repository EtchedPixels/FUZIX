#ifndef __VT_DOT_H__
#define __VT_DOT_H__

/* Optional defines */

#ifndef VT_MAP_CHAR
#define VT_MAP_CHAR(x) 	(x)
#endif

#ifndef VT_INITIAL_LINE
#define VT_INITIAL_LINE 0
#endif

/* Core functions */
void vtoutput(unsigned char *p, unsigned int len);
void vtinit(void);
/* Platform functions */
void clear_lines(int8_t y, int8_t ct);
void clear_across(int8_t y, int8_t x, int16_t l);
void cursor_off(void);
void cursor_on(int8_t y, int8_t x);
void scroll_up(void);
void scroll_down(void);
void plot_char(int8_t y, int8_t x, uint16_t c);
void do_beep(void);

#endif