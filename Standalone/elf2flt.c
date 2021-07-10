#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

typedef uint32_t uaddr_t;
#include "../Kernel/include/elf.h"
#include "../Kernel/include/flat.h"

#define perror_exit(msg) \
	do { perror(msg); exit(1); } while (0)
#define alignup(v,a) \
	(uint32_t)((intptr_t)((v) + (a)-1) & ~((a)-1))

#define ELFSTRUCT(offset) \
	(void*)((uint8_t*)elffile + offset)

static int stacksize = 4096;

static void syntax_error(void)
{
	fprintf(stderr, "syntax: elf2flt [-s stacksize] -o outputfile inputfile\n");
	exit(1);
}

static void write_error(void)
{
	perror_exit("cannot write output file");
}

void* load_file(const char* filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
		perror_exit("cannot open input file");
	
	struct stat st;
	if (fstat(fd, &st) == -1)
		perror_exit("cannot stat input file");

	void* ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	if (ptr == MAP_FAILED)
		perror_exit("cannot map input file");

	return ptr;
}

int main(int argc, char* const* argv)
{
	const char* inputfilename = NULL;
	const char* outputfilename = NULL;
	for (;;)
	{
		int opt = getopt(argc, argv, "s:o:");
		if (opt == -1)
			break;

		switch (opt)
		{
			case 'o':
				outputfilename = optarg;
				break;

			case 's':
				stacksize = strtoul(optarg, NULL, 0);
				break;

			default:
				syntax_error();
		}
	}

	if (!outputfilename
			|| ((optind+1) != argc))
		syntax_error();
	inputfilename = argv[optind];

	Elf32_Ehdr* elffile = load_file(inputfilename);
	FILE* outfp = fopen(outputfilename, "wb");
	if (!outfp)
		perror_exit("cannot open output file");

	/* Scan the section headers looking for stuff. */

	Elf32_Shdr* shdr = ELFSTRUCT(elffile->e_shoff);
	uint32_t bsshi = 0;
	uint32_t bsslo = 0xffffffff;
	uint32_t datahi = 0;
	uint32_t datalo = 0xffffffff;
	uint32_t texthi = 0;
	uint32_t textlo = 0xffffffff;
	uint32_t reloff = 0xffffffff;
	uint32_t relcount = 0;
	for (int i=0; i<elffile->e_shnum; i++)
	{
		Elf32_Shdr* sh = &shdr[i];
		uint32_t seclo = sh->sh_addr;
		uint32_t sechi = sh->sh_addr + sh->sh_size;
		switch (sh->sh_type)
		{
			case SHT_PROGBITS: /* Initialised data */
				if (sh->sh_flags & SHF_ALLOC)
				{
					if (sh->sh_flags & SHF_EXECINSTR)
					{
						/* Text segment. */

						if (seclo < textlo)
							textlo = seclo;
						if (sechi > texthi)
							texthi = sechi;
					}
					else
					{
						/* Data segment. */

						if (seclo < datalo)
							datalo = seclo;
						if (sechi > datahi)
							datahi = sechi;
					}
				}
				break;

			case SHT_NOBITS: /* Zero-initialised data */
				if (sh->sh_flags & SHF_ALLOC)
				{
					/* BSS segment. */

					if (seclo < bsslo)
						bsslo = seclo;
					if (sechi > bsshi)
						bsshi = sechi;
				}
				break;

			case SHT_REL: /* Relocation section */
				if (relcount != 0)
				{
					fprintf(stderr, "failed: multiple relocation sections\n");
					exit(1);
				}
				reloff = sh->sh_offset;
				relcount = sh->sh_size / sizeof(Elf32_Rel);
				break;
		}
	}

	printf("text:  0x%08x to 0x%08x\n", textlo, texthi);
	printf("data:  0x%08x to 0x%08x\n", datalo, datahi);
	printf("bss:   0x%08x to 0x%08x\n", bsslo, bsshi);
	printf("reloc: %d entries\n", relcount);

	if ((bsslo < texthi) || (bsslo < datahi) || (datalo < texthi))
	{
		fprintf(stderr, "failed: overlapping segments (ELF file is too complex?)\n");
		exit(1);
	}

	/* Now, load all the PROGBITS segments into memory. */

	uint8_t* memory = calloc(1, datahi - textlo);
	for (int i=0; i<elffile->e_shnum; i++)
	{
		Elf32_Shdr* sh = &shdr[i];
		switch (sh->sh_type)
		{
			case SHT_PROGBITS: /* Initialised data */
				if (sh->sh_flags & SHF_ALLOC)
					memcpy(memory - textlo + sh->sh_addr, ELFSTRUCT(sh->sh_offset), sh->sh_size);
				break;
		}
	}

	/* Assemble the bFLT header. */

	struct binfmt_flat flatheader =
	{
		.magic = FLAT_FUZIX_MAGIC,
		.rev = htonl(FLAT_VERSION),
		.flags = htonl(FLAT_FLAG_RAM),
		.entry = htonl(elffile->e_entry + sizeof(flatheader)),
		.data_start = htonl(datalo + sizeof(flatheader)),
		.data_end = htonl(datahi + sizeof(flatheader)),
		.bss_end = htonl(bsshi + sizeof(flatheader)),
		.reloc_start = htonl(datahi + sizeof(flatheader)),
		.reloc_count = htonl(relcount),
		.stack_size = htonl(stacksize),
	};
	if (fwrite(&flatheader, sizeof(flatheader), 1, outfp) != 1)
		write_error();
	if (fwrite(memory, datahi, 1, outfp) != 1)
		write_error();

	Elf32_Rel* rel = ELFSTRUCT(reloff);
	for (int i=0; i<relcount; i++)
	{
		uint32_t type = ELF32_R_TYPE(rel->r_info);
		uint32_t addr;
		switch (type)
		{
			case R_ARM_RELATIVE:
				addr = rel->r_offset;
				break;

			default:
				fprintf(stderr, "Unknown ELF relocation type %d\n", type);
				exit(1);
		}

		addr = htonl(addr);
		if (fwrite(&addr, sizeof(addr), 1, outfp) != 1)
			write_error();

		rel++;
	}
	fclose(outfp);
}

