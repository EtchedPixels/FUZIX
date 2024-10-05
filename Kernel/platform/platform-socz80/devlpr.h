#ifndef __DEVLPR_DOT_H__
#define __DEVLPR_DOT_H__

int lpr_open(uint8_t minor, uint16_t flag);
int lpr_close(uint8_t minor);
int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

#endif
