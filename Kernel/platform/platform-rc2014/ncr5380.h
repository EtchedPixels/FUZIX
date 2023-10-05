extern uint8_t ncr5380_command(void);
extern void ncr5380_reset_on(void);
extern void ncr5380_reset_off(void);
extern void scsi_init(void);
extern uint8_t ncr5380_check(void);

extern uint8_t *scsi_dbuf;
extern uint8_t scsicmd[16];
extern uint8_t scsimsg;
extern uint8_t scsi_idbits;
extern uint8_t scsi_target;
extern uint8_t scsi_burst;
