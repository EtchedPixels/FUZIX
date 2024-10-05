#ifndef __BETADISK_H__
#define __BETADISK_H__

int betadisk_open(uint8_t minor, uint16_t flag);
int betadisk_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int betadisk_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

/* Hacks in assembler */
void betadisk_seek_internal(uint16_t track);
void betadisk_read_internal(uint16_t sector, uint8_t* buf);

#endif
