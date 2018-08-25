#ifndef __DS1302_DOT_H__
#define __DS1302_DOT_H__

/* public interface */
void ds1302_init(void);
uint8_t platform_rtc_secs(void);
void ds1302_read_clock(uint8_t *buffer, uint8_t length);
int platform_rtc_read(void);
int platform_rtc_write(void);

#ifdef _DS1302_PRIVATE
/* consult the DS1302 datasheet for data format;
   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf table 3 */
void ds1302_send_byte(uint8_t byte);

uint8_t uint8_from_bcd(uint8_t value);

/* platform code must provide these functions */
void ds1302_set_pin_clk(bool state);
void ds1302_set_pin_ce(bool state);
void ds1302_set_pin_data(bool state);
bool ds1302_get_pin_data(void);
void ds1302_set_pin_data_driven(bool state); /* 0=tristate for input, 1=driven for output */
#endif

#endif
