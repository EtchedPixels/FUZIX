#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static enum {
  NEW_BLOCK,
  PREAMBLE_1,
  PREAMBLE_2,
  PREAMBLE_3,
  PREAMBLE_4,
  POSTAMBLE_1,
  POSTAMBLE_2,
  POSTAMBLE_3,
  POSTAMBLE_4,
  DONE,
  BYTESTREAM
} state = NEW_BLOCK;

static int copying = 0;

static unsigned char roms[32768];
static unsigned char decb[16384];
static unsigned char *decbptr = decb;

static void write_rom(char *path, unsigned char *buf, int len)
{
  int fd = creat(path, 0666);
  if (fd == -1) {
    perror(path);
    exit(1);
  }
  if (write(fd, buf, len) != len) {
    perror(path);
    exit(1);
  }
  if (close(fd) != 0) {
    perror(path);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  unsigned int c;
  unsigned int base, size;

  if (argc != 5) {
    fprintf(stderr, "%s: [bootrom] [secondrom] [cart1rom] [cart2rom]\n",
      argv[0]);
    exit(1);
  }  
  while(state != DONE && (c = getchar()) != EOF) {
    switch(state) {
      case NEW_BLOCK:
        if (c == 0xFF) {
          state = POSTAMBLE_1;
          break;
        }
        if (c == 0x00) {
          state = PREAMBLE_1;
          break;
        }
        fprintf(stderr, "Corrupt DECB (%d)\n", c);
        exit(1);
        break;

      case POSTAMBLE_1:
        if (c != 0x00) {
          fprintf(stderr, "DECB postamble byte 0 not 0x00\n");
          exit(1);
        }
        state = POSTAMBLE_2;
        break;
      case POSTAMBLE_2:
        if (c != 0x00) {
          fprintf(stderr, "DECB postamble byte 2 not 0x00\n");
          exit(1);
        }
        state = POSTAMBLE_3;
        break;
      case POSTAMBLE_3:
        state = POSTAMBLE_4;
        break;
      case POSTAMBLE_4:
        *decbptr++ = 0xFF;
        *decbptr++ = 0x00;
        *decbptr++ = 0x00;
        *decbptr++ = 0x80;
        *decbptr++ = 0x00;
        state = DONE;
        break;
 
      case PREAMBLE_1:
        size = c << 8;
        state = PREAMBLE_2;
        break;
      case PREAMBLE_2:
        size += c;
        state = PREAMBLE_3;
        break;
      case PREAMBLE_3:
        base = c << 8;
        state = PREAMBLE_4;
        break;
      case PREAMBLE_4:
        base += c;
        state = BYTESTREAM;
        if (base < 0x8000 && base + size > 0x8000) {
          fprintf(stderr, "Split block\n");
          exit(1);
        }
        if (base < 0x8000) {
          copying = 1;
          fprintf(stderr, "Copy block at %04x for %04x\n", base, size);
          *decbptr++ = 0x00;
          *decbptr++ = size >> 8;
          *decbptr++ = size & 0xff;
          *decbptr++ = base >> 8;
          *decbptr++ = base & 0xff;
        }
        else {
          fprintf(stderr, "ROM block at %04x for %04x\n", base, size);
          copying = 0;
        }
        break;
      case BYTESTREAM:
        if (copying)
          *decbptr++ = c;
        else
          roms[base++ - 0x8000] = c;
        size--;
        if (size == 0)
          state = NEW_BLOCK;
        break;
      default:
        fprintf(stderr, "State machine horked.\n");
        exit(1);
    }
  }
  if (state != DONE) {
    fprintf(stderr, "Unexpected EOF in state %d\n", state);
    exit(1);
  }
  write_rom(argv[1], decb, decbptr-decb);
  write_rom(argv[2], roms, 0x4000);
  write_rom(argv[3], roms + 0x4000, 0x2000);
  write_rom(argv[4], roms + 0x6000, 0x2000);
  
  exit(0);
}
