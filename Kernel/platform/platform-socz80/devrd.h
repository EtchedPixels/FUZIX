#ifndef __DEVRD_DOT_H__
#define __DEVRD_DOT_H__

/* public interface */
int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_init(void);
int rd_open(uint8_t minor, uint16_t flag);

#endif /* __DEVRD_DOT_H__ */
