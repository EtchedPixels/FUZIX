/***************************************************************
 * Utility to patch CP/M executable files as required by UZI's *
 * CP/M emulator. Copyright (C) 2000, 2001, Hector Peraza.     *
 ***************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

off_t tell(int fd)
{
  return lseek(fd, 0L, 0);
}

int main(int argc, char *argv[]) 
{
  int  fd;
  long fsize;
  unsigned char b1, b2, b3, bfr[14];


  if (argc != 2) {
    fprintf(stderr, "usage: %s filename\n", argv[0]);
    return 1;
  }

  fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "%s: can't open file %s\n", argv[0], argv[1]);
    return 1;
  }

  if (read(fd, bfr, 3) != 3) {
    fprintf(stderr, "%s: read error from %s\n", argv[0], argv[1]);
    return 1;
  }

  b1 = bfr[0];
  b2 = bfr[1];
  b3 = bfr[2];

  if (b1 == 0xc3) {  /* jp already there */
    printf("%s: no patch needed\n", argv[0]);
    close(fd);
    return 0;
  }

  lseek(fd, 0L, SEEK_END);
  fsize = tell(fd);

  bfr[0] = 0x21;    /* ld hl,100h */
  bfr[1] = 0x00;
  bfr[2] = 0x01;
  
  bfr[3] = 0x36;    /* ld (hl),b1 */
  bfr[4] = b1;
  bfr[5] = 0x23;    /* inc hl */

  bfr[6] = 0x36;    /* ld (hl),b2 */
  bfr[7] = b2;
  bfr[8] = 0x23;    /* inc hl */

  bfr[9] = 0x36;    /* ld (hl),b3 */
  bfr[10] = b3;

  bfr[11] = 0xc3;   /* jp 100h */
  bfr[12] = 0x00;
  bfr[13] = 0x01;

  if (write(fd, bfr, 14) != 14) {
    fprintf(stderr, "%s: file write error\n", argv[0]);
    return -1;
  }

  lseek(fd, 0L, SEEK_SET);
  if (tell(fd) != 0L) {
    fprintf(stderr, "%s: seek error\n", argv[0]);
    return -1;
  }

  bfr[0] = 0xc3;    /* jp instruction to our patching code */
  bfr[1] = (fsize + 0x100) & 0xff;
  bfr[2] = ((fsize + 0x100) >> 8) & 0xff;

  if (write(fd, bfr, 3) != 3) {
    fprintf(stderr, "%s: file write error\n", argv[0]);
    return -1;
  }

  close(fd);

  return 0;
}
