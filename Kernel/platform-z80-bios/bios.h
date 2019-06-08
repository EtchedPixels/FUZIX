#include <rtc.h>
#include <tty.h>
#include <blkdev.h>

extern void vd_init(void);
extern uint8_t vd_read(void);
extern uint8_t vd_write(void);
extern uint16_t callback_disk(uint16_t) __z88dk_fastcall;

#define VD_DRIVE_NR_MASK	0x0F
#define VD_DRIVE_COUNT		16

struct fuzixbios_info {
    uint8_t version;

    uint8_t num_banks;
    uint8_t common_base;
    uint8_t mem_top;
    uint8_t ram_kb;

    uint8_t num_tty;
    uint8_t num_lpt;
    uint8_t num_disk;

    uint8_t features;
#define FEATURE_RTC	1
};

struct fuzixbios_callbacks {
    uint16_t (*callback_tick)(uint16_t) __z88dk_fastcall;
#define TICK_TIMER	0
#define TICK_VBLANK	1    
    uint16_t (*callback_tty)(uint16_t) __z88dk_fastcall;
    /* One day we may need to handle lock/unlock of disks ? */
    uint16_t (*callback_disk)(uint16_t) __z88dk_fastcall;
#define DISK_CHANGE	0
};
    
extern struct fuzixbios_info *biosinfo;

extern struct fuzixbios_info *fuzixbios_getinfo(void);
extern uint8_t fuzixbios_param(uint8_t *p) __z88dk_fastcall;
extern void fuzixbios_set_bank(void);	/* Must preserve all, and oddity
                                           bank is in A */
extern void fuzixbios_idle(void);
extern void fuzixbios_reboot(void);
extern void fuzixbios_monitor(void);
extern void fuzixbios_set_callbacks(struct fuzixbios_callbacks *vc);

/* TODO : wire up core code so we can nicely set the tty defaults */
struct fuzixbios_ttyparam {
    uint16_t termios_mask;
    uint16_t initial_mode;	/* c_cflags */
    uint16_t features;
#define TF_SCREEN	1
#define TF_SIZE		2
#define TF_KEYBOARD	4
#define TF_VBLANK	8
    uint8_t width;
    uint8_t height;
};

struct fuzixbios_ttyconfig {
    uint8_t device;
    uint8_t flags;		/* Wait etc */
    struct termios termios;
};

/* TODO: get the config word mask, VT properties etc */
extern uint8_t fuzixbios_serial_txready(uint8_t minor) __z88dk_fastcall;
extern void fuzixbios_serial_tx(uint16_t info) __z88dk_fastcall;
extern void fuzixbios_serial_setup(struct fuzixbios_ttyconfig *t) __z88dk_fastcall;
extern uint8_t fuzixbios_serial_carrier(uint8_t minor) __z88dk_fastcall;
extern struct fuzixbios_ttyparam *fuzixbios_serial_param(uint8_t minor) __z88dk_fastcall;
/* TODO: vt graphics hooks, keyboard input events and masks */

extern uint8_t fuzixbios_lpt_busy(uint8_t minor) __z88dk_fastcall;
extern uint8_t fuzixbios_lpt_tx(uint16_t info) __z88dk_fastcall;

struct fuzixbios_diskparam {
    uint32_t blocks;
    uint16_t flags;
#define DP_SWAPPABLE	1
#define DP_PARTITION	2
#define DP_REMOVABLE	4
#define DP_FLOPPY	8
};

/* TODO: media change event, dynamic diskparam, geometyr, fd ioctl,
   fd format, eject, unlock etc */
extern uint8_t fuzixbios_disk_select(uint16_t info) __z88dk_fastcall;
#define SELECT_PROBE		0x0100
#define SELECT_MEDIA_ACK	0x0200
extern uint8_t fuzixbios_disk_set_lba(uint8_t *lba) __z88dk_fastcall;
extern uint8_t fuzixbios_disk_read(uint8_t *addr) __z88dk_fastcall;
extern uint8_t fuzixbios_disk_write(uint8_t *addr) __z88dk_fastcall;
extern uint8_t fuzixbios_disk_flush(struct blkparam *blk) __z88dk_fastcall;
extern struct fuzixbios_diskparam *fuzixbios_disk_param(void);

extern uint8_t fuzixbios_rtc_secs(void);
extern uint8_t fuzixbios_rtc_get(struct cmos_rtc *) __z88dk_fastcall;
extern uint8_t fuzixbios_rtc_set(struct cmos_rtc *) __z88dk_fastcall;

/* TODO: bank 2 bank copier hook */
/* TODO: error codes */
/* Geometry info, drive current v max */
/* FDC settings */

extern uint16_t callback_tick(uint16_t) __z88dk_fastcall;
extern uint16_t callback_tty(uint16_t) __z88dk_fastcall;
