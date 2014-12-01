#ifndef __DEVSD_DOT_H__
#define __DEVSD_DOT_H__

#define UZI_PARTITION_TYPE 0x5A    /* ASCII "Z", Wikipedia suggests this partition type is not widely used */
#define UZI_BLOCKDEV_SIZE_LOG2_SECTORS 16 /* Each device is 2^16 sectors ie 2^16 * 2^9 = 2^25 = 32MB */

int sd_init(void);

int sd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int sd_open(uint8_t minor, uint16_t flag);
int sd_close(uint8_t minor);

#endif /* __DEVSD_DOT_H__ */
