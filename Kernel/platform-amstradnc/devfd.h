#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int devfd_open(uint_fast8_t minor, uint16_t flag);
int devfd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int devfd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);

extern void devfd_spindown(void);

extern void fd765_do_nudge_tc(void);
extern void fd765_do_recalibrate(void);
extern void fd765_do_seek(void);
extern void fd765_do_read(void);
extern void fd765_do_write(void);
extern void fd765_do_read_id(void);

extern uint8_t fd765_track;
extern uint8_t fd765_head;
extern uint8_t fd765_sector;
extern uint8_t fd765_status[8];
extern uint8_t* fd765_buffer;
extern uint8_t fd765_sectors;
extern bool fd765_is_user;

#endif /* __DEVRD_DOT_H__ */
