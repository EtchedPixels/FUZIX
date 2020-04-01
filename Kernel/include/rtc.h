#ifndef _KERNEL_RTC_H
#define _KERNEL_RTC_H

struct cmos_rtc
{
	uint8_t type;
	union {
		uint8_t bytes[8];
		time_t clock;
	} data;
};

#define CMOS_RTC_BCD	0
/* BCD encodded YYYY MM DD HH MM SS */
#define CMOS_RTC_DEC	1
/* Byte encoded the same. Year is 2 bytes little endian */
#define CMOS_RTC_TIME	2
/* Unix time format */

struct cmos_nvram
{
	uint16_t offset;
	uint8_t val;
};

#define RTCIO_NVGET	0x0501
#define RTCIO_NVSET	0x4502
#define RTCIO_NVSIZE	0x0503

#endif
