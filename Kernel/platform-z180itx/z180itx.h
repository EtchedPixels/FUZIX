extern void gpio_set(uint8_t, uint8_t);
extern void spi_cs_set_sd(uint_fast8_t);
extern void spi_cs_clear(void);
extern void spi_cs_set(uint_fast8_t);

extern uint8_t detect_1mb(void);

extern uint8_t board_type;
#define GENERIC		0	/* Generic Z180 board */
#define SC126		1	/* SC126 - has 0x0C/0x0D, DS1302 */
#define ITX		2	/* Z180 MiniITX - PPI and 1Mb, DS1307+ */

/* For now */
extern void spi_select_port(uint8_t n);
#define spi_select_none()	spi_select_port(0)

#define spi_send	sd_spi_transmit_byte
#define spi_recv	sd_spi_receive_byte
