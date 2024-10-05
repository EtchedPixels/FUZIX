

extern void vd_init(void);
extern void vd_read(uint8_t *dptr);
extern void vd_write(uint8_t *dptr);

#define VD_DRIVE_NR_MASK	0x0F
#define VD_DRIVE_COUNT		16


#define OP_SET_LED		0x00
#define OP_SERIAL_TX		0x01
#define OP_GPIOA_WRITE		0x03
#define OP_GPIOB_WRITE		0x04
#define OP_IODIRA_WRITE		0x05
#define OP_IODIRB_WRITE		0x06
#define OP_GPPUA_WRITE		0x07
#define OP_GPPUB_WRITE		0x08
#define OP_SET_DISK		0x09
#define OP_SET_TRACK		0x0A
#define OP_SET_SECTOR		0x0B
#define OP_WRITE_SECTOR		0x0C
#define OP_SET_BANK		0x0D

#define OP_SERIAL_RX		0x80
#define OP_GPIOA_READ		0x81
#define OP_GPIOB_READ		0x82
#define OP_GET_SYSFLAGS		0x83
#define OP_GET_RTC		0x84
#define OP_GET_ERROR		0x85
#define OP_READ_SECTOR		0x86
#define OP_MOUNT_SD		0x87
#define OP_GET_TXBUF		0x88
#define OP_GET_IRQ		0x89

#define IRQ_CONSOLE		1
#define IRQ_TIMER		2

#define opread			0
#define opwrite			0
#define opcode			1
#define ttyport			1

#define OP_PORT			1
#define OP_RD_PORT		0
#define OP_WR_PORT		0

extern uint8_t has_rtc;
