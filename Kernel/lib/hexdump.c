#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/* Displays a hex dump in classic form. Useful for debugging. */

void hexdump(const void* data, size_t size)
{
	int i;
	size_t pos = 0;

	while (pos < size)
	{
		kputhex(i+pos);
		kputs(" : ");
		for (i=0; i<16; i++)
		{
			if ((pos+i) < size)
			{
				uint8_t c = ((uint8_t*)data)[i+pos];
				kputhexbyte(c);
			}
			else
				kputs("  ");
			kputchar(' ');
		}

		kputs(" : ");
		for (i=0; i<16; i++)
		{
			if ((pos+i) < size)
			{
				uint8_t c = ((uint8_t*)data)[i+pos];
				if ((c >= 32) && (c <= 126))
					kputchar(c);
				else
					kputchar('.');
			}
			else
				kputchar(' ');
		}

		pos += 16;
		kputchar('\n');
	}
}

