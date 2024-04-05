#ifndef __DEVLPR_DOT_H__
#define __DEVLPR_DOT_H__

int lpr_open(uint_fast8_t minor, uint16_t flag);
int lpr_close(uint_fast8_t minor);
int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);

#endif
