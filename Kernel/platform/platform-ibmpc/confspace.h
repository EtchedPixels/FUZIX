/*
 *	The boot time configuration space loaded at 0x800
 */

#define NUM_SER		4
#define NUM_PAR		4
 
struct serconf {
    uint8_t	driver;
#define SER_8250	0
    uint8_t	info;		/* Usually IRQ for non BIOS */
    uint16_t	port;
};

struct parconf {
    uint8_t	driver
#define PAR_BIOS	0
    uint8_t	info;
    uint16_t	port;
};

struct kconf {
    uint16_t	magic;
#define MAGIC	0xBA10
    uint8_t	kbd_driver;
#define KBD_BIOSHOOK	0	/* (info gives int to hook) */
#define KBD_XTKEY	1
    uint8_t	kbd_info;

    uint8_t	video_driver;
#define VID_BIOS	0	/* video_info gives mode to set or FF */
#define VID_PC		1	/* native PC driver */
    uint8_t	video_info[3];

    uint8_t	xms_driver;
#define XMS_OFF		0
#define XMS_RAMDISK	1
    uint8_t	xms_info;
    
    uint8_t	fd_driver
#define FD_BIOS		0
    uint8_t	fd_info;

    uint8_t	hd_driver
#define HD_BIOS		0
    uint8_t	hd_info;

    uint8_t	js_driver;
#define JS_BIOS		0
    uint8_t	js_info;

    uint8_t	ems_driver;
#define EMS_NONE	0
#define EMS_ABOVEBOARD	1
#define EMS_LOTECH	2
#define EMS_OPTI	3
#define EMS_BIOS	4
    uint8_t	ems_ctrl;
    uint16_t	ems_port;
    uint16_t	ems_step;
    uint16_t	ems_off;
    uint16_t	ems_mem;	/* KB or 0 */

    struct serconf ser[NUM_SER];
    struct lpconf  par[NUM_PAR];

};
