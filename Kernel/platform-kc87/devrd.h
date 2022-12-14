#ifndef __DEVRD_DOT_H__
#define __DEVRD_DOT_H__

/* public interface */
int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_open(uint_fast8_t minor, uint16_t flag);
extern void rd_probe(void);

#endif /* __DEVRD_DOT_H__ */

