/*
 *	Minimalist SASI/SCSI interface
 *
 *	Just what is needed for disk block I/O using the minimum number
 *	of commands. We have to minimally support sense data because of
 *	the 'corrected error' funny.
 *
 *	The host controller supplies
 *
 *	scsi_cmd(dev, command, buffer);
 *	which returns 0 if it completed, non zero if not
 *	it places the status byte in scsi_status. The helper
 *	is responsible for honouring td_raw and td_page.
 *
 *	scsi_reset();
 *	Reset the bus and wait as needed.
 *
 *	scsi_id is set by the code to the ID of the host controller
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <tinyscsi.h>

#ifdef CONFIG_TD_SCSI

uint8_t scsi_dev[CONFIG_TD_NUM];
uint_fast8_t scsi_first;
uint_fast8_t scsi_num;
uint8_t scsi_id;		/* Our initiator ID */

static uint8_t cmd_rw[6] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };
static uint8_t cmd_rw10[10] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00
};

static const uint8_t cmd_sense[6] = {
  0x03, 0x00, 0x00, 0x00, 0x10, 0x00
};

int scsi_sense(uint_fast8_t dev, uint8_t *buf)
{
  /* The sense goes to kernel so save and restore the tinydisk map */
  uint8_t mem = td_raw;
  uint8_t r;

  td_raw = 0;
  r = scsi_cmd(dev, cmd_sense, buf, 36);
  td_raw = mem;
  return r;
}

int scsi_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t * dptr)
{
  uint8_t buf[16];
  dev = scsi_dev[dev - scsi_first];
  /* Only use the 10 byte command if needed, older drives don't know it */
  if (lba > 0xFFFFF) {
      cmd_rw10[0] = is_read ? 0x2A : 0x2A; /* READ/WRITE EXTENDED */
      cmd_rw10[2] = lba >> 24;
      cmd_rw10[3] = lba >> 16;
      cmd_rw10[4] = lba >> 8;
      cmd_rw10[5] = lba;
      return 1;
  } else {
    cmd_rw[0] = is_read ? 0x08 : 0x0A;	/* READ6/WRITE6 */
    cmd_rw[1] = lba >> 16;
    cmd_rw[2] = lba >> 8;
    cmd_rw[3] = lba;
  }
  if (scsi_cmd(dev, cmd_rw, dptr, 512))
    return 0;
  if (scsi_status[0] & 0x02) {
      if (scsi_sense(dev, buf))
        return 1;
      /* Oddity: an error code 0108 means the disk got the data
         but had to correct it */
      if ((buf[0] & 0xFFF) == 0x0108) {
        kprintf("Correctable error on scsi%d\n", dev);
        if (lba > 0xFFFFF) {
          cmd_rw10[0] = 0x2A; /* Write long */
          scsi_cmd(dev, cmd_rw10, dptr, 512);
        } else {
          cmd_rw[0] = 0x0A;	/* WRITE6 */
          /* Write the dying block back */
          scsi_cmd(dev, cmd_rw, dptr, 512);
        }
      } else	/* Other codes mean we failed */
        /* TODO - dump the sense data */
        return 0;
  }
  return 1;
}

#endif
