/*
 *	Mark an HDF file as LBA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  unsigned char buf[512];
  FILE *f;
  
  if (argc != 2) {
    fprintf(stderr, "%s: hdffile\n", argv[0]);
    exit(1);
  }
  
  f = fopen(argv[1], "r+");
  if (f == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (fread(buf, 512, 1, f) != 1) {
    fprintf(stderr, "%s: short read.\n", argv[1]);
    exit(1);
  }
  if (memcmp(buf, "RS-IDE", 6)) {
    fprintf(stderr, "%s: not an HDF image.\n", argv[1]);
    exit(1);
  }
  buf[0x16 + 99] |= 2;
  rewind(f);
  if (fwrite(buf, 512, 1, f) != 1) {
    fprintf(stderr, "%s: short write.\n", argv[1]);
    exit(1);
  }
  if (fclose(f)) {
    perror(argv[1]);
    exit(1);
  }
  exit(0);
}
