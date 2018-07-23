/* SMC CRT9128 Video Terminal Logic Controller driver */
/* 2016-10-29 Tormod Volden */

/* CRT9128_BASE defined in platform config.h */
#define CRT9128_DATA_REG CRT9128_BASE		/* read/write */
#define CRT9128_ADDRESS_REG (CRT9128_BASE+1)	/* write only */
#define CRT9128_STATUS_REG CRT9128_ADDRESS_REG	/* read only */

#define CRT9128_REG_CHIP_RESET 0x6
#define CRT9128_REG_TOSADD 0x8
#define CRT9128_REG_CURLO 0x9
#define CRT9128_REG_CURHI 0xa
#define CRT9128_REG_FILADD 0xb
#define CRT9128_REG_ATTDAT 0xc
#define CRT9128_REG_CHARACTER 0xd
#define CRT9128_REG_MODE_REGISTER 0xe

#define CRT9128_STATUS_DONE (1<<7)
#define CRT9128_TOSADD_TIM (1<<7)
#define CRT9128_CURHI_SLE (1<<7)
#define CRT9128_ATTDAT_MODE_SELECT		(1<<7)
#define CRT9128_ATTDAT_CURSOR_SUPRESS		(1<<6)
#define CRT9128_ATTDAT_CURSOR_DISPLAY		(1<<5)
#define CRT9128_ATTDAT_SCREEN			(1<<4)
#define CRT9128_ATTDAT_TAG_CHAR_SUPRESS		(1<<3)
#define CRT9128_ATTDAT_TAG_INTENSITYs		(1<<2)
#define CRT9128_ATTDAT_TAG_UNDERLINE		(1<<1)
#define CRT9128_ATTDAT_TAG_REVERSE_VIDEO	(1<<0)
#define CRT9128_MODE_AUTO_INCREMENT (1<<7)

#define crt9128_status() (*(volatile uint8_t *)CRT9128_STATUS_REG)
#define crt9128_done() (crt9128_status() & CRT9128_STATUS_DONE)

void crt9128_init(void);

void crt9128_clear_across(int8_t y, int8_t x, int16_t num);
void crt9128_clear_lines(int8_t y, int8_t num);
void crt9128_scroll_up(void);
void crt9128_scroll_down(void);
void crt9128_plot_char(int8_t y, int8_t x, uint16_t c);
void crt9128_cursor_off(void);
void crt9128_cursor_on(int8_t newy, int8_t newx);
void crt9128_vtattr_notify(void);
void crt9128_video_cmd(uint8_t *ptr);
void crt9128_video_read(uint8_t *ptr);
void crt9128_video_write(uint8_t *ptr);
