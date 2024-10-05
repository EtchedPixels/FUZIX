#ifndef __DEVHD_DOT_H__
#define __DEVHD_DOT_H__

/* public interface */
int hd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int hd_open(uint8_t minor, uint16_t flag);
void hd_install(uint8_t slot);

extern uint8_t rw_cmd[7];
extern uint8_t block_rw_pascal(void);
extern uint8_t block_rw_prodos(void);
extern uint8_t block_units[8];

#endif /* __DEVHD_DOT_H__ */
