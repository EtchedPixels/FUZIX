#ifndef GLOBALS_H
#define GLOBALS_H

extern void flash_dev_init(void);
extern int devflash_ioctl(uint_fast8_t minor, uarg_t request, char* data);

extern void fuzix_main(void);

#endif

