/*
 *	The OSIHD format is each sector as a 4K block with pre-postamble
 *
 *	This at the moment only knows how to make a CD-36 format
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

unsigned char buf[4096];

static uint8_t checksum(uint8_t *p, unsigned n)
{
	uint16_t sum = 0;
	while (n--) {
		sum += *p++;
#ifdef OLD_CSUM
		if (sum & 0x0100) {
		      sum &= 0xFF:
			sum++;
		}
#endif
	}
	return sum;
}

static uint16_t checksum16(uint8_t *p, unsigned n)
{
	uint16_t sum = 0;
	while (n--)
		sum += *p++;
	return sum;
}


int main(int argc, char *argv[])
{
    unsigned sec = 0;
    unsigned track = 0;
    unsigned head = 0;
    unsigned sum;

    unsigned max_head;
    unsigned max_track;

    if (argc != 2) {
        fprintf(stderr, "%s type < raw > disk.hd\n", argv[0]);
        exit(1);
    }
    if (strcmp(argv[1], "cd-36") == 0) {
        max_track = 339;
        max_head = 6;
    } else if (strcmp(argv[1], "cd-74") == 0) {
        max_track = 339;
        max_head = 12;
    } else {
        fprintf(stderr, "%s: only cd-36 and cd-74 types supported.\n", argv[0]);
        exit(1);
    }
    while(track <= max_track) {
        read(0, buf + 24, 3584);
        /* Investigate low 16 bytes */
        buf[16] = 0xA1;
        buf[17] = 0x00;
        buf[18] = 0x00;
        buf[19] = track;
        buf[20] = head;
        buf[21] = sec;
        buf[22] = 0x00;
        buf[23] = checksum(buf + 18, 5);
        sum = checksum16(buf + 0x18, 3584);
        buf[0xE18] = sum;
        buf[0xE19] = sum >> 8;
        write(1, buf, 4096);
        sec++;
        if (sec == 5) {
            sec = 0;
            head++;
            if (head == max_head) {
                head = 0;
                track++;
            }
        }
    }
    return 0;
}
