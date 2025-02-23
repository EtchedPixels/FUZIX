#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_pollirq(void);
static void keydecode(void);

#define KEY_ROWS	10
#define KEY_COLS	8
#define PENR_BORDER_SELECT 0x10
/*For now We are in mode 2 with pen 0 for paper and pen 1 for ink*/
#define PENR_PAPER_SELECT 0x00 
#define PENR_INK_SELECT 0x01
#define INKR_COLOR_SET 0x40
extern uint8_t keymap[10];
extern uint8_t keyboard[10][8];
extern uint8_t shiftkeyboard[10][8];

extern uint8_t timer_wait;

extern int cpctty_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern void tty_pollirq_usifac(void);

extern uint8_t vtborder;

__sfr __banked __at 0x7F00 gatearray;
/* see: https://www.cpcwiki.eu/index.php/Gate_Array */
#if defined CONFIG_USIFAC_SERIAL
__sfr __banked __at 0xFBD0 usifdata;
__sfr __banked __at 0xFBD1 usifctrl;
__sfr __banked __at 0xFBDD usifspr;

#define USIFAC_RESET_COMMAND  0
#define USIFAC_CLEAR_RECEIVE_BUFFER_COMMAND 1
#define USIFAC_DISABLE_BURST_MODE_COMMAND 3
#define USIFAC_DISABLE_DIRECT_MODE_COMMAND 4
#define USIFAC_SET_115200B_COMMAND 16
#endif
#endif
