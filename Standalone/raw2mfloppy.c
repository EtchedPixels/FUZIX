/*
 *	Shuffle a disk into the right order
 *
 *	Converts the output of tools like ufs into an MTX512 mfloppy virtual
 *	disk (or vice versa)
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  char buf[8192];	/* 16 sectors */
  int track;
  int side;
  int in, out;
  
  if (argc != 3) {
    fprintf(stderr, "%s: in out\n", argv[0]);
    exit(1);
  }
  
  in = open(argv[1], O_RDONLY);
  if (in == -1) {
    perror(argv[1]);
    exit(1);
  }
  out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if (out == -1) {
    perror(argv[2]);
    exit(1);
  }

  for (side = 0; side < 2; side++) {
    for (track = 0; track < 40; track++) {
      if (lseek(in, 16384 * track + 8192 * side, 0) == -1) {
        perror(argv[1]);
        exit(1);
      }
      if (read(in, buf, 8192) != 8192) {
        perror(argv[1]);
        exit(1);
      }
      if (write(out, buf, 8192) != 8192) {
        perror(argv[2]);
        exit(1);
      }
    }
  }
  close(in);
  close(out);
  exit(0);
}
