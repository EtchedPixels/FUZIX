/*
 *	Platform specifics
 */

#define FDC_MOTOR_TIMEOUT	10		/* Seconds */

#define FDC765_MAX_FLOPPY	2		/* Two drives */


/* Hard code for now. It's not clear if we could support an external DS drive
   need to check the circuit diagrams */
#define fdc765_ds		0
#define fdc765_present		3