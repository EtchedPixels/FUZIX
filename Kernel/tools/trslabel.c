#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "../include/diskgeom.h"

struct minipart_lba p;
unsigned char buf[256];
FILE *vol;

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "%s: diskimage\n", argv[0]);
    exit(1);
  }
  vol = fopen(argv[1], "r+");
  if (vol == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (fread(buf, 256, 1, vol) != 1) {
    fprintf(stderr, "%s: failed to read header.\n", argv[1]);
    exit(1);
  }
  if(buf[0] != 0x56 || buf[1] != 0xCB || buf[2] != 0x10 || buf[4] != 1 || buf[6]) {
    fprintf(stderr, "%s: not a Reed 1.0 format hard disc volume.\n", argv[1]);
    exit(1);
  }
  
  memset(&p, 0xFF, sizeof(p));
  p.g.magic = MP_SIG_0;
  p.g.cyl = buf[28] | (buf[29] << 8);
  p.g.head = buf[30];
  printf("Disk is %d cylinders, %d heads\n", p.g.cyl, p.g.head);
  p.g.sec = 16;
  p.g.precomp = 0;
  p.g.land = p.g.cyl + 1;
  p.g.seek = 0;
  p.g.secsize = 8;
  
  p.cyl[0] = 0;
  p.cyl[1] = p.g.cyl - 32;
  p.type[0] = 0x55;
  p.type[1] = 0x56;

  /* FIXME: should fill in matching LBA bits */

  if (fseek(vol, 512 + 384L, SEEK_SET) < 0) {
    perror(argv[1]);
    exit(1);
  }
  if(fwrite(&p, 64, 1, vol) != 1) {
    fprintf(stderr, "%s: unable to write minipart\n");
    exit(1);
  }
  fclose(vol);
}
