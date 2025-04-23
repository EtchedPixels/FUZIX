extern void nap20(void);
extern void ch375_rblock(uint8_t *ptr);
extern void ch375_wblock(uint8_t *ptr);

__sfr __at 0xBE ch375_dport;
__sfr __at 0xBF ch375_sport;

#define ch375_rdata()	ch375_dport
#define ch375_rstatus()	ch375_sport

#define ch375_wdata(x)	do {ch375_dport = (x); } while(0)
#define ch375_wcmd(x)	do {ch375_sport = (x); } while(0)

