#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static unsigned int
bufpair(int bigend, unsigned char *p, int n)
{
  if (bigend) {
    /* mc6809, big endian */
    return (p[n] << 8) | p[n+1];
  }
  return p[n] | (p[n+1] << 8);
}

int main(int argc, char *argv[])
{
  FILE *fp;
  unsigned char buf[16];
  unsigned int basepage;
  int n, endian;
  unsigned int txtsz, datsz, bsssz;

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

    basepage = 0;
    if (buf[0] == 0xC3 || buf[0] == 0x18 || buf[0] == 0x4C || buf[0] == 0x38) {
	/* Z-80 or 6502 */
	endian = 0;
	basepage = buf[7] << 8;
    } else if (buf[0] == 0x7E || buf[0] == 0x20) {
	/* 6809 */
	endian = 1;
    } else {
	endian = -1;
    }
    if ((endian == -1) ||
	    (buf[3] != 'F') ||
	    (buf[4] != 'Z') ||
	    (buf[5] != 'X') ||
	    (buf[6] != '1')) {
	fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
	exit(1);
    }
    fclose(fp);
    printf(" base text data  bss   size  hex filename\n");

    /* Text, data, BSS */
    txtsz = bufpair(endian, buf, 10);
    datsz = bufpair(endian, buf, 12);
    bsssz = bufpair(endian, buf, 14);

    printf("%5x%5x%5x%5x%7d%5x %s\n",
      basepage,
      txtsz, datsz, bsssz,
      txtsz+datsz+bsssz,
      txtsz+datsz+bsssz,
      argv[n]);
  }
  exit(0);
}
