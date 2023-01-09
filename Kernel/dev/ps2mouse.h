
#ifndef PS2FASTCALL
#define PS2FASTCALL
#endif

/* Provided by transport layer */
extern uint16_t ps2mouse_get(void);
extern int ps2mouse_put(uint8_t c) PS2FASTCALL;

/* Set up a PS/2 mouse */
extern int ps2mouse_init(void);
/* Mouse data received */
extern void ps2mouse_byte(uint8_t c);

/* Platform provided */
extern void plt_ps2mouse_event(uint8_t *event);

/* Is the mouse open - so we can shortcut polling it */
extern uint_fast8_t ps2m_open;