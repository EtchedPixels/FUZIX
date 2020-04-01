/*
 *	Printer related ioctl interfaces
 */

/* Report the generic parallel printer status */
#define LPIOCSTAT	0x0600

#define LP_ACK		0x01
#define LP_BUSY		0x02
#define LP_ERROR	0x04
#define	LP_SELECT	0x08
#define LP_PAPER	0x10

/* Platform specific interface data */
#define LPIOCGSDATA	0x0610
#define LPIOCGSERR	0x0611
