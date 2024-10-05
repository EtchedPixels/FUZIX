
extern uint16_t features;

/* One of these is set for machine type */
#define FEATURE_FALCON	0x8000		/* Falcon */
/* 68030 16MHz, 68881/2, 56001DSP, VIDEL video, blitter, DMA audio, crossbar,
   2.5" IDE, 1.4MB FDC, SCSI, 2xRS232, enhanced joysticks, RS422 */
#define FEATURE_TT	0x4000		/* Atari TT */
/* 68030 32MHz, new graphics, extra MFP, VME, VGA, SCSI, no blitter, 1.44MB FDC,
fast RAM that isn't DMA capable, 3xRS232, RS422 */
#define FEATURE_STE	0x2000		/* Atari STe */
/* 68000 8MHz, blitter, PCM audio, extra joystick features, more colours */
#define FEATURE_MSTE	0x1000		/* Mega STe */
/* 68000 + 68881/2FPU, VME, 3xRS232, RS422, 1.44MB FDC, 3 button mouse 8/16MHz
   extra joystick features, ACSI/SCSI, blitter, more colours */
#define FEATURE_ST	0x0000		/* ST STF STFM Mega ST */
/* 68000 8MHz, 720K floppy (360 on earliest), blitter on some MegaST */
/* FIXME: can we tell MegaST by the RTC ? */

/* TODO: Stacy and STBook */

#define MACHINE_TYPE (features & 0xFF00)	

/* Device features */
#define FEATURE_IDE	0x0080		/* IDE controller - inbuilt or add in */
#define FEATURE_VME	0x0040		/* Has VME bus */
#define FEATURE_RTC	0x0020		/* Add in RTC */
#define FEATURE_BLITTER	0x0010		/* Has a Blitter */
#define FEATURE_TTRTC	0x0008		/* TT style MC146818A */

/* Various other things could be tested for but they are basically machine
   tied anyway. One exceptio we need to tidy up somewhere is 1.44MB floppy */

/* TODO: FPU detect */