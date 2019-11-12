extern int ps2kbd_init(void);
extern void ps2kbd_byte(uint_fast8_t byte);

extern uint16_t ps2kbd_get(void);
extern uint16_t ps2mouse_get(void);

extern int ps2kbd_put(uint8_t c) __fastcall;
extern int ps2mouse_put(uint8_t c) __fastcall;
