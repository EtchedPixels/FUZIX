#ifndef FTL_H
#define FTL_H

extern int ftl_init(void);
extern int ftl_read(uint32_t logical, int sector, uint8_t* buffer);
extern int ftl_write(uint32_t logical, int sector, const uint8_t* buffer);

extern int raw_flash_read(uint32_t physical, int sector, uint8_t* buffer);
extern int raw_flash_write(uint32_t physical, int sector, const uint8_t* buffer);
extern int raw_flash_erase(uint32_t physical);

#endif

