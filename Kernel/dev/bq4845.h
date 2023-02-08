/* Caller provided: these must be able to run in IRQ context */

extern uint_fast8_t bq4845_read(uint_fast8_t port);
extern void bq4845_write(uint_fast8_t port, uint_fast8_t val);

/* Methods we provide other than the platform rtc ones */
extern int bq4845_battery_good(void);
extern void bq4845_init(void);

extern uint_fast8_t bq4845_present;

/* offsets into RTC */
#define BQ4845_SEC	0x00 /* seconds: range 00-59*/
#define BQ4845_MIN	0x02 /* minutes: range 00-59 */
#define BQ4845_HR	0x04 /* hours: 24hr range 00-23 */
#define BQ4845_DOM	0x06 /* day of month: range 01-31 */
#define BQ4845_DOW	0x08 /* day of week: 1=sunday .. 7=saturday */
#define BQ4845_MON	0x09 /* month: range 01-12 */
#define BQ4845_YR	0x0A /* year: range 00-99 */

#define BQ4845_REGB	0x0B /* control register B */
#define BQ4845_REGC	0x0C /* control register C */
#define BQ4845_REGD	0x0D /* control register D */
#define REGD_BVF	0x01
#define BQ4845_REGE	0x0E /* control register E */
#define REGE_DSE	0x01
#define REGE_2412	0x02
#define REGE_STOP	0x04
#define REGE_UTI	0x08
