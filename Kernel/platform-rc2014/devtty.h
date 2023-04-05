
#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_pollirq(void);
int rctty_open(uint_fast8_t minor, uint16_t flag);
int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr);
int rctty_close(uint_fast8_t minor);

extern uint8_t shadowcon;
extern uint8_t nuart;

extern void tms9918a_reset(void);
extern void tms9918a_reload(void);
extern void tms9918a_set_char(uint_fast8_t c, uint8_t *d);
extern void tms9918a_udgload(void);
void tms9918a_attributes(void);

/* Until we move this lot into asm for neatness */
extern uint16_t scrolld_base, scrolld_mov, scrolld_s1, scrollu_w, scrollu_mov;
extern uint16_t vdpport;
extern uint16_t vdp_rop(struct vdp_rw *rw) __fastcall;
extern uint16_t vdp_wop(struct vdp_rw *rw) __fastcall;

/* For console */
extern uint8_t vt_twidth, vt_tright;
extern uint8_t vt_theight, vt_tbottom;

extern uint8_t inputtty;
#endif
