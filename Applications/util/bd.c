/**************************************************
UZI (Unix Z80 Implementation) Utilities:  bd.c
***************************************************/
/*
 *  Block Dump - to examine floppy and hard disks.
 *
 *  Usage:  bd dev blkno
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

char buf[512];

int main(int argc, char *argv[])
{
    int  i, j, d;
    unsigned blkno;
    char ch;

    if (argc != 3 || !isdigit(argv[2][0])) {
	fprintf(stderr, "usage: bd device blkno\n");
	exit(1);
    }

    blkno = atoi(argv[2]);
    d = open(argv[1], O_RDONLY);
    if (d < 0) {
        printf("bd: can't open %s\n", argv[1]);
        exit(1);
    }

    lseek(d, blkno * 512L, 0);
    i = read(d, buf, 512);
    close(d);
    
    if (i != 512) {
        printf("bd: error reading block %d\n", blkno);
        exit(1);
    }

    for (i = 0; i < 512 / 16; ++i) {
	printf("%04x  ", 16 * i);
	for (j = 0; j < 16; ++j) {
	    printf("%02x ", buf[16 * i + j] & 0xff);
	}
	printf("    ");
	for (j = 0; j < 16; ++j) {
	    ch = (buf[16 * i + j] & 0x00ff);
	    if ((ch >= ' ') && (ch < 0x7f))
		putchar(ch);
	    else
		printf(".");
	}
	printf("\n");
	if (((i + 1) % 16 == 0) && (i < 31)) {
	    printf("[return for more]");
	    getchar();
	    printf("\n");
	}
    }
    exit(0);
}
