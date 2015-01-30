extern int sys_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int sys_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int sys_ioctl(uint8_t minor, uarg_t request, char *data);
