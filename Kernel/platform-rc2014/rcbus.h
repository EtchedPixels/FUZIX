#ifndef __RCBUS_DOT_H__
#define __RCBUS_DOT_H__

/* Standard RC2014 */
#define SIO0_BASE 0x80
__sfr __at (SIO0_BASE + 0) SIOA_C;
__sfr __at (SIO0_BASE + 1) SIOA_D;
__sfr __at (SIO0_BASE + 2) SIOB_C;
__sfr __at (SIO0_BASE + 3) SIOB_D;

#define SIO1_BASE 0x84
__sfr __at (SIO1_BASE + 0) SIOC_C;
__sfr __at (SIO1_BASE + 1) SIOC_D;
__sfr __at (SIO1_BASE + 2) SIOD_C;
__sfr __at (SIO1_BASE + 3) SIOD_D;

/* The original RC2014 ACIA is partially decoded but notionally at 0x80.
   ROMWBW 2 however probes the 0xA0 alias. ROMWBW 3 uses the 0x80 address 
   and the later narrower decoding boards are set to 0x80 for ROMWBW
   use (eg 'The Missing Module'). */

#define ACIA_BASE 0x80
__sfr __at (ACIA_BASE + 0) ACIA_C;
__sfr __at (ACIA_BASE + 1) ACIA_D;

__sfr __at 0x98 tms9918a_data;
__sfr __at 0x99 tms9918a_ctrl;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;
extern uint16_t code1_end(void);
extern void set_console(void);

extern uint8_t acia_present;
extern uint8_t sio_present;
extern uint8_t sio1_present;
extern uint8_t quart_present;
extern uint8_t z180_present;
extern uint8_t tms9918a_present;
extern uint8_t ef9345_present;
extern uint8_t dma_present;
extern uint8_t zxkey_present;
extern uint8_t copro_present;
extern uint8_t ps2kbd_present;
extern uint8_t ps2mouse_present;
extern uint8_t sc26c92_present;
extern uint8_t u16x50_present;
extern uint8_t z512_present;
extern uint8_t eipc_present;
extern uint8_t macca_present;

extern uint8_t ctc_port;
extern uint8_t kio_port;
extern uint8_t ticksperclk;	/* CTC scaling from CTC3 counts to 10Hz */

extern uint8_t timer_source;
#define TIMER_NONE		0
#define TIMER_CTC		1
#define TIMER_TMS9918A		2
#define TIMER_QUART		3
#define TIMER_SC26C92		4
#define TIMER_Z180		5
#define TIMER_DS12885		6


extern uint16_t probe_z80dma(void);

extern void pio_setup(void);

/* From ROMWBW */
extern uint16_t syscpu;
extern uint16_t syskhz;
extern uint8_t systype;
extern uint8_t romver;

extern const char *uart_name[];

struct uart {
    uint8_t (*intr)(uint_fast8_t minor);
    uint8_t (*writeready)(uint_fast8_t minor);
    void (*putc)(uint_fast8_t minor, uint_fast8_t c);
    void (*setup)(uint_fast8_t minor);
    uint8_t (*carrier)(uint_fast8_t minor);
    uint16_t cmask;
    const char *name;
};

//extern struct uart *uart[NUM_DEV_TTY + 1];
extern uint16_t ttyport[NUM_DEV_TTY + 1];
extern uint8_t register_uart( uint16_t port, struct uart *);
extern void insert_uart(uint16_t port, struct uart *);
extern void display_uarts(void);

extern void do_conswitch(uint8_t con);

extern struct uart acia_uart;
extern struct uart sio_uart;
extern struct uart sio_uartb;
extern struct uart kio_uart;
extern struct uart kio_uartb;
extern struct uart eipc_uart;
extern struct uart eipc_uartb;
extern struct uart easy_uart;
extern struct uart easy_uartb;
extern struct uart ns16x50_uart;
extern struct uart z180_uart0;
extern struct uart z180_uart1;
extern struct uart tms_uart;
extern struct uart ef_uart;
extern struct uart macca_uart;
extern struct uart quart_uart;
extern struct uart sc26c92_uart;
extern struct uart xr88c681_uart;

extern uint8_t *init_alloc(uint16_t size);
extern uint8_t *code1_alloc(uint16_t size);

extern void do_timer_interrupt(void);

/*
 *	PS/2 stuff
 */
extern int ps2port_init(void);
extern void ps2_int(void);
extern int ps2_type;
#define PS2_NONE 	0
#define PS2_BITBANG	1
#define PS2_DIRECT	2

#endif
