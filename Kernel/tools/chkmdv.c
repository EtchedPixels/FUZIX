#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/errno.h>

static int mdv_sec;

static int bad = 0;

static void SectorError(const char *p, ...)
{
  va_list v;

  fprintf(stderr, "Block %d: ", mdv_sec);
  va_start(v, p);
  vfprintf(stderr, p, v);
  va_end(v);
  bad++;
}

  
static uint8_t mdv_csum(uint8_t *data, int len)
{
  uint32_t sum = 0;
  while(len--) {
    sum += *data++;
  }
  return sum % 0xFF;
}

static void mdv_check_header(uint8_t *buf, uint8_t sec)
{
  if (buf[0] != 1)
    SectorError("Bad header type: %d", buf[0]);
  if (buf[1] != sec)
    SectorError("Bad header block number: %d", buf[1]);
  if (memcmp(buf + 4, "FUZIX     ", 10))
    SectorError("Bad header magic");
  if (buf[14] != mdv_csum(buf, 14))
    SectorError("Bad header checksum %02x %02x\n", buf[14],
      mdv_csum(buf,14));
}

static void mdv_check_bufhdr(uint8_t *buf, uint8_t rec)
{
  int i;
  int lb = bad;

  if (buf[0] != 0)
    SectorError("Bad block type %d\n", buf[0]);
  if (buf[1] != rec)
    SectorError("Bad block header block number: %d", buf[1]);
  if (buf[2] != 0)
    SectorError("Bad block header zero byte 2: %d", buf[2]);
  if (buf[3] != 2)
    SectorError("Bad block header length byte: %d", buf[3]);
  if (memcmp(buf+4, "FUZIX     ", 10))
    SectorError("Bad block header magic");
  if (buf[14] != mdv_csum(buf, 14))
    SectorError("Bad block header checksum %02x %02x\n", buf[14],
      mdv_csum(buf,14));
  if (buf[527] != mdv_csum(buf+15, 512))
    SectorError("Bad block data checksum %02x %02x\n", buf[527],
      mdv_csum(buf+15, 512));
  for (i = 0; i < 32; i++) {
    if (i != 4 && memcmp(buf + i, "FUZIX     ", 10) == 0)
      SectorError("FUZIX tag at %d not 4\n", i);
  }
  if (bad > lb) {
    for (i = 0; i < 544; i++) {
      if (!(i & 15))
        printf("%03x: ", i);
      printf("%02x ", buf[i]);
      if ((i & 15) == 15)
        printf("\n");
    }
  }
}

static void mdv_check_sectormap(uint8_t *buf)
{
  ; /* TODO: Insert description */
}

static void mdv_read(FILE *fp, char *buf, int len)
{
  if (fread(buf, len, 1, fp) != 1) {
    if (errno)
      perror("fread");
    else
      fprintf(stderr, "Unexpected EOF\n");
  }
}

static void mdv_check_tape(FILE *fp)
{
  uint8_t buf[528];
  uint8_t *bits;
  int block = 1;
  

  while (block < 254) {  
    mdv_sec = block;
    mdv_read(fp, buf, 15);
    mdv_check_header(buf, block);
    mdv_read(fp, buf, 528);
    if (block == 1 || block == 128)
      mdv_check_sectormap(buf);
    else
      mdv_check_bufhdr(buf, block);
    block++;
  }
}

void main(int argc, char *argv[])
{
  FILE *fp;
  FILE *fsp = NULL;
  uint8_t zero = 0;

  if (argc != 2) {
    fprintf(stderr, "%s: microdrive.mdr\n", argv[0]);
    exit(1);
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror(argv[1]);
    exit(1);
  }
  mdv_check_tape(fp);
  fclose(fp);
  exit(0);
}
