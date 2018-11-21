/*
 *	Convert a banked FUZIX spectrum binary into a .Z80 file for ease
 *	of testing.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

static uint8_t emptybank[16384];
static uint8_t bank0[16384];
static uint8_t bank1[16384];
static uint8_t bank2[16384];
static uint8_t bank5[16384];
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

static void load_bank_at(uint8_t *p, char *name, int offset)
{
	int fd = open(name, O_RDONLY);
	if (fd == -1) {
		perror(name);
		exit(1);
	}
	if (lseek(fd, offset, SEEK_SET) != offset) {
		perror("lseek");
		exit(1);
	}
	if (read(fd, p, 16384) < 1) {
		perror(name);
		exit(1);
	}
	close(fd);
}

static void load_bank(uint8_t *p, char *name)
{
	load_bank_at(p, name, 0xC000);
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
	header[30] = 54;
	header[32] = 0x00;		/* Run from C000 */
	header[33] = 0xC0;
	header[34] = 4;			/* 128K */
	header[37] = 3;			/* Emulate LDIR and R details */
	header[61] = 0xFF;		/* low memory is ROM */
	header[62] = 0xFF;
	write(fd, header, 86);		/* Header */

	writeblock(fd, 0, bank0);
	writeblock(fd, 1, bank1);
	writeblock(fd, 2, bank5);
	writeblock(fd, 3, emptybank);
	writeblock(fd, 4, emptybank);
	writeblock(fd, 5, bank2);
	writeblock(fd, 6, emptybank);
	writeblock(fd, 7, bank7);
	close(fd);
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "%s: file.z80\n", argv[0]);
		exit(1);
	}
	load_bank_at(bank5, "common.bin", 0x8000);
	load_bank_at(bank2, "common.bin", 0x4000);
	load_bank(bank0, "common.bin");
	load_bank(bank1, "bank2.bin");
	load_bank(bank7, "bank3.bin");
	write_z80(argv[1]);
	exit(0);
}
