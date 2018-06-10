#ifndef __DEVFD3_DOT_H__
#define __DEVFD3_DOT_H__

/* public interface */
int fd3_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd3_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd3_open(uint8_t minor, uint16_t flag);

/* low level interface */
uint16_t fd3_reset(uint8_t *driveptr);
uint16_t fd3_operation(uint8_t *driveptr);
uint16_t fd3_motor_on(uint16_t drivesel);
uint16_t fd3_motor_off(uint16_t driveptr);

extern void floppy_setup(void);

#endif /* __DEVFD3_DOT_H__ */
