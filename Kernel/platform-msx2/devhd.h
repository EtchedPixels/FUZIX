#ifndef __DEVHD_DOT_H__
#define __DEVHD_DOT_H__

/* public interface */
int hd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int hd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int hd_open(uint_fast8_t minor, uint16_t flag);

#endif /* __DEVRD_DOT_H__ */

