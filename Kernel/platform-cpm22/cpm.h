/*
 *	CP/M 2 functionality
 */

/* CP/M 3 differs ... */

struct cpm_dpb {
    uint16_t spt;
    uint8_t bsh;
    uint8_t blm;
    uint8_t exm;
    uint16_t dsm;
    uint16_t drm;
    uint8_t al0;
    uint8_t al1;
    uint16_t cks;
    uint16_t off;
};

struct cpm_dph {
    void *xlt;
    uint16_t workspace[3];
    uint8_t *buffer;
    struct cpm_dpb *dpb;
    uint16_t csv;
    uint16_t alv;
};
    
extern uint8_t cpm_const(void);
extern uint8_t cpm_conin(void);
extern void cpm_conout(uint8_t c) __z88dk_fastcall;
extern void cpm_list(uint8_t c) __z88dk_fastcall;
extern void cpm_punch(uint8_t c) __z88dk_fastcall;
extern uint8_t cpm_reader(void);
extern void cpm_home(void);
extern struct cpm_dph *cpm_seldsk(uint16_t disk) __z88dk_fastcall;
#define SELDSK_WARM	0xFF00

extern void cpm_settrk(uint16_t track) __z88dk_fastcall;
extern void cpm_setsec(uint16_t sector) __z88dk_fastcall;
extern void cpm_setdma(uint8_t *dma) __z88dk_fastcall;
extern uint8_t cpm_read(void);
extern uint8_t cpm_write(void);
extern uint8_t cpm_listst(void);
extern uint16_t cpm_sectran(uint16_t sector, void *xlt);

/*
 *	Banked I/O helpers
 */

extern uint8_t cpm_diskread(void);
extern uint8_t cpm_diskwrite(uint8_t hint) __z88dk_fastcall;
extern uint8_t cpm_map;

/*
 *	Avoid re-entrancy
 */

extern uint8_t cpm_busy;
