#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  unsigned char buf[512];
  unsigned char n = 0;
  int c;
  
  if (read(0, buf, 512) != 512) {
    fprintf(stderr, "Wrong size\n");
    exit(1);
  }
  
  for (c = 0x0; c < 0x1ff; c++)
    n+=buf[c];
  printf("Checksum byte is %02x, should be %02x\n",
    buf[0x1ff], 0xff-n);
  return 0;
}
