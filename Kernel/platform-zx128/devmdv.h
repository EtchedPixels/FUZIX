#ifndef __DEVMDV_DOT_H__
#define __DEVMDV_DOT_H__

/* public interface */
int mdv_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int mdv_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int mdv_open(uint8_t minor, uint16_t flag);
int mdv_close(uint8_t minor);

/* low level interface */
int mdv_motor_on(uint8_t drive);
int mdv_motor_off(void);
int mdv_bread(void);
int mdv_bwrite(void);

/* tiner */
void mdv_timer(void);

#endif /* __DEVMDV_DOT_H__ */

