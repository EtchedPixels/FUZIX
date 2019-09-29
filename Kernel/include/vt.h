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
  uint8_t vtattr;
#define VTA_INVERSE	1
#define VTA_UNDERLINE	2
#define VTA_ITALIC	4
#define VTA_FLASH	8
#define VTA_BOLD	16
#define VTA_OVERSTRIKE	32
  /* 64 is set to ensure a valid normal character */
#define VTA_NOCURSOR	128
  signed char cursorx;
  signed char cursory;
  signed char ncursory;
  uint8_t cursorhide;
  uint8_t ink;
  uint8_t paper;
};

struct vt_repeat {
  uint8_t first;
  uint8_t continual;
};

/* Core functions */
void vtoutput(unsigned char *p, unsigned int len);
void vtinit(void);
/* Mode switcher functions */
void vt_save(struct vt_switch *vt);
void vt_load(struct vt_switch *vt);
/* Helpers for things like graphics */
void vt_cursor_on(void);
void vt_cursor_off(void);
/* Platform functions */
void clear_lines(int8_t y, int8_t ct);
void clear_across(int8_t y, int8_t x, int16_t l);
void cursor_off(void);
void cursor_on(int8_t y, int8_t x);
void cursor_disable(void);
void scroll_up(void);
void scroll_down(void);
void plot_char(int8_t y, int8_t x, uint16_t c);
void do_beep(void);
int vt_ioctl(uint_fast8_t minor, uarg_t op, char *ptr);
int vt_inproc(uint_fast8_t minor, unsigned char c);
void vtattr_notify(void);
extern uint8_t vtattr;
extern uint8_t vtattr_cap;
extern uint8_t vtink;
extern uint8_t vtpaper;
extern struct vt_repeat keyrepeat;

#endif
