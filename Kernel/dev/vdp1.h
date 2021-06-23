#ifndef _DEV_VDP1_H
#define _DEV_VDP1_H

extern uint16 vdpport;

extern uint8_t vdp_text32[8];
extern uint8_t vdp_text40[8];

extern void vdp_init(void);
extern void vdp_setup(uint8_t *settings) __fastcall;
extern uint8_t vdp_type(void);
extern void vdp_load_font(void);
extern void vdp_restore_font(void);

#endif
