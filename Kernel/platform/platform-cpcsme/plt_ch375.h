#ifndef PLT_CH375_H
#define PLT_CH375_H

extern void nap20(void);
extern void td_io_rblock(uint8_t *ptr);/* __z88dk_fastcall; */
extern void td_io_wblock(uint8_t *ptr);/* __z88dk_fastcall; */

#define nap2() __asm__ ("nop\nnop");

#define CH375_COMM_PARALLEL 0
#define CH375_COMM_SERIAL 1
#define CH375_SERIAL_WAIT 1

#if ((defined CONFIG_ALBIREO) && (defined CONFIG_USIFAC_CH376))
#define CH375_DEVICES 3
#ifdef _CH375_PRIVATE
uint16_t ch375_dports[CH375_DEVICES] = {0xFE80, 0xFE40, 0xFBD0};
uint16_t ch375_sports[CH375_DEVICES] = {0xFE81, 0xFE41, 0XFBD1};
uint8_t ch375_comm_type[CH375_DEVICES] = {CH375_COMM_PARALLEL, CH375_COMM_PARALLEL, CH375_COMM_SERIAL};
#endif
#endif

#if ((defined CONFIG_ALBIREO) && !(defined CONFIG_USIFAC_CH376))
#define CH375_DEVICES 2
#ifdef _CH375_PRIVATE
uint16_t ch375_dports[CH375_DEVICES] = {0xFE80, 0xFE40};
uint16_t ch375_sports[CH375_DEVICES] = {0xFE81, 0xFE41};
uint8_t ch375_comm_type[CH375_DEVICES] = {CH375_COMM_PARALLEL, CH375_COMM_PARALLEL};
#endif
#endif

#if (!(defined CONFIG_ALBIREO) && (defined CONFIG_USIFAC_CH376))
#define CH375_DEVICES 1
#ifdef _CH375_PRIVATE
uint16_t ch375_dports[CH375_DEVICES] = {0xFBD0};
uint16_t ch375_sports[CH375_DEVICES] = {0XFBD1};
uint8_t ch375_comm_type[CH375_DEVICES] = {CH375_COMM_SERIAL};
#endif
#endif

#if (defined CONFIG_USIFAC_SERIAL || defined CONFIG_USIFAC_CH376)
__sfr __banked __at 0xFBD0 usifdata;
__sfr __banked __at 0xFBD1 usifctrl;
__sfr __banked __at 0xFBD8 usifexists;
__sfr __banked __at 0xFBDD usifgetbaud;
__sfr __banked __at 0xFBDE usifgetusbstat;


#define USIFAC_RESET_COMMAND  0
#define USIFAC_CLEAR_RECEIVE_BUFFER_COMMAND 1
#define USIFAC_SET_9600B_COMMAND 12
#define USIFAC_SET_115200B_COMMAND 16
#define USIFAC_SET_1MBPS_COMMAND 22
#define USIFAC_SET_ONLY_SERIAL 100
#endif
#endif