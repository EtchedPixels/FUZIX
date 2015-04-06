#ifndef __DEVDW_DOT_H__
#define __DEVDW_DOT_H__

/* public interface */
int dw_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int dw_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int dw_open(uint8_t minor, uint16_t flag);

/* low level interface */
uint8_t dw_reset(uint8_t *drive);
uint8_t dw_operation(uint8_t *cmd, uint8_t *drive);

#endif /* __DEVDW_DOT_H__ */

