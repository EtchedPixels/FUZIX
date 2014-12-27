#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static int bufpair(unsigned char *p, int n)
{
  return p[n] + 256 *p[n+1];
}

int main(int argc, char *argv[])
{
  FILE *fp;
  unsigned char buf[16];
  int basepage;
  int n;

  if (argc < 2) {
    fprintf(stderr, "%s [executable]\n", argv[0]);
    exit(1);
  }
  for (n = 1; n < argc; n++) {
    fp = fopen(argv[n], "r");
    if (fp == NULL) {
      perror(argv[n]);
      exit(1);
    }
    if (fread(buf, 16, 1, fp) != 1) {
      fprintf(stderr, "%s: too short ?\n", argv[0]);
      exit(1);
    }
    if (buf[0] != 0xC3 || buf[3] != 'F' || buf[4] != 'Z'|| buf[5] != 'X' ||
      buf[6] != '1') {
      fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
      exit(1);
    }
    fclose(fp);
    basepage = buf[7] << 8;
    printf(" base text data  bss   size  hex filename\n");
    printf("%5x%5x%5x%5x%7d%5x %s\n",
      basepage,
      bufpair(buf, 10) - basepage,
      bufpair(buf, 12) - bufpair(buf, 10) - basepage,
      bufpair(buf, 14),
      bufpair(buf, 14) + bufpair(buf, 12) - basepage,
      bufpair(buf, 14) + bufpair(buf, 12) - basepage,
      argv[n]);
  }
  exit(0);
}
