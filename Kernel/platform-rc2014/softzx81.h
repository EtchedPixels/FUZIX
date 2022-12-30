#ifndef _SOFTZX81_H
#define _SOFTZX81_H

extern void soft_zx81(ptptr p) __fastcall;
extern void set_kscan(ptptr p) __fastcall;
extern int softzx81_on(uint_fast8_t minor);
extern int softzx81_off(uint_fast8_t minor);
extern void softzx81_int(void);

extern uint8_t soft81_on;

#endif