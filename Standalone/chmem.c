#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
  int v;
  FILE *fp;
  unsigned char buf[8];

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
  if (fread(buf, 8, 1, fp) != 1) {
    fprintf(stderr, "%s: too short ?\n", argv[0]);
    exit(1);
  }
  if (buf[0] != 0xC3 || buf[3] != 'U' || buf[4] != 'Z') {
    fprintf(stderr, "%s: not an UZI binary format.\n", argv[1]);
    exit(1);
  }
  if (argc == 2) {
    if (buf[5] == 'I')
      printf("classic UZI binary.\n");
    else if (buf[5] & 0x80)
      printf("chmem UZI binary set at %d bytes.\n", 
        (buf[5] & 0x7F) << 9);
    else
      printf("UZI binary with unknown tail byte %d\n", buf[5]);
    exit(0);
  }
      
  if (sscanf(argv[2], "%d", &v) != 1 || v < 0 || v > 65536) {
    fprintf(stderr, "%s: invalid chmem value '%s'.\n", argv[0], argv[2]);
    exit(1);
  }
  if (v == 0)
    buf[5] = 'I';
  else
    buf[5] = ((v + 511) >> 9) | 0x80;
  rewind(fp);
  if(fwrite(buf, 8, 1, fp) != 1) {
    fprintf(stderr, "%s: write error.\n", argv[0]);
    exit(1);
  }
  fclose(fp);
  exit(0);
}
