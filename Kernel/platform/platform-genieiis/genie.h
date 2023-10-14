
extern uint8_t genie_model;
#define GENIE_IIS	0
extern uint8_t card_map;

/* MMIO is only mapped in on demand */
extern void mmio_write(uint16_t addr, uint16_t val);
extern uint8_t mmio_read(uint16_t addr) __z88dk_fastcall;

