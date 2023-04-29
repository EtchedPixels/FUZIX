/*
 *	PCF8584
 */

extern void pcf8584_bus_reset(void);
extern int pcf8584_tx_msg(uint8_t to, const uint8_t *p, unsigned len);
extern int pcf8584_rx_msg(uint8_t from, uint8_t *p, unsigned len);
extern void pcf8584_init(uint8_t clock);

#define PCF_CLK_3MHZ	0x00
#define PCF_CLK_4MHZ	0x10
#define PCF_CLK_6MHZ	0x14
#define PCF_CLK_8MHZ	0x18
#define PCF_CLK_12MHZ	0x1C
