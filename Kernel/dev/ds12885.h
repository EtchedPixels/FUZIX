/* Caller provided: these must be able to run in IRQ context */

extern uint_fast8_t ds12885_read(uint_fast8_t port);
extern void ds12885_write(uint_fast8_t port, uint_fast8_t val);

/* Methods we provide other than the platform rtc ones */
extern int ds12885_battery_good(void);
extern void ds12885_init(void);

extern uint_fast8_t ds12885_interrupt(void);
/* 64Hz interval timer option */
extern void ds12885_set_interval(void);
extern void ds12885_disable_interval(void);

extern uint_fast8_t ds12885_present;

/* offsets into RTC */
#define DS12885_SEC	0x00 /* seconds: range 00-59*/
#define DS12885_SEC_AL	0x01 /* seconds alarm */
#define DS12885_MIN	0x02 /* minutes: range 00-59 */
#define DS12885_MIN_AL	0x03 /* minutes alarm */
#define DS12885_HR	0x04 /* hours: 24hr range 00-23 */
#define DS12885_HR_AL	0x05 /* hours alarm */
#define DS12885_DOW	0x06 /* day of week: 1=sunday .. 7=saturday */
#define DS12885_DOM	0x07 /* day of month: range 01-31 */
#define DS12885_MON	0x08 /* month: range 01-12 */
#define DS12885_YR	0x09 /* year: range 00-99 */
#define DS12885_CEN	0x32 /* century: range 00-99 */

#define DS12885_REGA	0x0A /* control register A */
#define RS0		0x01 /* RS3-RS0: interrupt rate selector */
#define RS1		0x02
#define RS2		0x04
#define RS3		0x08
#define DV0		0x10 /* DV2,DV1,DV0: oscillator, RTC control */
#define	DV1		0x20 /*   0,  1,  0: oscillator on, RTC on   */
#define DV2		0x40 /*   1,  1,  x: oscillator on, RTC hold */
#define UIP		0x80 /* update-in-progress (UIP) */

#define DS12885_REGB	0x0B /* control register B */
#define DSE		0x01 /* daylight savings enable */
#define HR24		0x02 /* 1: 24-hour mode; 0: 12-hour mode */
#define DM		0x04 /* data mode: 1=binary, 0=BCD */
#define SQWE		0x08 /* square wave enable */
#define UIE		0x10 /* update-ended interrupt enable */
#define	AIE		0x20 /* alarm interrupt enable */
#define PIE		0x40 /* periodic interrupt enable */
#define SET		0x80 /* set */

#define DS12885_REGC	0x0C /* control register C */
#define UF		0x10 /* update-ended interrupt flag */
#define	AF		0x20 /* alarm interrupt flag */
#define PF		0x40 /* periodic interrupt flag */
#define IRQF		0x80 /* interrupt request flag */

#define DS12885_REGD	0x0D /* control register D */
#define VRT		0x80 /* valid ram and time */
