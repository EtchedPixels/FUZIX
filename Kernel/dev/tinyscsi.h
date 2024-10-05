#ifndef TINYSCSI_H
#define TINYSCSI_H

void scsi_probe(uint_fast8_t my_id);
int scsi_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t * dptr);
int scsi_sense(uint_fast8_t dev, uint8_t *buf);

int scsi_cmd(uint_fast8_t dev, uint8_t *cmd, uint8_t *data, uint16_t len);
void scsi_reset(void);

extern uint8_t scsi_status[2];
extern uint8_t scsi_id;

#endif
