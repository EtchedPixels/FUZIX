extern int sys_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int sys_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int sys_ioctl(uint_fast8_t minor, uarg_t request, char *data);
extern int sys_close(uint_fast8_t minor);
