#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

int main(int argc, const char* argv[])
{
	FILE* inf = fopen(argv[1], "rb");
	FILE* outf = fopen(argv[2], "wb");

	uint16_t blockno = 0;
	while (!feof(inf))
	{
		uint8_t buffer[512*7] = {};
		buffer[4] = blockno & 0xff;
		buffer[5] = blockno >> 8;

		fwrite(buffer, 512, 1, outf);
		fread(buffer, 512, 7, inf);
		fwrite(buffer, 512, 7, outf);
		blockno++;
	}

	for (int i=0; i<4096; i++)
		fputc(0xff, outf);

	fclose(inf);
	fclose(outf);
	return 0;
}

