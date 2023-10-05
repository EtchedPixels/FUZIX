#ifndef __ETH_H
#define __ETH_H

#include <stdint.h>
#include "types.h"

int eth_open(uint_fast8_t minor, uint16_t flag);
int eth_close(uint_fast8_t minor);
int eth_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int eth_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int eth_ioctl(uint_fast8_t minor, uarg_t request, char *data);

void ethdev_init(void);

#endif /* __ETH_H */
