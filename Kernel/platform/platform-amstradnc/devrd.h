#ifndef __DEVRD_DOT_H__
#define __DEVRD_DOT_H__

/* public interface */
int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_open(uint_fast8_t minor, uint16_t flag);

/* asm banking helper */
void rd_memcpy(uint8_t isread, uint8_t map, uint16_t dptr, uint16_t block,
                uint16_t len, uint8_t map2, uint16_t len2);

#endif /* __DEVRD_DOT_H__ */

