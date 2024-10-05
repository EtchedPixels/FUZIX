#ifndef _SOCZ80_DEVETH_H
#define _SOCZ80_DEVETH_H

extern int eth_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int eth_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int eth_open(uint8_t minor, uint16_t flag);
extern int eth_close(uint8_t minor);
extern int eth_ioctl(uint8_t minor, uarg_t arg, char *data);

extern void deveth_init(void);

extern void eth_poll(void);

#endif