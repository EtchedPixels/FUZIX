#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_open(uint8_t minor, uint16_t flag);

int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_open(uint8_t minor, uint16_t flag);

extern uint8_t fd_op(void);

extern uint8_t fd_drive;
extern uint8_t fd_track;
extern uint16_t fd_sector;
extern uint16_t fd_dma;
extern uint8_t fd_page;
extern uint8_t fd_cmd;

#endif /* __DEVFD_DOT_H__ */
