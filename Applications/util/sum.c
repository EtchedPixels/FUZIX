/* sum - checksum a file		Author: Martin C. Atkins */

/*
 *	This program was written by:
 *		Martin C. Atkins,
 *		University of York,
 *		Heslington,
 *		York. Y01 5DD
 *		England
 *	and is released into the public domain, on the condition
 *	that this comment is always included without alteration.
 */
/* Stdio removed Alan Cox 2015 */

#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE (512)

int rc = 0;

char *defargv[] = {"-", 0};

void error(const char *s, const char *f);
void sum(int fd, const char *fname);
void putd(int number, int fw, int zeros);

int main(int argc, char *argv[])
{
  register int fd;

  if (*++argv == 0) argv = defargv;
  for (; *argv; argv++) {
	if (argv[0][0] == '-' && argv[0][1] == '\0')
		fd = 0;
	else
		fd = open(*argv, O_RDONLY);

	if (fd == -1) {
		error("can't open ", *argv);
		rc = 1;
		continue;
	}
	sum(fd, (argc > 2) ? *argv : (char *) 0);
	if (fd != 0) close(fd);
  }
  return(rc);
}

void error(const char *s, const char *f)
{

  write(STDERR_FILENO,"sum: ", 5);
  write(STDERR_FILENO,s, strlen(s));

  if (f) write(STDERR_FILENO, f, strlen(f));
  write(STDERR_FILENO, "\n", 1);
}

void sum(int fd, const char *fname)
{
  static char buf[BUFFER_SIZE];
  register int i, n;
  long size = 0;
  unsigned crc = 0;
  unsigned tmp, blks;

  while ((n = read(fd, buf, BUFFER_SIZE)) > 0) {
	for (i = 0; i < n; i++) {
		crc = (crc >> 1) + ((crc & 1) ? 0x8000 : 0);
		tmp = buf[i] & 0377;
		crc += tmp;
		crc &= 0xffff;
		size++;
	}
  }

  if (n < 0) {
	if (fname)
		error("read error on ", fname);
	else
		error("read error", (char *) 0);
	rc = 1;
	return;
  }
  putd(crc, 5, 1);
  blks = (size + (long) BUFFER_SIZE - 1L) / (long) BUFFER_SIZE;
  putd(blks, 6, 0);
  if (fname) {
    write(1, " ", 1);
    write(1, fname, strlen(fname));
  }
  write(1, "\n", 1);
}

void putd(int number, int fw, int zeros)
{
/* Put a decimal number, in a field width, to stdout. */

  char buf[10];
  int n;
  unsigned num;

  num = (unsigned) number;
  for (n = 0; n < fw; n++) {
	if (num || n == 0) {
		buf[fw - n - 1] = '0' + num % 10;
		num /= 10;
	} else
		buf[fw - n - 1] = zeros ? '0' : ' ';
  }
  write(1, buf, fw);
}
