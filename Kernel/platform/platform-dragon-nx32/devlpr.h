#ifndef __DEVLPR_DOT_H__
#define __DEVLPR_DOT_H__

extern int lpr_open(uint_fast8_t minor, uint16_t flag);
extern int lpr_close(uint_fast8_t minor);
extern int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);

extern void dw_lpr(uint8_t c);
extern void dw_lpr_close(void);

#endif
