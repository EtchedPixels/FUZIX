/*
 *	Platform specifics
 */

#define FDC_MOTOR_TIMEOUT	10		/* Seconds */

#define FDC765_MAX_FLOPPY	2		/* Two drives */

extern uint8_t fdc765_present;
extern uint8_t fdc765_ds;		/* Double sided ? */
