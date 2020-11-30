/* TEMP: multitask should be possible */
#define CONFIG_SINGLETASK

/* Fixed banking */
#define CONFIG_BANK_FIXED

/* 8 16K banks, 1 is kernel */
#define MAPBASE		0x4000
#define MAX_MAPS	2
#define MAP_SIZE	0xB000U

/* Read processes and big I/O direct into process space */
#define CONFIG_LARGE_IO_DIRECT(x)	1

/* Banks as reported to user space */
#define CONFIG_BANKS	1 /* Correct? */

#define TICKSPERSEC 15000000 /* Ticks per second */
#define PROGBASE    0x4000  /* also data base */
#define PROGLOAD    0x4100  /* also data base */
#define PROGTOP     0xF000  /* Top of program, base of U_DATA copy */
#define PROC_SIZE   60	  /* Memory needed per process */

#define CONFIG_FONT_4X6

#define BOOT_TTY (512 + 1)/* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

/* We need a tidier way to do this from the loader */
#define CMDLINE	NULL	  /* Location of root dev name */

/* Device parameters */
#define NUM_DEV_TTY 4

#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define NBUFS    6	  /* Number of block buffers */
#define NMOUNTS	 4	  /* Number of mounts at a time */

#define platform_discard()
#define platform_copyright()
