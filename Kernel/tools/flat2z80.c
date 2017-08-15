/*
 *	Convert an unbanked binary into a ZX spectrum +3 image for ease
 *	of testing.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

static uint8_t emptybank[16384];
static uint8_t bank3[16384];
static uint8_t bank4[16384];
static uint8_t bank6[16384];
static uint8_t bank7[16384];

static uint8_t header[86];
static uint8_t blkhdr[3] = { 0xFF, 0xFF, 0x00 };

static void write_buf(char *name, int fd, uint8_t *buf, int len)
{
	if (write(fd, buf, len) != len) {
		perror(name);
		exit(1);
	}
}

static void load_bank(uint8_t *p, int fd)
{
	if (read(fd, p, 16384) < 1) {
		perror("read");
		exit(1);
	}
}

static void load_first_bank(uint8_t *p, int fd)
{
	if (read(fd, p + 256, 16384 - 256) < 1) {
		perror("read");
		exit(1);
	}
}

static void writeblock(int fd, int block, uint8_t *ptr)
{
	blkhdr[2] = 3 + block;
	write(fd, blkhdr, 3);
	write(fd, ptr, 16384);
	printf("Bank %d signature %x %x %x\n",
		block, ptr[0],ptr[1],ptr[2]);
	
}

static void write_z80(char *name)
{
	int fd = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fd == -1) {
		perror(name);
		exit(1);
	}
	header[8] = 0xF0;		/* set SP somewhere */
	header[9] = 0xFF;
	
	header[29] = 0x40 | 1;		/* IM1, Kempston joystick */
	header[30] = 55;
	header[32] = 0x00;		/* Run from 0x0100 */
	header[33] = 0x01;
	header[34] = 7;			/* Spectrum +3 */
	header[35] = 0;			/* Screen at bank 5 */
	header[37] = 3;			/* Emulate LDIR and R details */
	header[61] = 0x00;		/* low memory is RAM */
	header[62] = 0x00;
	header[86] = 0x07;		/* Map 4/7/6/3 */
	write(fd, header, 87);		/* Header */

	writeblock(fd, 0, emptybank);
	writeblock(fd, 1, emptybank);
	writeblock(fd, 2, emptybank);
	writeblock(fd, 3, bank3);
	writeblock(fd, 4, bank4);
	writeblock(fd, 5, emptybank);
	writeblock(fd, 6, bank6);
	writeblock(fd, 7, bank7);
	close(fd);
}


int main(int argc, char *argv[])
{
	int fd;
	if (argc != 3) {
		fprintf(stderr, "%s: input.bin output.z80\n", argv[0]);
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	load_first_bank(bank4, fd);
	load_bank(bank7, fd);
	load_bank(bank6, fd);
	load_bank(bank3, fd);
	close(fd);
	write_z80(argv[2]);
	exit(0);
}
