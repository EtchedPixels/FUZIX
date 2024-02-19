#define io_bank_0		(*(volatile uint8_t *)0xFE00)
#define io_bank_1		(*(volatile uint8_t *)0xFE01)
#define io_bank_2		(*(volatile uint8_t *)0xFE02)
#define io_bank_3		(*(volatile uint8_t *)0xFE03)

#define io_serial_0_flags	(*(volatile uint8_t *)0xFE10)
#define io_serial_0_in		(*(volatile uint8_t *)0xFE11)
#define io_serial_0_out		(*(volatile uint8_t *)0xFE12)
#define io_serial_1_flags	(*(volatile uint8_t *)0xFE18)
#define io_serial_1_in		(*(volatile uint8_t *)0xFE19)
#define io_serial_1_out		(*(volatile uint8_t *)0xFE1A)
#define SERIAL_FLAGS_OUT_FULL	128
#define SERIAL_FLAGS_IN_AVAIL	64

#define io_disk_cmd		(*(volatile uint8_t *)0xFE60)
#define io_disk_prm0		(*(volatile uint8_t *)0xFE61)
#define io_disk_prm1		(*(volatile uint8_t *)0xFE62)
#define io_disk_prm2		(*(volatile uint8_t *)0xFE63)
#define io_disk_prm3		(*(volatile uint8_t *)0xFE64)
#define io_disk_data		(*(volatile uint8_t *)0xFE65)
#define io_disk_status		(*(volatile uint8_t *)0xFE66)
#define DISK_CMD_SELECT		0
#define DISK_CMD_SEEK		1
#define DISK_STATUS_OK		0
#define DISK_STATUS_NOK		1

#define io_timer_target		(*(volatile uint8_t *)0xFE80)
#define io_timer_count		(*(volatile uint8_t *)0xFE81)
#define io_timer_reset		(*(volatile uint8_t *)0xFE82)
#define io_timer_trig		(*(volatile uint8_t *)0xFE83)
#define io_timer_pause		(*(volatile uint8_t *)0xFE84)
#define io_timer_cont		(*(volatile uint8_t *)0xFE85)
