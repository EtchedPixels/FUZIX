#ifndef __DS1302_DOT_H__
#define __DS1302_DOT_H__

/* public interface */
uint_fast8_t ds1302_init(void);
uint_fast8_t platform_rtc_secs(void);
void ds1302_read_clock(uint8_t *buffer, uint_fast8_t length);
int platform_rtc_read(void);
int platform_rtc_write(void);
int platform_rtc_ioctl(uarg_t request , char *data);

extern uint8_t rtc_defer;		/* Don't poll the RTC right now */
extern uint8_t rtc_shadow;		/* Shadow for other bits in port */
extern uint16_t rtc_port;		/* I/O address to use */
/* Optional helpers */
#ifdef DS1302_SETUP
extern void platform_ds1302_setup(void);
extern void platform_ds1302_restore(void);
#else
#define platform_ds1302_setup()		do {} while(0)
#define platform_ds1302_restore()	do {} while(0)
#endif
extern uint8_t ds1302_present;

#ifdef _DS1302_PRIVATE

void ds1302_write_register(uint_fast8_t reg, uint_fast8_t val);

/* consult the DS1302 datasheet for data format;
   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf table 3 */
void ds1302_send_byte(uint_fast8_t byte);

uint_fast8_t uint8_from_bcd(uint_fast8_t value);

/* platform code must provide these functions */
void ds1302_set_pin_clk(bool state) __fastcall;
void ds1302_set_pin_ce(bool state) __fastcall;
void ds1302_set_pin_data(bool state) __fastcall;
bool ds1302_get_pin_data(void);
void ds1302_set_pin_data_driven(bool state) __fastcall; /* 0=tristate for input, 1=driven for output */
#endif

#endif
