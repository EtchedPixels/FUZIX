#ifndef __DEVSIL_DOT_H__
#define __DEVSIL_DOT_H__

/* public interface */
int sil_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int sil_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int sil_open(uint_fast8_t minor, uint16_t flag);

/* asm banking helper */
void sil_memcpy(uint8_t isread, uint8_t map, uint16_t dptr, uint16_t block, uint16_t dev);

#endif /* __DEVRD_DOT_H__ */

