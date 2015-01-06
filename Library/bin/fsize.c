#include <stdio.h>
#include <stdlib.h>

unsigned int z8016(const unsigned char *buf)
{
  unsigned int r = buf[1];
  r <<= 8;
  r |= buf[0];
  return r;
}

int main(int argc, char *argv[])
{
  FILE *f;
  unsigned char buf[16];
  unsigned int cs, ds, bs, cm;

  while(*++argv) {
    f = fopen(*argv, "r");
    if (f == NULL) {
      perror(*argv);
      exit(1);
    }
    if (fread(buf, 16, 1, f) != 1) {
      perror("fread");
      exit(1);
    }
    fclose(f);
    if (buf[0] != 0xC3 || memcmp(buf+3, "FZX1", 4)) {
      fprintf(stderr, "%s: not a valid Fuzix binary.\n", *argv);
      continue;
    }
    cm = z8016(buf + 7);
    cs = z8016(buf + 9) - 0x100;
    ds = z8016(buf + 11) - cs;
    bs = z8016(buf + 13);
    
    printf("%s: Code %d, Data %d, BSS %d, Chmem %d, Total %d\n",
      *argv, cs, ds, bs, cm, cs + ds + bs);
      
  }
  exit(0);
}
    