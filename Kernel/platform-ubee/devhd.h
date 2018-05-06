#ifndef __DEVHD_DOT_H__
#define __DEVHD_DOT_H__

/* public interface */
static uint8_t hd_transfer_sector(void);
void hd_init(void);

/* helpers in common memory for the block transfers */
static void hd_xfer_in(void);
static void hd_xfer_out(void);

#endif /* __DEVHD_DOT_H__ */
