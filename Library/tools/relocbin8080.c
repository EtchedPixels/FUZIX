#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *	This is a close relative of the kernel binman but produces
 *	user space binaries and doesn't have common packing magic or all
 *	the magic segments in the kernel
 *
 *	Our input is
 *	file 1:	linked at 0x100
 *	file 2: linked at 0x200
 */

static uint8_t buf[65536];
static uint8_t bufb[65536];
static int32_t bsize, csize, dsize;
static uint16_t numzp;

static void sweep_relocations(void)
{
  /* We link at 0x0100 and 0x0200. Do not relocate the header except for
     the sig vector. */
  uint8_t *base = buf + 0x10;
  uint8_t *base2 = bufb + 0x10;
  uint8_t *relbase = buf + csize + dsize; /* write relocs into BSS head */
  uint8_t *relptr = relbase;
  int relsize;
  int len = csize + dsize - 0x10;
  int pos = 0x10;
  int lastrel = 0;

  /* Magic fudge. We will patch the relocation base into 0x12/0x13 which is
     currently zero in both cases. We want to relocate it so our loader is
     cleaner */
  bufb[0x12] = 0x01;	/* Force a relocation */

  /* Relocation table two is the code/data references */
  while(len--) {
    if (*base == *base2) {
      base++;
      base2++;
      pos++;
      continue;
    }
    if (*base == *base2 - 1) {
      int diff = pos - lastrel;
      /* 1 - 254 skip that many and reloc, 255 move on 254, 0 end */
      while(diff > 254) {
          diff -= 254;
          *relptr++ = 255;
      }
      *relptr++ = diff;
      lastrel = pos;
      pos++;
      (*base)--;	/* 0 base */
      base++;
      base2++;
      continue;
    }
    fprintf(stderr, "Bad relocation at %x (%02X:%02X:%02X v %02X:%02X:%02x)\n", pos,
      base[-1], *base, base[1], base2[-1], *base2, base2[1]);
    fprintf(stderr, "base2 - bufb = %x base-buf = %x\n",
      base2-bufb, base-buf);
    exit(1);
  }
  *relptr++ = 0x00;
  relsize = relptr - relbase;
  /* In effect move the relocations from BSS into DATA */
  dsize += relsize;
  bsize -= relsize;
  /* Corner case - more relocations than data - grow the object size slightly */
  if (bsize < 0)
    bsize = 0;
}

int main(int argc, char *argv[])
{
  FILE *bin;
  uint8_t *bp;
  uint16_t relbase;

  if (argc != 4) {
    fprintf(stderr, "%s: <binary1> <binary2> <output>\n", argv[0]);
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
  
  bin = fopen(argv[3], "w");
  if (bin == NULL) {
    perror(argv[3]);
    exit(1);
  }

  /* Work out sizes from the binary */
  csize = buf[6] + 256 * buf[7];
  dsize = buf[8] + 256 * buf[9];
  bsize = buf[10] + 256 * buf[11];

  /* Offset to start of BSS before adjustment */
  relbase = csize + dsize;

  /* Compute the relocations */
  sweep_relocations();
  
  /* Modify the existing binary header. We touch only the data/bss size */
  buf[3] = 0;	/* Relocatable */
  buf[8] = dsize;
  buf[9] = dsize >> 8;
  buf[10] = bsize;
  buf[11] = bsize >> 8;
  /* User 16/17 are reserved already for signal vector */
  buf[18] = relbase;
  buf[19] = relbase >> 8;
  
  /* Write out everything that is data, omit everything that will 
     be zapped */
  if (fwrite(buf, csize + dsize, 1, bin) != 1) {
   perror(argv[4]);
   exit(1);
  }
  fclose(bin);
//  printf("%s: %d bytes, ZP size %d.\n", argv[3], csize + dsize, numzp);
  exit(0);
}
