#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
  int v;
  FILE *fp;
  unsigned char buf[10];
  unsigned short top;

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
  /* FIXME : add 6809 but remember its big endian! */
  if ((buf[0] != 0x4C && buf[0] != 0xC3) || buf[3] != 'F' || buf[4] != 'Z'|| buf[5] != 'X' ||
    buf[6] != '1') {
    fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
    exit(1);
  }
  if (argc == 2) {
    top = buf[8] | (buf[9] << 8);
    if (top)
      printf("Fuzix binary set at %d bytes.\n", top);
    else
      printf("Fuzix binary, set to allocate all available.\n");
    exit(0);
  }
      
  if (sscanf(argv[2], "%d", &v) != 1 || v < 0 || v > 65536) {
    fprintf(stderr, "%s: invalid chmem value '%s'.\n", argv[0], argv[2]);
    exit(1);
  }
  buf[8] = v & 0xFF;
  buf[9] = v >> 8;
  rewind(fp);
  if(fwrite(buf, 10, 1, fp) != 1) {
    fprintf(stderr, "%s: write error.\n", argv[0]);
    exit(1);
  }
  fclose(fp);
  exit(0);
}
