extern int ide_open(uint8_t minor, uint16_t flag);
extern int ide_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int ide_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int ide_ioctl(uint8_t minor, uarg_t request, char *data);
extern void ide_probe(void);

extern uint8_t ide_present;
