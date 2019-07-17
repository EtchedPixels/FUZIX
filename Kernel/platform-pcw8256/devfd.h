#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int fd_open(uint8_t minor, uint16_t flag);

extern uint8_t new_fdc;

/* asm interface */
extern uint16_t fd765_buffer;
extern uint8_t fd765_user;
extern uint8_t fd765_cmdbuf[3];
extern uint8_t fd765_rw_data[9];
extern uint8_t fd765_statbuf[8];

/* Read a sector */
extern int fd765_read_sector(void);
/* Write a sector */
extern int fd765_write_sector(void);
/* 2 byte command */
extern int fd765_cmd2(void);
/* 3 byte command */
extern int fd765_cmd3(void);
/* Int wait */
extern int fd765_intwait(void);

extern void fd_probe(void);
#endif /* __DEVRD_DOT_H__ */
