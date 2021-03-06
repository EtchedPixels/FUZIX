#ifndef USBCONSOLE_H
#define USBCONSOLE_H

extern void core1_init(void);

extern bool usbconsole_is_readable(void);
extern bool usbconsole_is_writable(void);
extern uint8_t usbconsole_getc_blocking(void);
extern void usbconsole_putc_blocking(uint8_t b);

#endif

