#ifndef __DEVLPR_DOT_H__
#define __DEVLPR_DOT_H__

extern int lpr_open(uint8_t minor, uint16_t flag);
extern int lpr_close(uint8_t minor);
extern int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

extern void dw_lpr(uint8_t c);
extern void dw_lpr_close(void);

#endif
