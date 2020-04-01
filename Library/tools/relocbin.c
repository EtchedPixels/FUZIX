#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *	This is a close relative of the kernel binman but produces
 *	user space binaries and doesn't have common packing magic or all
 *	the magic segments in the kernel
 */

static uint8_t buf[65536];
static uint8_t bufb[65536];

static unsigned int s__INITIALIZER, s__INITIALIZED;
static unsigned int l__INITIALIZER;

static unsigned int s__DATA, l__DATA;

static unsigned int progload = 0x100;

static void ProcessMap(FILE *fp)
{
  char buf[512];
  
  while(fgets(buf, 511, fp)) {
    char *p1 = strtok(buf," \t\n");
    char *p2 = NULL;
    
    if (p1)
      p2 = strtok(NULL, " \t\n");

    if (p1 == NULL || p2 == NULL)
      continue;
          
    if (strcmp(p2, "s__DATA") == 0)
      sscanf(p1, "%x", &s__DATA);
    if (strcmp(p2, "l__DATA") == 0)
      sscanf(p1, "%x", &l__DATA);
    if (strcmp(p2, "s__INITIALIZED") == 0)
      sscanf(p1, "%x", &s__INITIALIZED);
    if (strcmp(p2, "s__INITIALIZER") == 0)
      sscanf(p1, "%x", &s__INITIALIZER);
    if (strcmp(p2, "l__INITIALIZER") == 0)
      sscanf(p1, "%x", &l__INITIALIZER);
 }
}

static void sweep_relocations(void)
{
  struct exec *ex;
  uint8_t *base = buf + 0x0100;
  uint8_t *base2 = bufb + 0x0200;
  uint8_t *relptr = buf + s__DATA;	/* write relocs into BSS head */
  int relsize;
  int len = s__DATA - 0x0100;
  int pos = 0x0100;
  int lastrel = 0x0100;
  int rels = 0;

  /* As sdcc thinks anything from s__DATA onwards is zero (ie BSS) */
  while(len--) {
    if (*base == *base2) {
      base++;
      base2++;
      pos++;
      continue;
    }
    if (*base == *base2 - 1) {
      int diff = pos - lastrel;
//      printf("Relocation %d at %x\n", ++rels, pos);
      /* 1 - 254 skip that many and reloc, 255 move on 254, 0 end */
      while(diff > 254) {
          diff -= 254;
          *relptr++ = 255;
      }
      *relptr++ = diff;
      lastrel = pos;
      pos++;
      (*base)--;
      base++;
      base2++;
      continue;
    }
    fprintf(stderr, "Bad relocation at %d (%02X v %02X)\n", pos,
      *base, *base2);
    exit(1);
  }
  *relptr++ = 0x00;
  relsize = relptr - (buf + s__DATA);
  /* In effect move the relocations from DATA into INITIALIZED */
//  printf("%d relocations %d bytes BSS\n", relsize, l__DATA);
  s__DATA += relsize;
  if (l__DATA >= relsize)
    l__DATA -= relsize;
  else
    l__DATA = 0;
}

int main(int argc, char *argv[])
{
  FILE *map, *bin;
  uint8_t *bp;
  /* Our primary binary is at 0x0100 but we reloc it down without shifting
     in the buffer */
  static uint16_t progload = 0x0100;

  if (argc != 5) {
    fprintf(stderr, "%s: <binary1> <binary2> <map> <output>\n", argv[0]);
    exit(1);
  }

  bin = fopen(argv[1], "r");
  if (bin == NULL) {
    perror(argv[1]);
    exit(1);
  }
  if (fread(buf, 1, 65536, bin) == 0) {
    fprintf(stderr, "%s: read error on %s\n", argv[0], argv[1]);
    exit(1);
  }
  fclose(bin);
  bin = fopen(argv[2], "r");
  if (bin == NULL) {
    perror(argv[2]);
    exit(1);
  }
  if (fread(bufb, 1, 65536, bin) == 0) {
    fprintf(stderr, "%s: read error on %s\n", argv[0], argv[2]);
    exit(1);
  }
  fclose(bin);
  map = fopen(argv[3], "r");
  if (map == NULL) {
    perror(argv[3]);
    exit(1);
  }
  ProcessMap(map);
  fclose(map);
  
  bin = fopen(argv[4], "w");
  if (bin == NULL) {
    perror(argv[4]);
    exit(1);
  }

  if (s__INITIALIZER + l__INITIALIZER > 65535 || s__INITIALIZED + l__INITIALIZER > 65535 || s__DATA > 65535) {
    fprintf(stderr, "%s: too large.\n", argv[0]);
    exit(1);
  }
  memcpy(buf + s__INITIALIZED, buf + s__INITIALIZER, l__INITIALIZER);
  memcpy(bufb + s__INITIALIZED + 0x100, bufb + s__INITIALIZER + 0x100, l__INITIALIZER);

  sweep_relocations();

  /* Now update the header. We do this byte by byte because we want to be
     sure we get the right alignment when cross building. See
     kernel/include/exec.h */
  bp = buf + progload + 6;
  /* Text size (little endian) */
  *bp++ = s__INITIALIZED - progload;
  *bp++ = (s__INITIALIZED - progload) >> 8;
  /* Data size (little endian) */
  *bp++ = s__DATA - s__INITIALIZED;
  *bp++ = (s__DATA - s__INITIALIZED) >> 8;
  /* BSS size (little endian */
  *bp++ = l__DATA;
  *bp = l__DATA >> 8;
  /* And the rest we dont need to touch for Z80 */

  /* Write out everything that is data, omit everything that will 
     be zapped */
  if (fwrite(buf + progload, s__DATA - progload, 1, bin) != 1) {
   perror(argv[4]);
   exit(1);
  }
  fclose(bin);
  printf("%s: %d bytes from %d\n", argv[4], s__DATA - progload, progload);
  exit(0);
}
