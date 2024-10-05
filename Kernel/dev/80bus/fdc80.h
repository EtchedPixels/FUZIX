extern uint8_t fdc80_readsec(uint16_t info);
extern uint8_t fdc80_writesec(uint16_t info);
extern uint8_t fdc80_cmd(uint16_t info);
extern uint8_t fdc80_seek(uint16_t info);
extern uint8_t fdc80_reset(void);

extern uint8_t fdc80_track;
extern uint16_t fdc80_dptr;
extern uint8_t fdc80_iopage;

extern int fdc80_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int fdc80_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int fdc80_open(uint_fast8_t minor, uint16_t flags);
extern int fdc80_ioctl(uint_fast8_t minor, uarg_t request, char *buffer);
extern unsigned fdc80_probe(void);

#define FDC_NONE	0
#define FDC_NASCOM	1
#define FDC_GM809	2
#define FDC_GM849	3

/* Platform provides */
extern void plt_disable_nmi(void);
extern void plt_enable_nmi(void);
