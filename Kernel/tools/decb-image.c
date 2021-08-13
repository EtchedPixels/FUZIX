#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
 *	Turn a DECB file into a 64K raw memory dump
 */

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

static unsigned char image[65536];

static void write_image(char *path)
{
  int fd = creat(path, 0666);
  if (fd == -1) {
    perror(path);
    exit(1);
  }
  if (write(fd, image, 65536) != 65536) {
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

  if (argc != 2) {
    fprintf(stderr, "%s [image]\n", argv[0]);
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
        if (base + size > 65536) {
          fprintf(stderr, "Oversized image (0x%x to 0x%x)\n", base, base + size - 1);
          exit(1);
        }
        break;
      case BYTESTREAM:
        if (image[base])
          fprintf(stderr, "Overwrite at 0x%X\n", base);
        image[base++] = c;
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
  write_image(argv[1]);
  exit(0);
}
