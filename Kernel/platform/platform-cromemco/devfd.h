extern int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_close(uint8_t minor);
extern int fd_open(uint8_t minor, uint16_t flag);

extern uint8_t fd_reset(void);
extern uint8_t fd_operation(void);
extern uint8_t fd_seek(void);
