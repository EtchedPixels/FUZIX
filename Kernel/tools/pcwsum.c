#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	uint8_t buf[512];
	uint8_t n = 0;
	uint8_t base;
	int c;
	int fd;

	if (argc != 3) {
		fprintf(stderr, "%s: file <dotmatrix|daisy>.\n", argv[0]);
		exit(1);
	}

	if (strcmp(argv[2], "daisy") == 0)
		base = 0x01;
	else if (strcmp(argv[2], "dotmatrix") == 0)
		base = 0xFF;
	else {
		fprintf(stderr, "%s: unknown machine class '%s'.\n",
			argv[0], argv[2]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}

	if (read(fd, buf, 512) != 512) {
		fprintf(stderr, "Wrong size\n");
		exit(1);
	}
	if (lseek(fd, 0L, SEEK_SET) < 0) {
		perror(argv[1]);
		exit(1);
	}
	for (c = 0x0; c < 0x1ff; c++)
		n += buf[c];
	buf[0x1ff] = base - n;
	printf("Checksum set to %02x\n", buf[0x1ff]);
	if (write(fd, buf, 512) != 512 || close(fd)) {
		fprintf(stderr, "%s: write failed.\n", argv[1]);
		exit(1);
	}
	return 0;
}
