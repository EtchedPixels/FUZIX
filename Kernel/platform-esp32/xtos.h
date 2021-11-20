#ifndef XTOS_H
#define XTOS_H

extern int ets_printf(const char* format, ...);
extern void ets_write_char_uart(char c);

/* A 32-bit aligned copy (which can work on instruction data). */
extern void* xthal_memcpy(void *dst, const void *src, size_t len);

/* Sets the address the app core jumps to when reset. */
extern void ets_set_appcpu_boot_addr(void(*)(void));

extern void bzero(void* ptr, size_t size);

#endif

