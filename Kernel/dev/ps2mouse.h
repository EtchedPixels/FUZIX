/* Provided by transport layer */
extern uint16_t ps2mouse_get(void);
extern int ps2mouse_put(uint8_t c) __fastcall;

/* Set up a PS/2 mouse */
extern int ps2mouse_init(void);
/* Poll the mouse for data */
extern void ps2mouse_poll(void);

/* Platform provided */
extern void platform_ps2mouse_event(uint8_t *event);

