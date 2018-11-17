#ifndef __SYS_LIBRARY_H
#define __SYS_LIBRARY_H

#include <stdint.h>

struct library {
    uint16_t magic;
#define LIB_MAGIC	0xB377
    uint16_t version;
    uint16_t size;
    int16_t fd;
    uint16_t spare[12];
    uint8_t reloc[480];
};
/* 512 bytes of header block so the library direct loads nicely */

extern int libopen(struct library *__lib, const char *__path, int __version);
extern int libopenfd(struct library *__lib, int __fd, int __version);
extern int libload(struct library *__lib, uint8_t *__addr);
extern int libclose(struct library *__lib);

#endif
