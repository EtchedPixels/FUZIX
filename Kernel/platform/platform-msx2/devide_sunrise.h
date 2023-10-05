extern uint8_t *do_ide_init_drive(uint8_t drive) __z88dk_fastcall;
extern void do_ide_begin_reset(void) __z88dk_fastcall;
extern void do_ide_end_reset(void) __z88dk_fastcall;
extern uint8_t do_ide_flush_cache(uint8_t drive) __z88dk_fastcall;
extern unsigned do_ide_xfer(uint16_t drive) __z88dk_fastcall;

extern void sunrise_probe(void);

#define IDE_DRIVE_COUNT		2

#define FLAG_WRITE_CACHE	0x80
#define FLAG_CACHE_DIRTY	0x40

#define IDE_DRIVE_NR_MASK	0x03
