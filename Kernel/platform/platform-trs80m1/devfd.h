#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_open(uint8_t minor, uint16_t flag);
int fd_ioctl(uint8_t minor, uarg_t request, char *buffer);

/* low level interface */
uint8_t fd_restore(uint8_t *driveptr) __z88dk_fastcall;
uint16_t fd_operation(uint8_t *driveptr) __z88dk_fastcall;
uint16_t fd_motor_on(uint16_t drivesel) __z88dk_fastcall;

/* low level interface */
uint8_t fd3_restore(uint8_t *driveptr) __z88dk_fastcall;
uint16_t fd3_operation(uint8_t *driveptr) __z88dk_fastcall;
uint16_t fd3_motor_on(uint16_t drivesel) __z88dk_fastcall;

struct fd_ops {
    uint8_t (*fd_restore)(uint8_t *driveptr) __z88dk_fastcall;
    uint16_t (*fd_op)(uint8_t *driveptr) __z88dk_fastcall;
    uint16_t (*fd_motor_on)(uint16_t drivesel) __z88dk_fastcall;
};
    
#endif /* __DEVFD_DOT_H__ */
