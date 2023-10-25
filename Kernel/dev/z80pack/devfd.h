#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int fd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int fd_open(uint_fast8_t minor, uint16_t flag);

int hd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int hd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int hd_open(uint_fast8_t minor, uint16_t flag);

void fd_bankcmd(uint16_t cmd, uint16_t *bank);

#endif /* __DEVRD_DOT_H__ */
