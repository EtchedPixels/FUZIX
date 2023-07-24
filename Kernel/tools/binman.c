#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned char buf[65536];
static unsigned char out[65536];

static unsigned int s__CODE, s__CODE2, s__INITIALIZER, s__DATA,
    s__INITIALIZED, s__INITIALIZER, s__COMMONMEM, s__VIDEO, l__INITIALIZED,
    l__GSFINAL, l__GSINIT, l__COMMONMEM, s__FONT, l__FONT, s__DISCARD,
    l__DISCARD, l__CODE, l__CODE2, l__VIDEO, l__DATA, s__CONST, l__CONST,
    s__HEAP, l__HEAP, s__BOOT=0xFFFF, l__BOOT, s__PAGE0 = 0xFFFF,
    s__SYSMOD, l__SYSMOD, s__COMMONDATA, l__COMMONDATA;


static void ProcessMap(FILE * fp)
{
	char buf[512];

	while (fgets(buf, 511, fp)) {
		char *p1 = strtok(buf, " \t\n");
		char *p2 = NULL;

		if (p1)
			p2 = strtok(NULL, " \t\n");

		if (p1 == NULL || p2 == NULL)
			continue;

		if (strcmp(p2, "s__CODE") == 0)
			sscanf(p1, "%x", &s__CODE);
		if (strcmp(p2, "s__CODE2") == 0)
			sscanf(p1, "%x", &s__CODE2);
		if (strcmp(p2, "l__CODE") == 0)
			sscanf(p1, "%x", &l__CODE);
		if (strcmp(p2, "l__CODE2") == 0)
			sscanf(p1, "%x", &l__CODE2);
		if (strcmp(p2, "s__DISCARD") == 0)
			sscanf(p1, "%x", &s__DISCARD);
		if (strcmp(p2, "l__DISCARD") == 0)
			sscanf(p1, "%x", &l__DISCARD);
		if (strcmp(p2, "s__CONST") == 0)
			sscanf(p1, "%x", &s__CONST);
		if (strcmp(p2, "l__CONST") == 0)
			sscanf(p1, "%x", &l__CONST);
		if (strcmp(p2, "s__VIDEO") == 0)
			sscanf(p1, "%x", &s__VIDEO);
		if (strcmp(p2, "l__VIDEO") == 0)
			sscanf(p1, "%x", &l__VIDEO);
		if (strcmp(p2, "s__DATA") == 0)
			sscanf(p1, "%x", &s__DATA);
		if (strcmp(p2, "l__DATA") == 0)
			sscanf(p1, "%x", &l__DATA);
		if (strcmp(p2, "s__INITIALIZED") == 0)
			sscanf(p1, "%x", &s__INITIALIZED);
		if (strcmp(p2, "s__INITIALIZER") == 0)
			sscanf(p1, "%x", &s__INITIALIZER);
		if (strcmp(p2, "s__COMMONMEM") == 0)
			sscanf(p1, "%x", &s__COMMONMEM);
		if (strcmp(p2, "s__COMMONDATA") == 0)
			sscanf(p1, "%x", &s__COMMONDATA);
		if (strcmp(p2, "s__FONT") == 0)
			sscanf(p1, "%x", &s__FONT);
		if (strcmp(p2, "l__INITIALIZED") == 0)
			sscanf(p1, "%x", &l__INITIALIZED);
		if (strcmp(p2, "l__GSFINAL") == 0)
			sscanf(p1, "%x", &l__GSFINAL);
		if (strcmp(p2, "l__GSINIT") == 0)
			sscanf(p1, "%x", &l__GSINIT);
		if (strcmp(p2, "l__COMMONMEM") == 0)
			sscanf(p1, "%x", &l__COMMONMEM);
		if (strcmp(p2, "l__COMMONDATA") == 0)
			sscanf(p1, "%x", &l__COMMONDATA);
		if (strcmp(p2, "l__FONT") == 0)
			sscanf(p1, "%x", &l__FONT);
		if (strcmp(p2, "s__HEAP") == 0)
			sscanf(p1, "%x", &s__HEAP);
		if (strcmp(p2, "l__HEAP") == 0)
			sscanf(p1, "%x", &l__HEAP);
		if (strcmp(p2, "s__BOOT") == 0)
			sscanf(p1, "%x", &s__BOOT);
		if (strcmp(p2, "l__BOOT") == 0)
			sscanf(p1, "%x", &l__BOOT);
		if (strcmp(p2, "s__SYSMOD") == 0)
			sscanf(p1, "%x", &s__SYSMOD);
		if (strcmp(p2, "l__SYSMOD") == 0)
			sscanf(p1, "%x", &l__SYSMOD);
		if (strcmp(p2, "s__PAGE0") == 0)
			sscanf(p1, "%x", &s__PAGE0);
	}
}


int main(int argc, char *argv[])
{
	FILE *map, *bin;
	int tail = 0;
	int start;
	unsigned int end = 0;
	int reloc = 0;
	int pack_discard = 0;
	int no_pack = 0;
	int base;

	if (argc != 4) {
		fprintf(stderr, "%s: [binary] [map] [output]\n", argv[0]);
		exit(1);
	}
	bin = fopen(argv[1], "r");
	if (bin == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fread(buf, 1, 65536, bin) == 0) {
		fprintf(stderr, "%s: read error on %s\n", argv[0],
			argv[1]);
		exit(1);
	}
	fclose(bin);

        /* Start with the output matching the input */
	memcpy(out, buf, 65536);

	map = fopen(argv[2], "r");
	if (map == NULL) {
		perror(argv[2]);
		exit(1);
	}
	ProcessMap(map);
	fclose(map);

	if (s__COMMONMEM > 0xFFFF || s__COMMONMEM + l__COMMONMEM + l__COMMONDATA > 0x10000) {
		fprintf(stderr, "Move common down by at least %d bytes\n",
			s__COMMONMEM + l__COMMONMEM + l__COMMONDATA - 0x10000);
		exit(1);
	}

	/* linker will allow us to overlap _DISCARD (which may grow)
	   with _COMMONMEM. Don't however worry if DISCARD is above common
	   as can happen in some odder layouts */
	if(s__DISCARD && s__DISCARD+l__DISCARD > s__COMMONMEM &&
		s__DISCARD < s__COMMONMEM + l__COMMONMEM) {
		fprintf(stderr, "Move _DISCARD down by at least %d bytes\n",
			s__DISCARD + l__DISCARD - s__COMMONMEM);
		exit(1);
	}
	/* If we have a discard area that overlaps the initializer block
	   then the compiler will have messed it up. Even though we then
	   copy the initializers into initialized we can only use the space
	   for bss */
	if (s__DISCARD && s__DISCARD + l__DISCARD > s__INITIALIZER &&
		s__INITIALIZER + l__INITIALIZED > s__DISCARD) {
		fprintf(stderr, "Initializer will have overwritten DISCARD\n");
		exit(1);
	}

        printf("Scanning data from 0x%x to 0x%x\n",
                s__DATA, s__DATA + l__DATA);
	base = s__DATA;
	while (base < s__DATA + l__DATA) {
	        if (buf[base] && buf[base] != 0xFF) {
	                fprintf(stderr, "0x%04x:0x%02x\n",
	                        base, (unsigned char)buf[base]);
                }
                base++;
        }

	/* Our standard layout begins with the code */
	start = s__CODE;

	/* Special case for 32K/32K layouts. We have a high and a low page
	   and we need to pack the entire binary space. We can revist this
	   later but for now it's good enough. Also used as a flag by
	   other ports to tell us to leave them alone */
	if (s__PAGE0 != 0xFFFF) {
		pack_discard = 0;
		start = 0;
		end = 0xFFFF;
		no_pack = 1;
	}

	/* Some kernels need a special boot area and it may be before the
	   CODE block */
	if (s__BOOT < start)
		start = s__BOOT;

	/* TODO: Support a proper discardable high discard in other mappings */

	/* In an environment with a single process mapped we put the discard
	   area into process space, so it will be below the kernel in most
	   cases. In that case we start on the DISCARD segment if need be */
	/* Low discard (pure swap model) */
	if (s__DISCARD && s__DISCARD < start) {
		start = s__DISCARD;
		if (s__DISCARD + l__DISCARD > s__CODE) {
			fprintf(stderr,
				"Move discard down by at least %d bytes\n",
				s__DISCARD + l__DISCARD - s__CODE);
			exit(1);
		}
		printf("Discard at %04x (%d bytes), space %d\n",
		       s__DISCARD, l__DISCARD,
		       s__CODE - s__DISCARD - l__DISCARD);
	} else if (s__DISCARD && s__DISCARD < s__COMMONMEM) {
		/* Discard may also be set up high below the common */
	        pack_discard = 1;
        }

	if (end < s__INITIALIZER)
		end = s__INITIALIZER;
	if (end < start) {
		/* We are dealing with an unpacked binary where the end
		   will instead be _HEAP */
		end = s__HEAP;
	}
	if (end < s__DATA) {
		/* Image with data highest */
		end  = s__DATA;
		pack_discard = 0;
	}

	if (s__CODE2 < 0x10000) {
		/* Move the initialized data into the right spot rather than sdcc's
		   assumption of being ROM code */
		memcpy(out + s__INITIALIZED, buf + s__INITIALIZER,
		       l__INITIALIZED);
		if (l__GSFINAL || l__GSINIT)
			fprintf(stderr, "Warning %s contains gs code.\n",
				argv[1]);
		/* Pack any common memory on the end of the main code/memory if its
		   relocated */
		if (!no_pack && (!s__DISCARD || pack_discard)) {
		        base = s__DATA;
		        tail = l__SYSMOD;
		        if (tail) {
				memcpy(out + base, buf + s__SYSMOD, l__SYSMOD);
				base += l__SYSMOD;
			}
			printf("Common packed at 0x%04X\n", base);
			tail += l__COMMONMEM + l__COMMONDATA;
			memcpy(out + base, buf + s__COMMONMEM,
			       l__COMMONMEM + l__COMMONDATA);
			base += l__COMMONMEM + l__COMMONDATA;
			/* If we have the font packed high then add it to the image */
			if (l__FONT && s__FONT > end) {
				printf("Font packed at 0x%04X\n", base);
				memcpy(out + base, buf + s__FONT, l__FONT);
				tail += l__FONT;
				base += l__FONT;
			}
			end = s__COMMONMEM + l__COMMONMEM + l__COMMONDATA + l__FONT;
			if (pack_discard) {
				printf("Discard packed at 0x%04X\n", base);
			        memcpy(out + base, buf + s__DISCARD, l__DISCARD);
			        base += l__DISCARD;
			        tail += l__DISCARD;
			        end += l__DISCARD;
                        }
			reloc = 1;
		}

		printf("Code at 0x%04x (%d bytes)\n", s__CODE, l__CODE);
		printf("Code2 at 0x%04x (%d bytes)\n", s__CODE2, l__CODE2);
		printf("Const at 0x%04x (%d bytes)\n", s__CONST, l__CONST);
		printf("Data at 0x%04x (%d bytes)\n", s__DATA,
		       l__DATA + l__INITIALIZED);
		printf("Common at 0x%04x (%d + %d bytes)\n", s__COMMONMEM,
		       l__COMMONMEM,  l__COMMONDATA);
		if (l__FONT)
			printf("Font at 0x%04x (%d bytes)\n", s__FONT,
			       l__FONT);
		if (l__VIDEO)
			printf("Video at 0x%04x (%d bytes)\n", s__VIDEO,
			       l__VIDEO);
		printf("Discard at 0x%04x (%d bytes)\n", s__DISCARD,
		       l__DISCARD);
		if (s__DATA + l__DATA > s__COMMONMEM &&
		        s__DATA < s__COMMONMEM) {
		        fprintf(stderr, "Data overlaps common by %d bytes.\n",
		                s__DATA + l__DATA - s__COMMONMEM);
                        exit(1);
                }
		if (s__CONST + l__CONST > s__DATA &&
		        s__CONST < s__DATA) {
		        fprintf(stderr, "Const overlaps data by %d bytes.\n",
		                s__CONST + l__CONST - s__DATA);
                        exit(1);
                }
		/* For a complex image we have the initialized data last */
		if (end < s__INITIALIZED +  l__INITIALIZED)
		        end = s__INITIALIZED + l__INITIALIZED;
                /* Common may follow but only if we didn't relocate it */
		if (reloc == 0 && end < s__COMMONMEM +  l__COMMONMEM + l__COMMONDATA)
		        end = s__COMMONMEM + l__COMMONMEM + l__COMMONDATA;

                /* Packed image with common over data */
		if (!no_pack && (!s__DISCARD || pack_discard)) {
			end = base;
			printf("\nPacked image %d bytes, for RAM target\n",
			       end);
		}
		else
		        printf("End at 0x%04x\n", end);
	} else {
		printf("ROM target: leaving\n");
		exit(0);
	}
	bin = fopen(argv[3], "w");
	if (bin == NULL) {
		perror(argv[3]);
		exit(1);
	}
	printf("Image file: %04X to %04X\n", start, end);
	if (fwrite(out + start, end - start + 1, 1, bin) !=
	    1) {
		perror(argv[3]);
		exit(1);
	}
	fclose(bin);
	exit(0);
}

