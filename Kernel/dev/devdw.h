#ifndef __DEVDW_DOT_H__
#define __DEVDW_DOT_H__

/* public interface */
int dw_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int dw_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int dw_open(uint8_t minor, uint16_t flag);
int dw_ioctl(uint8_t minor, uarg_t request, char *data);

/* low level interface */
uint8_t dw_reset(void);
uint8_t dw_operation(uint8_t *cmd);
uint16_t dw_transaction( unsigned char *send, uint16_t scnt,
			 unsigned char *recv, uint16_t rcnt, uint8_t rawf );

#endif /* __DEVDW_DOT_H__ */

