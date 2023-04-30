#ifndef __TTYDW_DOT_H__
#define __TTYDW_DOT_H__

/* Stores drivewire server type on boot */
extern uint8_t dwtype;
#define DWTYPE_DW3          0xfc
#define DWTYPE_UNKNOWN      0xfd
#define DWTYPE_NOTFOUND     0xfe
#define DWTYPE_PYDRIVEWIRE  0xff
#define DWTYPE_DW4          0x04
#define DWTYPE_LWWIRE       0x80


void dw_putc(uint8_t minor, unsigned char c);
void dw_vopen(uint8_t minor);
void dw_vclose(uint8_t minor);
int dw_carrier(uint8_t minor);
void dw_vpoll(void);
int dw_init(void);

#endif
