
extern uint8_t biosdata_read(uint16_t offset);

extern uint16_t bioshd_reset(uint16_t drive);
extern uint32_t bioshd_param(uint16_t drive);
extern uint32_t bioshd_read(uint16_t cylsec, uint8_t dev, uint8_t head,
    uint16_t page, uint16_t dptr, uint16_t len);
extern uint32_t bioshd_write(uint16_t cylsec, uint8_t dev, uint8_t head,
    uint16_t page, uint16_t dptr, uint16_t len);

extern uint16_t kernel_ds;
extern uint16_t equipment_word;
