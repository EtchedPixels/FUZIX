#ifndef TINYIDE_H
#define TINYIDE_H

int ide_xfer(uint_fast8_t unit, bool is_read, uint32_t lba, uint8_t * dptr);
extern uint8_t ide_master;
extern uint8_t ide_slave;

#endif
