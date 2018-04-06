extern int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_close(uint8_t minor);
extern int fd_open(uint8_t minor, uint16_t flag);

extern void fd_reset(uint8_t *ptr);
extern int fd_operation(uint8_t *ptr);
