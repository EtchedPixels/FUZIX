#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static int mdv_sec = 1;

static uint8_t mdv_csum(uint8_t *data, int len)
{
  uint32_t sum = 0;
  while(len--) {
    sum += *data++;
  }
  return sum % 0xFF;
}


static void mdv_write(FILE *fp, uint8_t *data, int len)
{
  if (fwrite(data, len, 1, fp) != 1) {
    perror("mdv_write");
    exit(1);
  }
}

static void mdv_make_hdr(uint8_t *buf)
{
  printf("Physical block %d: ", mdv_sec);
  memset(buf, 0, 14);
  buf[0] = 1;
  buf[1] = mdv_sec++;
  memcpy(buf + 4, "FUZIX BOOT", 10);
  buf[14] = mdv_csum(buf, 14);
}

static void mdv_make_bufhdr(uint8_t *buf, uint8_t rec, uint8_t *bits)
{
  printf("logical %d for %04x\n", rec, ((uint16_t)rec) << 8);
  memset(buf, 0, 14);
  buf[0] = 0;
  buf[1] = rec;	/* Actually load page */
  buf[2] = 0;
  buf[3] = 2;	/* Always 512 bytes */
  strncpy(buf+4, "FKERNEL.OS", 10);
  buf[14] = mdv_csum(buf, 14);
  memcpy(buf + 15, bits, 512);
  buf[527] = mdv_csum(buf + 15, 512);
}

static uint8_t data[65536];


static void mdv_write_tape(FILE *fp)
{
  uint8_t buf[528];
  uint32_t dl = 0x5B00;
  
  
  while(dl < 0xA000) {
    mdv_make_hdr(buf);
    mdv_write(fp, buf, 15);
    mdv_make_bufhdr(buf, dl >> 8, &data[dl]);
    mdv_write(fp, buf, 528);
    dl += 512;
  }

  dl = 0xC000;
  while(dl < 0x10000) {
    mdv_make_hdr(buf);
    mdv_write(fp, buf, 15);
    mdv_make_bufhdr(buf, dl>> 8, &data[dl]);
    mdv_write(fp, buf, 528);
    dl += 512;
  }
}

void main(int argc, char *argv[])
{
  FILE *fp;

  if (argc != 4) {
    fprintf(stderr, "%s: input.bin cartridge.rom microdrive.mdr\n", argv[0]);
    exit(1);
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (fread(data, 1, 65536, fp) < 49152) {
    fprintf(stderr, "%s: short binary ?\n", argv[1]);
    exit(1);
  }
  fclose(fp);
  
  fp = fopen(argv[2], "w");
  if (fp == NULL) {
    perror(argv[2]);
    exit(1);
  }
  if (fwrite(data, 16384, 1, fp) != 1) {
    perror(argv[2]);
    exit(1);
  }
  fclose(fp);
  fp = fopen(argv[3], "w");
  if (fp == NULL) {
    perror(argv[3]);
    exit(1);
  }
  mdv_write_tape(fp);
  /* Double it up */
  mdv_write_tape(fp);
  fclose(fp);
  exit(0);
}
