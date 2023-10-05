#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

extern uint8_t new_fdc;

extern void fd_probe(void);
extern int fd765_intwait(void);			/* Int wait */
extern uint8_t *fd765_send_cmd(uint8_t *cmd) __z88dk_fastcall;
extern uint8_t *fd765_send_cmd3(uint8_t *cmd) __z88dk_fastcall;

#endif /* __DEVRD_DOT_H__ */
