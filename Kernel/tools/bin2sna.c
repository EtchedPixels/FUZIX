/*
 *	Convert a banked FUZIX spectrum binary into a .SNA file for ease
 *	of testing.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

static uint8_t zeroes[27];
static uint8_t emptybank[16384];
static uint8_t bank0[16384];
static uint8_t bank1[16384];
static uint8_t bank2[16384];
static uint8_t bank5[16384];
static uint8_t bank7[16384];
static uint8_t pc[2] = { 0x03, 0xC0 };	/* kick off at 0xC003 in bank 0 */
static uint8_t bank = 24;	/* Screen high, 48K ROM, bank 0 */

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


static void write_sna(char *name)
{
	int fd = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fd == -1) {
		perror(name);
		exit(1);
	}
	write(fd, zeroes, 27);		/* Header */
	write(fd, bank2, 16384);	/* 0x4000-0x7FFF */
	write(fd, bank5, 16384);	/* 0x8000-0xBFFF */
	write(fd, bank0, 16384); 	/* 0xC000 */
	write(fd, pc, 2);		/* Jump to 0xC000 */
	write(fd, &bank, 1);		/* Bank 0, screen in 7, 48K ROM */
	write(fd, zeroes, 1);		/* No TR-DOS */
	write(fd, bank1, 16384);	/* Bank 1 */
	/* Bank 2 is at 0x4000 */
	write(fd, emptybank, 16384);	/* Bank 3 (will be user) */
	write(fd, emptybank, 16384);	/* Bank 4 (will be user) */
	/* Bank 5 is at 0x8000 */
	write(fd, emptybank, 16384);	/* Bank 6 (will be user) */
	write(fd, bank7, 16384);	/* Video, fonts, etc */
	close(fd);
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "%s: file.sna\n", argv[0]);
		exit(1);
	}
	load_bank_at(bank5, "common.bin", 0x8000);
	load_bank_at(bank2, "common.bin", 0x4000);
	load_bank(bank0, "common.bin");
	load_bank(bank1, "bank2.bin");
	load_bank(bank7, "bank3.bin");
	write_sna(argv[1]);
	exit(0);
}