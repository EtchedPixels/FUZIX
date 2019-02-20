/*
 *	RAMdisc driver
 */

int rd_open(uint8_t minor, uint16_t flags);
int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

extern uint8_t rd_wr;
extern uint8_t *rd_dptr;
extern uint8_t rd_page;
extern uint16_t rd_addr;

void rd_io(void);
