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
