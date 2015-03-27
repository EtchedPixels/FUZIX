#ifndef _DEV_SIO_H
#define _DEV_SIO_H

extern int select_sio(void);
extern void deselect_sio(int old);

extern void sio_write(uint8_t *buf, int len);
extern int sio_read(uint8_t *buf, int len);

/* These don't truely belong here but it will do for now */
extern void read_from_bank(uint16_t bank, uint16_t dptr, uint8_t *buf, uint16_t len);

#endif