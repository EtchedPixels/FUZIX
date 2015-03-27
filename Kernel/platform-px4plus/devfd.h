#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_open(uint8_t minor, uint16_t flag);

int rom_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rom_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rom_open(uint8_t minor, uint16_t flag);

/* asm code */
void rom_sidecar_read(void);
void rom_cartridge_read(void);

/* In common */
extern uint16_t romd_size;
extern uint16_t romd_addr;
extern uint16_t romd_off;
extern uint8_t romd_mode;

/* Set up by boot code */
extern uint8_t sidecar;
extern uint8_t carttype;

#endif /* __DEVRD_DOT_H__ */
