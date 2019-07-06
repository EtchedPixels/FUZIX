#define CART_DRAGONDOS	1		/* DragonDOS floppy */
#define CART_DELTADOS	2		/* DeltaDOS floppy */
#define CART_RSDOS	3		/* RSDOS Cartridge */
#define CART_ORCH90	4		/* Orchestra Sound */
#define CART_HDBDOS	5		/* Something with HDBDOS */
#define CART_IDE	6		/* Glenside compatible IDE port */
#define CART_TC3	7		/* Cloud 9 TC^3 SCSI */
#define CART_KENTON	8		/* KenTon SCSI */
#define CART_LRTECH	9		/* LR-Tech Super Board and possibly
					   Owlware */
#define CART_HDII	10		/* Disto H Disk SCSI */
#define CART_4N1	11		/* 4 in 1 card */
#define CART_DRIVEWIRE	12		/* Drivewire card */
#define CART_BECKER	13		/* Becker card */
#define CART_JMCP	14		/* J&M */
#define CART_SDBOOT	15		/* SDBoot */

extern int cart_find(int id);
