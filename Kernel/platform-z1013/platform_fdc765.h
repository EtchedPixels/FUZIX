/*
 *	Platform specifics
 */

#define FDC_MOTOR_TIMEOUT	10		/* Seconds */

#define FDC765_MAX_FLOPPY	2		/* Two drives */

/* Assume double sided (we need to wire up ioctl interfaces etc) */
#define fdc765_ds		1
