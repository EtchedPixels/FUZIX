__sfr __at 0x00	fdc_status;		/* uPD 765A, in PIO mode */
__sfr __at 0x01 fdc_data;
__sfr __at 0x88 par0;
__sfr __at 0x89 par1;
__sfr __at 0x8A par2;
__sfr __at 0x8B par3;
__sfr __at 0x8C par4;
__sfr __at 0x8D par5;
__sfr __at 0x8E par6;
__sfr __at 0x8F par7;
__sfr __at 0x9F kempjoy;
__sfr __at 0xA0 amx0;
__sfr __at 0xA1 amx1;
__sfr __at 0xA2 amx2;
__sfr __at 0xA8 gem_data;	/* PC/XT equivalent controller ? */
__sfr __at 0xA9 gem_cs;
__sfr __at 0xAA gem_csp;
__sfr __at 0xAB gem_dmaint;
__sfr __at 0xC8 fax;
__sfr __at 0xD0 kempmouse0;
__sfr __at 0xD1 kempmouse1;
__sfr __at 0xD2 kempmouse2;
__sfr __at 0xD3 kempmouse3;
__sfr __at 0xE0 dart_data_a;
__sfr __at 0xE1 dart_ctrl_a;
__sfr __at 0xE2 dart_data_b;
__sfr __at 0xE3 data_ctrl_b;
__sfr __at 0xE4 i8253_c0;
__sfr __at 0xE5 i8253_c1;
__sfr __at 0xE7 i8253_mode;
__sfr __at 0xF0 bank0;		/* Write only so need shadows for kernel/user */
__sfr __at 0xF1 bank1;
__sfr __at 0xF2 bank2;
__sfr __at 0xF3 bank3;
__sfr __at 0xF4 bank4;
__sfr __at 0xF4 irq;
__sfr __at 0xF5 rollerram;
__sfr __at 0xF6 vscreen;
__sfr __at 0xF7 screenon;
__sfr __at 0xF8 irqcheck;		/* Read */
__sfr __at 0xF8 syscontrol;		/* Write */
#define SYSCTRL_FLOPPY_NMI	2
#define SYSCTRL_FLOPPY_IRQ	3
#define SYSCTRL_FLOPPY_NOIRQ	4
#define SYSCTRL_FLOPPY_SETTC	5
#define SYSCTRL_FLOPPY_CLRTC	6
__sfr __at 0xFC par9512;		/* FIXME: the 9512 are Z180 type decodes */
__sfr __at 0xFD par9512_b;		/* at 00FC 01FC 00FD */
__sfr __at 0xFC iomatrix_d;
__sfr __at 0xFD iomatrix_c;
__sfr __at 0xFE locolink;


extern uint8_t is_joyce;
extern uint8_t model;

#define MODEL_PCW8256		0
#define MODEL_PCW8512		1
#define MODEL_PCW9512		2
#define MODEL_PCW9256		3
#define MODEL_PCW9512PLUS	4

extern void machine_ident(void);

extern uint8_t cps_centronics_busy(void);
extern void cps_centronics_strobe(uint8_t r);
