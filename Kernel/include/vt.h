#ifndef __VT_DOT_H__
#define __VT_DOT_H__

#include <keycode.h>

/* Optional defines */

#ifndef MAX_VT
#define MAX_VT	1
#endif

#ifndef VT_MAP_CHAR
#define VT_MAP_CHAR(x) 	(x)
#endif

#ifndef VT_INITIAL_LINE
#define VT_INITIAL_LINE 0
#endif

struct vt_switch {
  uint8_t vtmode;
  signed char cursorx;
  signed char cursory;
  signed char ncursory;
};

/* Core functions */
void vtoutput(unsigned char *p, unsigned int len);
void vtinit(void);
/* Mode switcher functions */
void vt_save(struct vt_switch *vt);
void vt_load(struct vt_switch *vt);
/* Platform functions */
void clear_lines(int8_t y, int8_t ct);
void clear_across(int8_t y, int8_t x, int16_t l);
void cursor_off(void);
void cursor_on(int8_t y, int8_t x);
void scroll_up(void);
void scroll_down(void);
void plot_char(int8_t y, int8_t x, uint16_t c);
void do_beep(void);
int vt_ioctl(uint8_t minor, uint16_t op, char *ptr);
int vt_inproc(uint8_t minor, unsigned char c);

#endif