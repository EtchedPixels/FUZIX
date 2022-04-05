
/* Caller provided: these must be IRQ safe */
extern uint_fast8_t ds12885_read(uint_fast8_t port);
extern void ds12885_write(uint_fast8_t port, uint_fast8_t val);

/* Methods we provide other than the platform rtc ones */
extern int ds12885_battery_good(void);
extern void ds12885_init(void);

extern uint_fast8_t ds12885_interrupt(void);
#define DS12885_IRQF	0x80
#define DS12885_PF	0x40
#define DS12885_AF	0x20
#define DS12885_UF	0x10
/* 64Hz interval timer option */
extern void ds12885_set_interval(void);
extern void ds12885_disable_interval(void);

