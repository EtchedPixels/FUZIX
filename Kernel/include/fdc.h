#ifndef __FDC_DOT_H__
#define __FDC_DOT_H__

/* For INFO reports the current media, for CAP reports the limits */
struct fdcinfo {
        uint8_t mode;			/* Mode number, current mode on getcap */
        uint8_t type;			/* Type description for mode */
        uint8_t config;
#define FDC_DSTEP       1               /* Double step */
#define FDC_AUTO        2               /* Media autodetect */
#define FDC_SEC0        4               /* Start at sector 0 */
#define FDC_PRECOMP	8		/* Can control precomp on/off */
        uint16_t features;
#define FDF_SD          1               /* Can do single density (FM) */
#define FDF_DD          2               /* Can do double density (MFM) */
#define FDF_HD          4               /* Can do high density */
#define FDF_ED          8               /* Can do extra-high density */

#define FDF_DS          16
#define FDF_8INCH       32              /* Can do 8" (double bit rate) */

#define FDF_SEC128      128
#define FDF_SEC256      256
#define FDF_SEC512      512
#define FDF_SEC1024     1024
#define FDF_SEC2048     2048

#define FDF_SECSIZE	(FDF_SEC128|FDF_SEC256|FDF_SEC512|FDF_SEC1024|FDF_SEC2048)


#define FDF_MEDIACH     8192            /* Has media change event */
#define FDF_EJECT       16384           /* Has an eject mechanism */
#define FDF_HARD        32768           /* Hard sectored */

        /* Drive data: see HDIO_GETGEO for current */
        uint8_t sectors;                /* Per track */
        uint8_t tracks;
        uint8_t heads;
        uint8_t precomp;

};

struct fdcstep {
        uint8_t steprate;		/* in ms */
        uint8_t headload;
        uint8_t settle;
        uint8_t unused;
};

#define FDIO_GETCAP	0x01F0
#define FDIO_GETMODE	0x01F1
#define FDIO_SETMODE	0x41F2
#define	FDIO_FMTTRK	0x01F3
#define FDIO_RESTORE	0x01F4 
#define FDIO_SETSKEW	0x41F5
#define FDIO_SETSTEP	0x41F6

#define FDTYPE_CUSTOM	0x00	/* Not a standard type */

/* Standard PC formats */
#define FDTYPE_PC144	0x01	/* 1.44MB PC */
#define FDTYPE_PC720	0x02	/* 720K 3.5" PC */
#define FDTYPE_PC12	0x03	/* 1.2MB 5.25" PC */
#define FDTYPE_PC360	0x04	/* 360K 5.25" PC */
#define FDTYPE_PC288	0x05	/* 2.88MB EHD PC */
#define FDTYPE_PC180	0x06	/* 180K 5.25" PC style SS 40 track */

/* Amstrad and other 3" floppy formats */
#define FDTYPE_AMS180	0x10	/* 3" 180K SSDD 40 track */
#define FDTYPE_AMS720	0x11	/* 3" 720K DSDD 80 track */

/* Classic 8" formats */
#define FDTYPE_8SSSD	0x18	/* 8" SSSD 128 byte sector 77 track */
#define FDTYPE_8DSSD	0x19	/* 8" DSSD 128 byte sector 77 track */
#define FDTYPE_8SSDD	0x1A	/* 8" SSDD 256 byte sector 77 track */
#define FDTYPE_8DSDD	0x1B	/* 8" DSDD 256 byte sector 77 track */

/* TRS80 formats 10spt for SD 18 for DD */
#define FDTYPE_TRS35SD	0x20	/* Original TRS80 35 track */
#define FDTYPE_TRS35DD	0x21	/* With doubler */
#define FDTYPE_TRS40SD	0x22	/* 40 track SD */
#define FDTYPE_TRS40DD	0x23	/* With doubler */
#define FDTYPE_TRSDSDD	0x24	/* double sided drives in the later model 4 */

#define FDTYPE_COCO	0x2C	/* 35 track SSDD, 18 spt */

/* System specific formats */
#define FDTYPE_AMIGA	0x30	/* Amiga 880K */
#define FDTYPE_MAC400	0x31	/* Older Mac68K SS (newer is PC144) */
#define FDTYPE_MAC800	0x32	/* Older Mac68K DS (newer is PC144) */
#define FDTYPE_APPL20	0x33	/* Apple II 13 sector */
#define FDTYPE_APPL21	0x34	/* Apple II 16 sector */

/* BBC micro (actually most of the 256 byte formats. Define any equivalent
   systems to the same values) */
#define FDTYPE_BBCSD40	0x40	/* 10 x 256 byte FM 40 track */
#define FDTYPE_BBCSD80	0x41	/* 80 track variant */
#define FDTYPE_BBCADFSS	0x42	/* 16 x 256 byte MFM 40 track */
#define FDTYPE_BBCADFSM	0x43	/* 80 track verson */
#define FDTYPE_BBCADFSL	0x44	/* Double sided 80 track version */
/* 1K sector variants: all 80 track DSDD/DSHD */
#define FDTYPE_BBCADFSD	0x45	/* 2 sides, 5 sectors */
#define FDTYPE_BBCADFSE	0x45	/* Physically ADFSD */
#define FDTYPE_BBCADFSF	0x46	/* 2 sides, 10 sectors HD */
#define FDTYPE_BBCADFSG	0x47	/* 2 sides, 20 sectors EHD */
/* Other uses of the 256 byte 16spt formats */
#define FDTYPE_BETA40S	0x42	/* Beta disk 40 track SSDD */
#define FDTYPE_BETA80S	0x43	/* Beta disk 80 track SSDD */
#define FDTYPE_BETA40D	0x48	/* No BBC equivalent 40 track DS */
#define FDTYPE_BETA80D	0x44	/* Beta disk 80 track DSDD */

/* 512 byte sector 10 sector per track formats: Microbee and some others. All
   double density */
#define FDTYPE_UBEESS40	0x50	/* 40 track single sided */
#define FDTYPE_UBEEDS40	0x51	/* 40 track double sided */
#define FDTYPE_UBEESS80	0x52	/* 80 track single sided */
#define FDTYPE_UBEEDS80	0x53	/* 80 track double sided */
#define FDTYPE_UBEEDS82	0x53	/* Physically the same */
#define FDTYPE_UBEEDS84	0x53	/* Physically the same */
#define FDTYPE_PC800	0x53	/* PC style 800K */
#define FDTYPE_SAMCOUPE	0x53	/* Another user of the same format */
#define FDTYPE_PC400	0x51	/* PC style 400K */

/* NASCOM PolyDOS. Much like the BBC but 18spt not 16 for MFM. To
   confuse further Nascom CP/M used 512x10x77 */
#define FDTYPE_NASDSSD	0x60	/* Nascom 35 track DSSD 10spt */
#define FDTYPE_NASDSDD	0x61	/* Nascom 35 track DSDD 18spt */
#define FDTYPE_NASSS80	0x62	/* Nascom SSDD 80 track, 18spt */
#define FDTYPE_NASCPMS	0x63	/* 512bps/10spt/77 track SS DD */
#define FDTYPE_NASCPMD	0x64	/* 512bps/10spt/77 track SS DD */
#define FDTYPE_NASPDOS2	FDTYPE_PC400	/* PolyDOS 2 */
#define FDTYPE_NASPDOS3	FDTYPE_PC800	/* PolyDOS 3 (77 track though) */
#define FDTYPE_NASDOS	FDTYPE_8DSDD

/* Geneve - mostly match other people's configurations as an alias */
#define FDTYPE_GENSD40	0x70	/* 40 track 9 sectors per track */
#define FDTYPE_GENSD40D	0x71	/* 40 track 9 sectors per track double sided */
#define FDTYPE_GENDD4016	FDTYPE_BBCADFSS	/* 40 track x 16 */
#define FDTYPE_GENDD8016	FDTYPE_BBCADFSM	/* 80 track x 16 */
#define FDTYPE_GENDD8016D	FDTYPE_BBCADFSL /* 80 track x 16 x 2 */
#define FDTYPE_GENDD4018 0x72	/* 40 x 18 */
#define FDTYPE_GENDD8018	FDTYPE_NASSS80	/* 80 x 18 */
#define FDTYPE_GENDD8018D 0x73
#define FDTYPE_GENHD80D	0x74	/* 80 x 36 x 2 1.44MB media */

#endif
