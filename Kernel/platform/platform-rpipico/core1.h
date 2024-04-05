#ifndef USBCONSOLE_H
#define USBCONSOLE_H

extern void core1_init(void);

extern int usbconsole_read(uint8_t *buffer, int size);
extern bool usbconsole_is_writable(uint8_t minor);
extern bool usbconsole_is_available(uint8_t minor);
extern void usbconsole_putc(uint8_t minor, uint8_t b);
extern void usbconsole_setsleep(uint8_t minor, bool sleeping);
extern bool usbconsole_is_sleeping(uint8_t minor);


#endif

