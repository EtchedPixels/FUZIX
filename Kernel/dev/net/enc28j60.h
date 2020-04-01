#ifndef _ENC28J60_H
#define _ENC28J60_H

extern int enc_init(void);
extern int enc_write_packet(uint8_t *packet, uint16_t len);
extern int enc_link_up(void);
extern int enc_read_begin(void);
extern void enc_read_complete(uint8_t *buf, uint16_t len);

/* Platform provided methods */
extern void enc_nap_1ms(void);
extern void enc_reset(void);
extern void enc_select(void);
extern void enc_deselect(void);
extern void enc_read_block(uint8_t *buf, unsigned int len);
extern void enc_read_block_user(uint8_t *buf, unsigned int len);

#include <platform_enc28j60.h>

#define READBUF_CMD	0x3A

#endif

