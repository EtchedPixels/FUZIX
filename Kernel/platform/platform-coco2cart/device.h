#ifndef __DEVICE_DOT_H__
#define __DEVICE_DOT_H__

extern uint8_t system_id;
extern uint8_t mpi_present(void);
extern uint8_t mpi_set_slot(uint8_t slot);
extern uint16_t cart_hash(void);

extern uint8_t spi_setup(void);

#endif /* __DEVICE_DOT_H__ */
