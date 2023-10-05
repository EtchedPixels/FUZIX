/*
 *	VT helpers provided by each video target
 */

#define VT_CON	4

extern int vtzx_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern uint8_t carrier_unwired(uint_fast8_t minor);

/* TMS9918A */
extern void tms_cursor_off(void);
extern void tms_cursor_disable(void);
extern void tms_cursor_on(int8_t y, int8_t x);
extern void tms_plot_char(int8_t y, int8_t x, uint16_t c);
extern void tms_clear_lines(int8_t y, int8_t ct);
extern void tms_clear_across(int8_t y, int8_t x, int16_t l);
extern void tms_vtattr_notify(void);
extern void tms_scroll_up(void);
extern void tms_scroll_down(void);
extern void tms_setoutput(uint8_t minor);
extern int tms_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void tms_set_mode(uint8_t minor);
extern void tms_set_console(void);

extern uint8_t tms_do_setup(void);

/* FIXME: probably can't use fastcall due to banking */
extern uint16_t vdp_rop(struct vdp_rw *rw) __fastcall;
extern uint16_t vdp_wop(struct vdp_rw *rw) __fastcall;

/* EF9345 */
extern void ef_cursor_off(void);
extern void ef_cursor_disable(void);
extern void ef_cursor_on(int8_t y, int8_t x);
extern void ef_plot_char(int8_t y, int8_t x, uint16_t c);
extern void ef_clear_lines(int8_t y, int8_t ct);
extern void ef_clear_across(int8_t y, int8_t x, int16_t l);
extern void ef_vtattr_notify(void);
extern void ef_scroll_up(void);
extern void ef_scroll_down(void);
extern void ef_setoutput(uint8_t minor);
extern int ef_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void ef_set_console(void);

extern uint8_t ef9345_probe(void);
extern void ef9345_init(void);
extern void ef9345_colour(uint16_t c);
extern void ef9345_set_output(void);

/* Maccasoft */
extern void ma_cursor_off(void);
extern void ma_cursor_disable(void);
extern void ma_cursor_on(int8_t y, int8_t x);
extern void ma_plot_char(int8_t y, int8_t x, uint16_t c);
extern void ma_clear_lines(int8_t y, int8_t ct);
extern void ma_clear_across(int8_t y, int8_t x, int16_t l);
extern void ma_vtattr_notify(void);
extern void ma_scroll_up(void);
extern void ma_scroll_down(void);
extern void ma_setoutput(uint8_t minor);
extern int ma_ioctl(uint8_t minor, uarg_t arg, char *ptr);
extern void ma_set_console(void);

#ifdef CONFIG_RC2014_EXTREME
#define MACCA_BASE	0x40B8
#else
#define MACCA_BASE	0x40
#endif

extern uint8_t macca_init(void);

extern uint8_t inputtty, outputtty;
extern uint8_t vswitch;
extern uint8_t curvid;

#define VID_NONE		0
#define VID_TMS9918A		1
#define VID_EF9345		2
#define VID_MACCA		3

extern uint8_t vidcard[5];
extern struct vt_switch ttysave[VT_CON];
