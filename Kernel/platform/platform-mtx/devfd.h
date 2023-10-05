#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_open(uint_fast8_t minor, uint16_t flag);
void fd_motor_timer(void);

/* low level interface */
uint16_t fd_reset(uint8_t *driveptr);
uint16_t fd_operation(uint8_t *driveptr);
uint16_t fd_motor_on(uint16_t drivesel);
uint16_t fd_motor_off(void);

/* In common */
extern uint8_t fd_cmd[8];
extern uint16_t fd_data;

#endif /* __DEVFD_DOT_H__ */
