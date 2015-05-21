#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

static void writes(const char *p)
{
	write(2, p, strlen(p));
}

static long parse_number(const char *p, int base)
{
	char *end;
	unsigned long result;

	errno = 0;
	result = strtoul(p, &end, base);
	if (errno || *end)
		return -1;
	return result;
}

int do_mknod(char *path, char *modes, char *devs)
{
	int mode;
	int dev;

	mode = parse_number(modes, 8);
	if (mode < 0) {
		writes("mknod: bad mode\n");
		return 1;
	}

	if (!S_ISFIFO(mode) && !S_ISDEV(mode)) {
		writes("mknod: mode is not device/fifo\n");
		return 1;
	}

	dev = parse_number(devs, 10);
	if (dev < 0) {
		writes("mknod: bad device\n");
		return 1;
	}

	if (mknod(path, mode, dev) != 0) {
		perror("mknod");
		return 1;
	}
	return (0);
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		writes("usage: mknod path modes devs\n");
		return 1;
	}
	return do_mknod(argv[1], argv[2], argv[3]);
}
