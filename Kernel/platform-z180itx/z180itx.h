extern uint8_t ps2kbd_present;
extern uint8_t ps2mouse_present;

/* For now */
extern void spi_select_port(uint8_t n);
#define spi_select_none()	spi_select_port(7)

#define spi_send	sd_spi_transmit_byte
#define spi_recv	sd_spi_receive_byte
