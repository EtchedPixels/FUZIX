/*
 *  MIT License
 *
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE
 */

#include <stdio.h>
#include <string.h>
#include "uf2format.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "USAGE: %s file.bin file.uf2 address\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "No such file: %s\n", argv[1]);
        return 1;
    }

    fseek(f, 0L, SEEK_END);
    uint32_t sz = ftell(f);
    fseek(f, 0L, SEEK_SET);

    const char *outname = argc > 2 ? argv[2] : "flash.uf2";

    FILE *fout = fopen(argv[2], "wb");
	uint32_t address = strtoul(argv[3], NULL, 0);

    UF2_Block bl;
    memset(&bl, 0, sizeof(bl));

    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.magicEnd = UF2_MAGIC_END;
    bl.flags = UF2_FLAG_FAMILY_ID_PRESENT;
    bl.fileSize = RP2040_FAMILY_ID;
    bl.targetAddr = address;
    bl.numBlocks = (sz + 255) / 256;
    bl.payloadSize = 256;
    int numbl = 0;
    while (fread(bl.data, 1, bl.payloadSize, f)) {
        bl.blockNo = numbl++;
        fwrite(&bl, 1, sizeof(bl), fout);
        bl.targetAddr += bl.payloadSize;
        // clear for next iteration, in case we get a short read
        memset(bl.data, 0, sizeof(bl.data));
    }
    fclose(fout);
    fclose(f);
    printf("Wrote %d blocks to %s\n", numbl, outname);
    return 0;
}
