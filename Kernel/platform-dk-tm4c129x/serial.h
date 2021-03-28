#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>
#include <stdbool.h>

void serial_late_init(void);
bool serial_rxavailable(void);
bool serial_txnotfull(void);
void serial_putc(uint32_t c);

#endif /* __SERIAL_H */
