/* Callbacks */
extern void netat_hangup(void);
extern void netat_event(void);
/* Platform provided */
extern void netat_nowake(void);
extern void netat_wake(void);
extern uint8_t netat_byte(void);
extern void netat_poll(void);
extern void netat_outbyte(uint8_t);
extern uint8_t netat_ready(void);
extern void netat_flush(void);
