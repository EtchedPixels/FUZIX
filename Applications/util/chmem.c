#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
  unsigned int v;
  FILE *fp;
  unsigned char buf[10];
  unsigned short top;
  uint8_t be;

  if (argc != 2 && argc != 3) {
    fprintf(stderr, "%s [executable] {size}\n", argv[0]);
    exit(1);
  }
  if (argc == 2)
    fp = fopen(argv[1], "r");
  else
    fp = fopen(argv[1], "r+");
  if (fp == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (fread(buf, 10, 1, fp) != 1) {
    fprintf(stderr, "%s: too short ?\n", argv[0]);
    exit(1);
  }
  if (buf[3] != 'F' || buf[4] != 'Z'|| buf[5] != 'X' ||
    buf[6] != '1') {
    fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
    exit(1);
  }
  if (buf[0] == 0x7E || buf[0] == 0x20)
    be = 1;	/* 6809 */
  else if (buf[0] == 0x4C || buf[0] == 0x38)
    be = 0;	/* 6502 */
  else if (buf[0] == 0xC3 || buf[0] == 0x18)
    be = 0;	/* Z80 */
  else {
    fprintf(stderr, "%s: unknown Fuzix binary type.\n", argv[1]);
    exit(1);
  }
  if (argc == 2) {
    if (be)
      top = buf[9] | (buf[8] << 8);
    else
      top = buf[8] | (buf[9] << 8);
    if (top)
      printf("Fuzix binary set at %d bytes.\n", top);
    else
      printf("Fuzix binary, set to allocate all available.\n");
    exit(0);
  }
      
  if (sscanf(argv[2], "%u", &v) != 1 || v > 65536) {
    fprintf(stderr, "%s: invalid chmem value '%s'.\n", argv[0], argv[2]);
    exit(1);
  }
  if (be) {
    buf[8] = v >> 8;
    buf[9] = v & 0xFF;
  } else {
    buf[8] = v & 0xFF;
    buf[9] = v >> 8;
  }
  rewind(fp);
  if(fwrite(buf, 10, 1, fp) != 1) {
    fprintf(stderr, "%s: write error.\n", argv[0]);
    exit(1);
  }
  fclose(fp);
  exit(0);
}
