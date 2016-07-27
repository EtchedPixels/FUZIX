#ifndef __DEVHD_DOT_H__
#define __DEVHD_DOT_H__

/* public interface */
extern int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int hd_open(uint8_t minor, uint16_t flag);

extern void hd_probe(void);

#ifdef _HD_PRIVATE

__sfr __at 0xC0 hd_wpbits;	/* Write protect and IRQ (not used) */
__sfr __at 0xC1 hd_ctrl;	/* Reset and enable bits */
__sfr __at 0xC8 hd_data;
__sfr __at 0xC9 hd_precomp;	/* W/O */
__sfr __at 0xC9 hd_err;		/* R/O */
__sfr __at 0xCA hd_seccnt;
__sfr __at 0xCB hd_secnum;
__sfr __at 0xCC hd_cyllo;
__sfr __at 0xCD hd_cylhi;
__sfr __at 0xCE hd_sdh;
__sfr __at 0xCF hd_status;	/* R/O */
__sfr __at 0xCF hd_cmd;

#define HDCMD_RESTORE	0x10
#define HDCMD_READ	0x20
#define HDCMD_WRITE	0x30
#define HDCMD_VERIFY	0x40	/* Not on the 1010 later only */
#define HDCMD_FORMAT	0x50
#define HDCMD_INIT	0x60	/* Ditto */
#define HDCMD_SEEK	0x70

#define RATE_4MS	0x08	/* 4ms step rate for hd (conservative) */

#define HDCTRL_SOFTRESET	0x10
#define HDCTRL_ENABLE		0x08
#define HDCTRL_WAITENABLE	0x04

#define HDSDH_ECC256		0x80

/* Seek and restore low 4 bits are the step rate, read/write support
   multi-sector mode but not all emulators do .. */

#define MAX_HD	4

extern struct minipart parts[MAX_HD];

extern uint8_t hd_waitready(void);
extern uint8_t hd_waitdrq(void);
extern uint8_t hd_xfer(bool is_read);

/* helpers in common memory for the block transfers */
extern int hd_xfer_in(uint8_t *addr);
extern int hd_xfer_out(uint8_t *addr);

#endif
#endif /* __DEVHD_DOT_H__ */
