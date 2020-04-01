extern int ps2kbd_init(void);
extern void ps2kbd_byte(uint_fast8_t byte);

extern uint16_t ps2kbd_get(void);
extern uint16_t ps2mouse_get(void);

extern int ps2kbd_put(uint8_t c) __fastcall;
extern int ps2mouse_put(uint8_t c) __fastcall;

extern void ps2kbd_poll(void);
extern void ps2kbd_conswitch(uint8_t con);

extern void ps2kbd_beep(void);

#define PS2_NOCHAR		-1
#define PS2_PARITY		-2
#define PS2_TIMEOUT		-3
#define PS2_BADSTART		-4
