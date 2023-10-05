#ifndef __N8_H

int n8tty_ioctl(uint8_t minor, uarg_t arg, char *ptr);
int n8tty_close(uint_fast8_t minor);

__sfr __at 0x98 tms9918a_data;
__sfr __at 0x99 tms9918a_ctrl;

/* For console */
extern uint8_t vt_twidth, vt_tright;
extern uint8_t tms9918a_type;

#endif
