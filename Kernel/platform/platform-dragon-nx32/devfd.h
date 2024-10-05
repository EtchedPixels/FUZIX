#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_open(uint_fast8_t minor, uint16_t flag);
void fd_timer_tick(void);

/* low level interface */
uint8_t fd_reset(uint8_t *drive);
uint8_t fd_operation(uint8_t *cmd, uint8_t *drive);
uint8_t fd_motor_on(uint8_t drive);
uint8_t fd_motor_off(void);

#endif /* __DEVFD_DOT_H__ */

