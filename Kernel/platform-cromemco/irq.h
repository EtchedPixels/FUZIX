extern int request_irq(uint8_t vec, void (*func)(uint8_t));
extern void free_irq(uint8_t vec);

/* These have to live in common space */
extern uint8_t irqvec[];
extern void spurious(uint8_t vec);
extern void set_irq(uint16_t vec, void (*func)(uint8_t));

extern void tuart0_timer4(uint8_t unused);
