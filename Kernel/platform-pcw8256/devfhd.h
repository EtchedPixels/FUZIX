struct dpb {
    uint16_t spt;
    uint8_t  bsh;
    uint8_t  blm;
    uint8_t  exm;
    uint16_t dsm;
    uint16_t drm;
    uint8_t  al0;
    uint8_t  al1;
    uint16_t cks;
    uint16_t off;
    uint8_t  psh;
    uint8_t  phm;
};

extern int devfhd_init(void);

extern uint8_t fhd_drive;
extern uint8_t fhd_op;
extern uint16_t fhd_track;
extern uint16_t fhd_sector;
extern struct dpb *fhd_dpb;

extern uint8_t probe_fidhd(void);
extern uint8_t install_fidhd(void);
extern uint8_t login_fidhd(void);
extern uint8_t flush_fidhd(void);
extern uint8_t rw_fidhd(void);
