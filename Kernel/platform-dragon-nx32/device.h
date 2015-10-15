#ifndef __DEVICE_DOT_H__
#define __DEVICE_DOT_H__

extern uint8_t system_id;
extern uint8_t mpi_present(void);
extern uint8_t mpi_set_slot(uint8_t slot);
extern uint16_t cart_hash(void);
extern uint16_t cart_analyze_hdb(void);

extern uint16_t hdb_port;
extern uint8_t hdb_timeout;
extern uint8_t hdb_id;

extern uint16_t cartaddr[];
extern uint8_t carttype[];
extern uint8_t cartslots;
extern uint8_t bootslot;
extern uint8_t membanks;

extern uint8_t spi_setup(void);

#endif /* __DEVICE_DOT_H__ */
