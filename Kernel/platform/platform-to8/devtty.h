extern void poll_keyboard(void);
extern uint8_t in_bios;
extern uint8_t vblank_wait;

extern int gfx_ioctl(uint_fast8_t minor, uarg_t request, char *ptr);

#define VIDMAP_BASE	0x0000