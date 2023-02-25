extern int lpr_open(uint_fast8_t minor, uint16_t flag);
extern int lpr_close(uint_fast8_t minor);
extern int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int lpr_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);
