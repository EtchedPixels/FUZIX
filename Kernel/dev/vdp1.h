#ifndef _DEV_VDP1_H
#define _DEV_VDP1_H

extern uint16_t vdpport;

extern uint8_t vdp_text32[8];
extern uint8_t vdp_text40[8];

extern void vdp_init(void);
extern void vdp_setup(uint8_t *settings) __fastcall;
extern uint8_t vdp_type(void);
extern void vdp_load_font(void);
extern void vdp_restore_font(void);
extern int16_t vdp_rop(struct vdp_rw *op) __fastcall;
extern int16_t vdp_wop(struct vdp_rw *op) __fastcall;
extern void vdp_setup40(void);
extern void vdp_setup32(void);
extern void vdp_set_console(void);

#endif
