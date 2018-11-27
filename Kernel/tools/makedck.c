/*
 *	Make a DCK file of the top 32K of a 64K image
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    FILE *in, *out;
    char buf[32768];
    if (argc != 3) {
        fprintf(stderr, "%s image dockfile.\n", argv[0]);
        exit(1);
    }
    in = fopen(argv[1], "r");
    if (in == NULL) {
        perror(argv[1]);
        exit(1);
    }
    out = fopen(argv[2], "w");
    if (out == NULL) {
        perror(argv[2]);
        exit(1);
    }
    fputc(0, out);		/* Dock */
    fputc(0x1, out);		/* Low 32K is RAM, data absent */
    fputc(0x1, out);
    fputc(0x1, out);
    fputc(0x1, out);
    fputc(0x2, out);		/* Next 32K is ROM, data present */
    fputc(0x2, out);
    fputc(0x2, out);
    fputc(0x2, out);

    /* Now the data */
    if (fread(buf, 32768, 1, in) != 1 ||
        fread(buf, 32768, 1, in) != 1) {
            fprintf(stderr, "%s: image '%s' is too short.\n",
                argv[0], argv[1]);
            exit(1);
    }
    if (fwrite(buf, 32768,1, out) != 1 || fclose(out)) {
        fprintf(stderr, "%s: short write to '%s'.\n", argv[0], argv[2]);
        exit(1);
    }
    return 0;
}
        