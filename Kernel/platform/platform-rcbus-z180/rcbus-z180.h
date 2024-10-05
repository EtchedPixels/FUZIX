extern void gpio_set(uint8_t, uint8_t);

extern uint8_t detect_1mb(void);

/* For now */
extern void spi_select_port(uint8_t n);
#define spi_select_none()	spi_select_port(0)

#define spi_send	sd_spi_tx_byte
#define spi_recv	sd_spi_rx_byte

extern uint16_t syscpu;
extern uint16_t syskhz;
extern uint8_t systype;
extern uint8_t romver;
extern uint8_t turbo;
