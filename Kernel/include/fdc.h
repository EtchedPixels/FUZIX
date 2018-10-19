#ifndef __FDC_DOT_H__
#define __FDC_DOT_H__

/* For INFO reports the current media, for CAP reports the limits */
struct fdcinfo {
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
        uint8_t steprate;
        uint8_t precomp;
        uint8_t spare;

        uint8_t config;
#define FDC_DSTEP       1               /* Double step */
#define FDC_AUTO        2               /* Media autodetect */
#define FDC_SEC0        4               /* Start at sector 0 */
#define FDC_PRECOMP	8		/* Can control precomp on/off */
 
        uint8_t fmttype;                /* Type of format buffer needed */
#define FDC_FMT_AUTO		0	/* No buffer needed */
#define FDC_FMT_17XX            1
#define FDC_FMT_765             2
#define FDC_FMT_8271		3
        uint16_t fmtbuf;                /* Size of required format buffer */

        uint16_t spare1;
        uint16_t spare2;		/* to 16 bytes */
};

#define FDIO_GETCAP	0x01F0
#define FDIO_GETINFO	0x01F1
#define FDIO_SETINFO	0x41F2
#define	FDIO_FMTTRK	0x01F3
#define FDIO_RESTORE	0x01F4 
#define FDIO_SETSKEW	0x41F5

#endif
