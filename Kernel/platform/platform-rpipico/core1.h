#ifndef USBCONSOLE_H
#define USBCONSOLE_H

#include <tty.h>

extern void core1_init(void);

extern int usbconsole_getc(uint8_t usb_num);
extern ttyready_t usbconsole_ready(uint8_t usb_num);
extern bool usbconsole_is_available(uint8_t usb_num);
extern void usbconsole_putc(uint8_t usb_num, uint8_t c);
extern void usbconsole_sleeping(uint8_t usb_num);

#endif
