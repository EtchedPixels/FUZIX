extern uint8_t *do_ide_init_drive(uint8_t drive) __z88dk_fastcall;
extern void do_ide_begin_reset(void) __z88dk_fastcall;
extern void do_ide_end_reset(void) __z88dk_fastcall;
extern uint8_t do_ide_flush_cache(uint8_t drive) __z88dk_fastcall;
extern unsigned do_ide_xfer(uint16_t drive) __z88dk_fastcall;

extern void sunrise_probe(void);
