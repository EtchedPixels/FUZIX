

extern int sdc_open(uint8_t minor, uint16_t flag);
extern int sdc_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int sdc_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int sdc_ioctl(uint8_t minor, uarg_t request, char *data);

extern uint8_t sdc_present;
