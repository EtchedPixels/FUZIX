#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

/*
 *   Shift loading address of low-address DECB segments
 *
 *   Since the bootloader runs from internal memory
 *   in the 0-0x2000 range, we cannot map banks there
 *   while loading, and some tricks are needed:
 *
 *   Segments below 0x2000 are shifted up 0x2000 bytes so
 *   that they can be initially loaded into an MMU bank
 *   at 0x2000 which later will be remapped at 0x0000.
 *
 *   Also map in the lower address banks since the
 *   bootloader only maps banks at 0x8000 and up.
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

int main(int argc, char *argv[])
{
  unsigned int c;
  unsigned int base, size;
  unsigned int execmsb;
  bool shifted = false;

  /* map banks 5,6,7 at 0x2000-0x7fff */
  printf("%c%c%c%c%c%c", 0, 0, 0x01, 0xff, 0xa1, 0x05);
  printf("%c%c%c%c%c%c", 0, 0, 0x01, 0xff, 0xa2, 0x06);
  printf("%c%c%c%c%c%c", 0, 0, 0x01, 0xff, 0xa3, 0x07);

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
        execmsb = c;
        state = POSTAMBLE_4;
        break;
      case POSTAMBLE_4:
        state = DONE;
        /* write out postamble */
        printf("%c%c%c%c%c", 0xff, 0, 0, execmsb, c);
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
        if (c < 0x20) {
            c += 0x20; /* shift it up */
            if (!shifted) {
		/* map bank 4 temporarily at 0x2000 */
                printf("%c%c%c%c%c%c", 0, 0, 0x01, 0xff, 0xa1, 0x04);
                shifted = true;
            }
        } else if (shifted) {
	    /* map back bank 5 at 0x2000 */
            printf("%c%c%c%c%c%c", 0, 0, 0x01, 0xff, 0xa1, 0x05);
            shifted = false;
        }
        base = c << 8;
        state = PREAMBLE_4;
        break;
      case PREAMBLE_4:
        base += c;
        state = BYTESTREAM;
        if (base + size > 65535) {
          fprintf(stderr, "Oversized image\n");
          exit(1);
        }
        /* write out preamble */
        printf("%c%c%c%c%c", 0, size >> 8, size & 0xFF, base >> 8, c);
        fprintf(stderr, "preamble %04x (%04x)\n", base, size);
        break;
      case BYTESTREAM:
        printf("%c", c);
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
  exit(0);
}
