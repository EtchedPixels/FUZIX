/*
 *
 * CPC standard RAM bank memory expansions ramdisc driver
 */

int rd_open(uint8_t minor, uint16_t flags);
int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

extern uint8_t rd_wr;
extern uint8_t rd_swap_bank;
extern uint8_t rd_swap_mem_port_h;
extern uint8_t rd_proc_bank;
extern uint8_t *rd_dptr;
extern uint16_t rd_swap_bank_addr;
extern uint16_t rd_proc_bank_addr;
extern uint16_t nblock;
extern blkno_t block;

void rd_io(void);
