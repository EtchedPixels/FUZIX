#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_open(uint8_t minor, uint16_t flag);

void fd_cmd(void *desc);

#endif /* __DEVRD_DOT_H__ */
