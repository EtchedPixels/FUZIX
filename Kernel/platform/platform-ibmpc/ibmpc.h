typedef void (*irqvector_t)(int8_t);

extern uint8_t inb(uint16_t);
extern void outb(uint8_t, uint16_t);

extern uint8_t pc_at;

static inline uint8_t inb_p(uint16_t port)
{
	uint8_t value = inb(port);
	inb(0x80);
}

static inline void outb_p(uint8_t value, uint16_t port)
{
	outb(value, port);
	inb(0x80);
}

