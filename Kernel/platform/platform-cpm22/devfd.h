
extern int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int fd_open(uint8_t minor, uint16_t flag);
extern int hd_open(uint8_t minor, uint16_t flag);
extern int fd_close(uint8_t minor);
extern int hd_close(uint8_t minor);
extern void fdhd_init(void);


extern void found_swap(uint16_t dev, uint16_t blocks);


