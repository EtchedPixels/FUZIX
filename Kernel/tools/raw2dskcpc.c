#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Turn a raw image into a DSK file

   Our input is raw sector data single sided, our output is a 40 track
   spectrum +3 image we hope
   
   9 sectors per track MFM, 40 tracks, single side (flippy)

   We want this to work in the +3 with 3" drive - the 80 track and double
   side options only came in later add on drives */

int main(int argc, char *argv[])
{
    FILE *in, *out;
    static unsigned char buf[512 * 9];
    unsigned char *bp;
    int track, sector;

    strcpy(buf, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n");
    buf[0x30] = 40;
    buf[0x31] = 1;
    buf[0x32] = 0;
    buf[0x33] = 0x13;

    if (argc != 3) {
        fprintf(stderr, "%s: source dest.\n", argv[0]);
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
    for (track = 0; track < 40; track++) {
        /* Generate the track header */
        memset(buf, 0, 512);
        strcpy(buf, "Track-Info\r\n");
        buf[0x10] = track;
        buf[0x11] = 0;
        buf[0x14] = 2;
        buf[0x15] = 9;
        buf[0x16] = 0x4E;
        buf[0x17] = 0xE5;
        bp = buf + 0x18;
        for (sector = 1; sector < 10; sector++) {
            *bp++ = track;
            *bp++ = 0;
            *bp++ = 0x40+sector;
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
