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
  memset(buf, 0, 14);
  buf[0] = 1;
  buf[1] = mdv_sec++;
  memcpy(buf + 4, "FUZIX     ", 10);
  buf[14] = mdv_csum(buf, 14);
}

static void mdv_make_bufhdr(uint8_t *buf, uint8_t rec, uint8_t *bits)
{
  memset(buf, 0, 14);
  buf[0] = 0;
  buf[1] = rec;	/* Actually load page */
  buf[2] = 0;
  buf[3] = 2;	/* Always 512 bytes */
  strncpy(buf+4, "FUZIX     ", 10);
  buf[14] = mdv_csum(buf, 14);
  memcpy(buf + 15, bits, 512);
  buf[527] = mdv_csum(buf + 15, 512);
}

static uint8_t blank[512];
static uint8_t sectormap[512];
static uint8_t fsmap[512];

static uint8_t *mdv_fs_get(FILE *fsp)
{
  int err = fread(fsmap, 512, 1, fsp);
  if (err < 0) {
    perror("read");
    exit(1);
  }
  if (err == 0)	/* EOF - blank to end */
    return blank;
  return fsmap;
}

static void mdv_write_tape(FILE *fp, FILE *fsp)
{
  uint8_t buf[528];
  uint8_t *bits;
  int block = 1;
  

  while (block < 254) {  
    mdv_make_hdr(buf);
    mdv_write(fp, buf, 15);
    if (block == 1 || block == 128)
      bits = sectormap;
    else {
      if (fsp == NULL)
        bits = blank;
      else
        /* Our blocks are in order so we can simply pull the next one */
        bits = mdv_fs_get(fsp);
    }
    mdv_make_bufhdr(buf, block, bits);
    mdv_write(fp, buf, 528);
    block++;
  }
}

static void mdv_make_map(void) {
  int n = 2;
  int i;

  memset(sectormap, 0xFF, 512);
  sectormap[256] = 0xA5;
  sectormap[257] = 0x5A;

  /* Logical to physical map, so start at 2 and go up missing 128 as its
     a copy of this map (as is 1) */

  for (i = 0 ; i < 252; i++) {
    sectormap[i] = n++;
    if (n == 128)
      n++;
  }
}

void main(int argc, char *argv[])
{
  FILE *fp;
  FILE *fsp = NULL;
  uint8_t zero = 0;

  if (argc != 2 && argc != 3) {
    fprintf(stderr, "%s: microdrive.mdr [filesystem]\n", argv[0]);
    exit(1);
  }

  mdv_make_map();
  fp = fopen(argv[1], "w");
  if (fp == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (argc == 3) {
    fsp = fopen(argv[2], "r");
    if (fsp == NULL) {
      perror(argv[2]);
      exit(1);
    }
  }
  mdv_write_tape(fp, fsp);
  fwrite(&zero, 1, 1, fp);
  fclose(fp);
  exit(0);
}
