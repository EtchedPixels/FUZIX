#include <stdio.h>
#include <stdlib.h>

/* Turn a raw image into an MGT file. Our input is ordered head/track/sector
   and we want it track/head/sector - which is a pain to work with using dd
   hence we post convert */

int main(int argc, char *argv[])
{
    FILE *in, *out;
    unsigned char buf[5120];
    int track;

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

    for (track = 0; track < 80; track++) {
        if (fseek(in, track * 5120L, 0) == -1) {
            perror(argv[1]);
            exit(1);
        }
        if (fread(buf, 5120, 1, in) != 1) {
            perror(argv[1]);
            exit(1);
        }
        if (fwrite(buf, 5120, 1, out) != 1) {
            perror(argv[2]);
            exit(1);
        }
        if (fseek(in, (80 + track) * 5120L, 0) == -1) {
            perror(argv[1]);
            exit(1);
        }
        if (fread(buf, 5120, 1, in) != 1) {
            perror(argv[1]);
            exit(1);
        }
        if (fwrite(buf, 5120, 1, out) != 1) {
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
