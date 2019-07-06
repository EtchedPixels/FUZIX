#ifndef _DEV_PPIDE_H
#define _DEV_PPIDE_H
extern void ppide_init(void);

extern uint8_t ppide_readb(uint8_t reg);
extern void ppide_writeb(uint8_t reg, uint8_t val);
extern void ppide_read_data(void);
extern void ppide_write_data(void);

#endif
