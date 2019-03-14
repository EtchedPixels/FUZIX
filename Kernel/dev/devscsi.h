/*
 *	Fuzix SCSI/SASI inteface logic
 *
 *	Based upon the code from OMU68K by the late Dr Steve Hosgood &
 *	Terry Barnaby, licensed under the GPLv2.
 */

/*
 *	Scsi data control block Class 0 comands
 */
struct	Sidcb0 {
	uint8_t	opcode;
	uint8_t	hiblock;	/* Top bits are LUN in SASI & early SCSI */
	uint8_t	miblock;
	uint8_t	loblock;
	uint8_t	noblocks;
	uint8_t	control;
};

/*
 *	Scsi data control block Class 1 comands
 */
struct	Sidcb1 {
	uint8_t	opcode;
	uint8_t	none_1;
	uint8_t none_2;
	uint8_t	hiblock;
	uint8_t	miblock;
	uint8_t	loblock;
	uint8_t	none1;
	uint8_t	hnoblocks;
	uint8_t	lnoblocks;
	uint8_t	control;
};

/* Largest block we should ever need to send */
struct Sidcbmax {
	uint8_t buf[16];
};

struct Sidcmd {
	/* These must come first */
	union {
		struct Sidcb0 cb0;
		struct Sidcb1 cb1;
		struct Sidcbmax bytes;
	} cb;
	/* Length must follow the dcb - some asm code assumes this on
	   certain platforms. If we add a bigger dcb we'll need to deal
	   with the effect */
	uint16_t length;
	uint8_t direction;
#define SIDIR_NONE	0
#define SIDIR_READ	1
#define SIDIR_WRITE	2
	/* We need to think a bit more about what this means and have a
	   LUN map for the blkdev entries ! */
	uint8_t bus;
	uint8_t device;
	uint8_t lun;
};

/*
 *	Scsi commands
 */
# define	SICLASS(a)	(((a) >> 5) & 0x3) /* Scsi opcode class */
# define	SITEST_RDY	0	/* Test unit ready */
# define	SIREZERO	1	/* Rezero unit */
# define	SIREQSENSE	3	/* Request sense */
# define	SIFORMAT	4
# define	SIREASSIGNBLKS	7
# define	SIREAD		8
# define	SIWRITE		0x0A
# define	SISEEK		0x0B
# define	SIREADUSAGE	0x11
# define	SIINQUIRY	0x12
# define	SIMODE_SELECT	0x15
# define	SIRESERVE	0x16
# define	SIRELEASE	0x17
# define	SIMODE_SENSE	0x1A
# define	SISTART_STOP	0x1B
# define	SIREC_DIAG	0x1C
# define	SISEND_DIAG	0x1D
# define	SIREAD_CAP	0x25
# define	SIREADBIG	0x28
# define	SIWRITEBIG	0x2A
# define	SIREAD_DEFECT	0x37
# define	SICERTIFY	0xE2

/*	SCSI Sense errors */
struct	Sierr {
	uint8_t	logadd;
	uint8_t	none0;
	uint8_t	sensekey;
	uint8_t	hiblk;
	uint8_t	mhiblk;
	uint8_t	mlblk;
	uint8_t	loblk;
	uint8_t	none1[5];
	uint8_t	errcode;
	uint8_t	none2[5];
};

/*	IOCTL command structure */
struct	Siioctl {
	struct	Sidcmd si_dcb;		/* Command to scsi */
	uint8_t	*si_data;		/* Pointer to data area */
};

struct	Sicap {
	uint32_t lblock;
	uint32_t blocklen;
};

/*	SCSI errors: FIXME  */
# define	SCSI_SELTIMEOUT	250000	/* Scsi SEL timeout 0.25 secs Approx */
# define	SCSI_HSTIMEOUT	50000	/* Scsi Hand shake timeout */
# define	SIERR_BUSY	1	/* SCSI BUSY timeout */
# define	SIERR_NODEV	2	/* No device with the given ID */
# define	SIERR_NOREQTX	3	/* No request on tx timeout */
# define	SIERR_NOREQRX	4	/* No request on rx timeout */


#define DRIVE_NR_MASK	0x1F

#ifndef NSCSI
#define NSCSI	7
#endif

/* SCSI layer provides */

extern struct Sidcmd si_dcb;
extern uint8_t si_device;
extern uint8_t si_user;

/* Blkdev interface for SCSI disk I/O */
extern uint8_t si_cmd(void);
/* Ioctl */
extern int si_ioctl(uint_fast8_t dev, uarg_t req, char *data);
/* Cache flush */
extern int si_flush(void);
/* Low level command issue (internal use) */
extern int si_docmd(uint8_t *data);

/* Discardables (we don't do hot plug) */
extern void devscsi_init(void);

/* Driver provided */

/* Read dcb.length bytes into the given pointer: honour user/kernel flags */
extern uint8_t si_read(uint8_t *ptr);
/* Write dcb.length bytes from the given pointer: honour user/kernel flags */
extern uint8_t si_write(uint8_t *ptr);
/* Write the dcb command block for length bytes to the device */
extern uint8_t si_writecmd(uint8_t len);
/* Select the device sidev. Must immediately fail if sidev is the controller id */
extern uint8_t si_select(void);
/* Clear anything stuck on the bus, or wrong directions */
extern void si_clear(void);
/* Reset the bus if possible: Oddity - this one fn has to select the controller
   itself then deselect (don't select a drive!) */
extern void si_reset(void);
/* Deselect the scsi controller */
extern void si_deselect(void);
