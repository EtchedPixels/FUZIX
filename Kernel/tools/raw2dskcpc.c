#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Turn a raw image into a DSK file

raw2dskcpc input.raw output.raw tracks sides sector_id_offset
  
   9 sectors per track MFM, 40/80 tracks, single/double side (floppy)
 */

int main(int argc, char *argv[])
{
    FILE *in, *out;
    static unsigned char buf[512 * 9];
    unsigned char *bp;
    int track, sector, side;
    int tracks = atoi(argv[3]);
    int sides = atoi(argv[4]);
    int sector_id_offset = atoi(argv[5]);

    strcpy(buf, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n");
    buf[0x30] = tracks;
    buf[0x31] = sides;
    buf[0x32] = 0;
    buf[0x33] = 0x13;

    if (argc != 6) {
        fprintf(stderr, "%s: source dest tracks sides sector_id_offset.\n", argv[0]);
        exit(1);
    }

    in = fopen(argv[1], "r");
    if (in == NULL) {
        perror(argv[1]);
        exit(1);
    }
    out =  fopen(argv[2], "w");
    if (out == NULL) {
        perror(argv[2]);
        exit(1);
    }

    /* Write the header */
    if (fwrite(buf, 256, 1, out) != 1) {
        perror(argv[2]);
        exit(1);
    }    
    for (track = 0; track < tracks; track++) {
        for (side = 0; side < sides; side++){ /* Generate the track header */
            memset(buf, 0, 512);
            strcpy(buf, "Track-Info\r\n");
            buf[0x10] = track;
            buf[0x11] = side;
            buf[0x14] = 2;
            buf[0x15] = 9;
            buf[0x16] = 0x2A;
            buf[0x17] = 0xE5;
            bp = buf + 0x18;
            for (sector = 1; sector < 10; sector++) {
                *bp++ = track;
                *bp++ = side;
                *bp++ = sector_id_offset+sector;
                *bp++ = 2;
                *bp++ = 0;
                *bp++ = 0;
                *bp++ = 0;
                *bp++ = 0;
            }
            if (fwrite(buf, 256, 1, out) != 1) {
                perror(argv[2]);
                exit(1);
            }
            if (fread(buf, 512, 9, in) != 9) {
                perror(argv[1]);
                exit(1);
            }
            if (fwrite(buf, 512, 9, out) != 9) {
                perror(argv[2]);
                exit(1);
            }
        }
    }
    if (fclose(in)) {
        perror(argv[1]);
        exit(1);
    }
    if (fclose(out)) {
        perror(argv[2]);
        exit(1);
    }
    exit(0);
}
