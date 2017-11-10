#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

static void writes(const char *p)
{
	write(2, p, strlen(p));
}

int main(int argc, char *argv[])
{
	uint16_t mode;
	uint16_t dev;

	if (argc !=3 && argc != 5)
		goto usage;

	if (argv[2][1])
		goto badtype;
	switch (*argv[2]) {
	case 'b':
		mode = S_IFBLK|0777;
		break;
	case 'c':
		mode = S_IFCHR|0777;
		break;
	case 'p':
		mode = S_IFIFO|0777;
		break;
	default:
		goto badtype;
	}

	if (*argv[2] != 'p') {
		if (argc != 5)
			goto usage;
		errno = 0;
		dev = atoi(argv[3]) << 8;
		dev |= atoi(argv[4]);
		if (errno)
			goto usage;
	} else if (argc != 3)
		goto usage;

	if (mknod(argv[1], mode, dev) != 0) {
		perror(argv[1]);
		return 1;
	}
	return 0;

badtype:
	writes("mknod: invalid type (must be b c or p)\n");
	return 1;
usage:
	writes("usage: mknod path type {major minor}\n");
	return 1;
}
