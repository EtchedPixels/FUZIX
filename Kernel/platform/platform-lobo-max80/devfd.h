#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_open(uint_fast8_t minor, uint16_t flag);

/* low level interface */
uint16_t fd_restore(uint8_t *driveptr);
uint16_t fd_operation(uint8_t *driveptr);
uint16_t fd_motor_on(uint16_t drivesel);
uint16_t fd_motor_off(uint16_t driveptr);

#endif /* __DEVFD_DOT_H__ */
