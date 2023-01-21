int fd_open(uint8_t minor, uint16_t flag);
int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_ioctl(uint8_t minor, uarg_t request, char *buffer);

#define fdbios_op	(*((uint8_t *)0x2048))
#define fdbios_drive	(*((uint8_t *)0x2049))
#define fdbios_track 	(*((uint16_t *)0x204A))
#define fdbios_sector	(*((uint8_t *)0x204C))
#define fdbios_addr	(*((uint8_t *)0x204F))
#define fdbios_floppy	(*((uint8_t *)0x2080))

#define FDOP_READ	0x02
#define FDOP_WRITE	0x08
