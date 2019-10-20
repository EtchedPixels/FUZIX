/*
 *	Ld symbol loading support. For the moment we don't do anything clever
 *	to avoid memory wastage.
 *
 *	Theory of operation
 *
 *	We scan the header and load the symbols (non debug) for each object
 *	We then compute the total size of each segment
 *	We calculate the base address of each object file code/data/bss
 *	We write a dummy executable header
 *	We relocate each object code and write all the code to the file
 *	We do the same with the data
 *	We set up the header and bss sizes
 *	We write out symbols (optional)
 *	We write the header back
 *
 *	The relocation can be one of three forms eventually
 *	ld -r:
 *		We write the entire object out as one .o file with all the
 *		internal references resolved and all the symbols adjusted
 *		versus that. Undefined symbols are allowed and carried over
 *	a.out (or similar format)
 *		We resolve the entire object as above but write out with a
 *		binary header. No undefined symbols are allowed
 *	bin:
 *		We resolve the entire object and perform all relocations
 *		to generate a binary with a fixed load address. No undefined
 *		symbols or relocations are left
 *
 *	There are a few things not yet addressed
 *	1.	For speed libraries can stat with an _RANLIB ar file node whic
 *		is an index of all the symbols by library module for speed.
 *	2.	Testing bigendian support.
 *	3.	Banked binaries (segments 5-7 ?).
 *	4.	Use typedefs and the like to support 32bit as well as 16bit
 *		addresses when built on bigger machines..
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>
#include <ar.h>

#include "obj.h"
#include "ld.h"

static char *arg0;			/* Command name */
static struct object *processing;	/* Object being processed */
static struct object *objects, *otail;	/* List of objects */
static struct symbol *symhash[NHASH];	/* Symbol has tables */
static uint16_t base[4];		/* Base of each segment */
static uint16_t size[4];		/* Size of each segment */
static uint16_t align;			/* Alignment */

#define LD_RELOC	0		/* Output a relocatable binary */
#define LD_RFLAG	1		/* Output an object module */
#define LD_ABSOLUTE	2		/* Output a linked binary */
static uint8_t ldmode = LD_RELOC;	/* Operating mode */

static uint8_t split_id;		/* True if code and data both zero based */
static uint8_t arch;			/* Architecture */
static uint16_t arch_flags;		/* Architecture specific flags */
static uint8_t verbose;			/* Verbose reporting */
static int err;				/* Error tracking */
static int dbgsyms = 1;			/* Set to dumb debug symbols */
static int strip = 0;			/* Set to strip symbols */
static int obj_flags;			/* Object module flags for compat */
static const char *mapname;		/* Name of map file to write */
static const char *outname;		/* Name of output file */
static uint16_t dot;			/* Working address as we link */

static uint8_t progress;		/* Did we make forward progress ?
					   Used while library linking */

/*
 *	Report an error, and if possible give the object or library that
 *	we were processing.
 */
static void error(const char *p)
{
	if (processing)
		fprintf(stderr, "While processing: %s\n", processing->path);
	fputs(p, stderr);
	fputc('\n', stderr);
	err |= 2;

	exit(err | 2);
}

/*
 *	Standard routines wrapped with error exit tests
 */
static void *xmalloc(size_t s)
{
	void *p = malloc(s);
	if (p == NULL)
		error("out of memory");
	return p;
}

static FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (fp == NULL) {
		perror(path);
		exit(err | 1);
	}
	return fp;
}

static void xfclose(FILE *fp)
{
	if (fclose(fp)) {
		perror("fclose");
		exit(err | 1);
	}
}

static void xfseek(FILE *fp, off_t pos)
{
	if (fseek(fp, pos, SEEK_SET) < 0) {
		perror("fseek");
		exit(err | 1);
	}
}

/*
 *	Manage the linked list of object files and object modules within
 *	libraries that we have seen.
 */
static struct object *new_object(void)
{
	struct object *o = xmalloc(sizeof(struct object));
	o->next = NULL;
	o->syment = NULL;
	return o;
}

static void insert_object(struct object *o)
{
	if (otail)
		otail->next = o;
	else
		objects = o;
	otail = o;
}

static void free_object(struct object *o)
{
	if (o->syment)
		free(o->syment);
	free(o);
}

/*
 *	Add a symbol to our symbol tables as we discover it. Log the
 *	fact if tracing.
 */
struct symbol *new_symbol(const char *name, int hash)
{
	struct symbol *s = xmalloc(sizeof(struct symbol));
	strncpy(s->name, name, 16);
	s->next = symhash[hash];
	symhash[hash] = s;
	if (verbose)
		printf("+%.16s\n", name);
	return s;
}

/*
 *	Find a symbol in a given has table	
 */
struct symbol *find_symbol(const char *name, int hash)
{
	struct symbol *s = symhash[hash];
	while (s) {
		if (strncmp(s->name, name, 16) == 0)
			return s;
		s = s->next;
	}
	return NULL;
}

/*
 *	A simple but adequate hashing algorithm. A better one would
 *	be worth it for performance.
 */
static uint8_t hash_symbol(const char *name)
{
	int hash = 0;
	uint8_t n = 0;

	while(*name && n++ < 16)
		hash += *name++;
	return (hash&(NHASH-1));
}

/*
 *	Check if a symbol name is known but undefined. We use this to decide
 *	whether to incorporate a library module
 */
static int is_undefined(const char *name)
{
	int hash = hash_symbol(name);
	struct symbol *s = find_symbol(name, hash);
	if (s == NULL || !(s->type & S_UNKNOWN))
		return 0;
	/* This is a symbol we need */
	progress = 1;
	return 1;
}

/*
 *	Check that two versions of a symbol are compatible.
 */
static void segment_mismatch(struct symbol *s, uint8_t type2)
{
	uint8_t seg1 = s->type & S_SEGMENT;
	uint8_t seg2 = type2 & S_SEGMENT;

	/* Matching */
	if (seg1 == seg2)
		return;
	/* Existing entry was 'anything'. Co-erce to definition */
	if (seg1 == S_ANY) {
		s->type &= ~S_SEGMENT;
		s->type |= seg2;
		return;
	}
	/* Regardless of the claimed type, an absolute definition fulfills
	   any need. */
	if (seg2 == ABSOLUTE || seg2 == S_ANY)
		return;
	fprintf(stderr, "Segment mismatch for symbol '%.16s'.\n", s->name);
	fprintf(stderr, "Want segment %d but constrained to %d.\n",
		seg2, seg1);
	err |= 2;
}

/*
 *	We have learned about a new symbol. Find the symbol if it exists, or
 *	create it if not. Do the necessary work to promote unknown symbols
 *	to known and also to ensure we don't have multiple incompatible
 *	definitions or have incompatible definition and requirement.
 */
static struct symbol *find_alloc_symbol(struct object *o, uint8_t type, const char *id, uint16_t value)
{
	uint8_t hash = hash_symbol(id);
	struct symbol *s = find_symbol(id, hash);

	if (s == NULL) {
		s = new_symbol(id, hash);
		s->type = type;
/*FIXME         strlcpy(s->name, id, 16); */
		strncpy(s->name, id, 16);
		s->value = value;
		if (!(type & S_UNKNOWN))
			s->definedby = o;
		else
			s->definedby = NULL;
		return s;
	}
	/* Already exists. See what is going on */
	if (type & S_UNKNOWN) {
		/* We are an external reference to a symbol. No work needed */
		segment_mismatch(s, type);
		return s;
	}
	if (s->type & S_UNKNOWN) {
		/* We are referencing a symbol that was previously unknown but which
		   we define. Fill in the details */
		segment_mismatch(s, type);
		s->type &= ~S_UNKNOWN;
		s->value = value;
		s->definedby = o;
		return s;
	}
	/* Two definitions.. usually bad but allow duplicate absolutes */
	if (((s->type | type) & S_SEGMENT) != ABSOLUTE || s->value != value) {
		/* FIXME: expand to report files somehow ? */
		fprintf(stderr, "%s: multiply defined.\n", id);
	}
	/* Duplicate absolutes - just keep current entry */
	return s;
}

/*
 *	Add the internal symbols indicating where the segments start and
 *	end.
 */
static void insert_internal_symbol(const char *name, int seg, uint16_t val)
{
	if (seg == -1)
		find_alloc_symbol(NULL, S_ANY | S_UNKNOWN, name, 0);
	else
		find_alloc_symbol(NULL, seg | S_PUBLIC, name, val);
}

/*
 *	Number the symbols that we will write out in the order they will
 *	appear in the output file. We don't care about the mode too much.
 *	In valid reloc mode we won't have any S_UNKNOWN symbols anyway
 */
static void renumber_symbols(void)
{
	static int sym = 0;
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++)
		for (s = symhash[i]; s != NULL; s=s->next)
			if (s->type & (S_PUBLIC|S_UNKNOWN))
				s->number = sym++;
}

/* Write the symbols to the output file */
static void write_symbols(FILE *fp)
{
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++) {
		for (s = symhash[i]; s != NULL; s=s->next) {
			fputc(s->type, fp);
			fwrite(s->name, 16, 1, fp);
			fputc(s->value, fp);
			fputc(s->value >> 8, fp);
		}
	}
}

/*
 *	TODO: Fold all these symbol table walks into a helper
 */

/*
 *	Print a symbol for the map file
 */
static void print_symbol(struct symbol *s, FILE *fp)
{
	char c;
	if (s->type & S_UNKNOWN)
		c = 'U';
	else {
		c = "acdb"[s->type & S_SEGMENT];
		if (s->type & S_PUBLIC)
			c = toupper(c);
	}
	fprintf(fp, "%04X %c %s\n", s->value, c, s->name);
}

/*
 *	Walk the symbol table generating a map file as we go
 */
static void write_map_file(FILE *fp)
{
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++) {
		for (s = symhash[i]; s != NULL; s=s->next)
			print_symbol(s, fp);
	}
}

/*
 *	Check that the newly discovered object file is the same format
 *	as the existing one. Also check for big endian as we don't yet
 *	support that (although we are close).
 */
static void compatible_obj(struct objhdr *oh)
{
	if (obj_flags != -1 && oh->o_flags != obj_flags) {
		fprintf(stderr, "Mixed object types not supported.\n");
		exit(1);
	}
	if (oh->o_flags & OF_BIGENDIAN) {
		fprintf(stderr, "Warning: Big endian not tested.\n");
	}
	obj_flags = oh->o_flags;
}

/*
 *	See if we already merged an object module. With a library we
 *	scan mutiple times but we don't import the same module twice
 */

static int have_object(off_t pos, const char *name)
{
	struct object *o = objects;
	while(o) {
		if (o->off == pos && strcmp(name, o->path) == 0)
			return 1;
		o = o->next;
	}
	return 0;
}

/*
 *	Load a new object file. The off argument allows us to load an
 *	object module out of a library by giving the library file handle
 *	and the byte offset into it.
 *
 *	Do all the error reporting and processing needed to incorporate
 *	the module, and load and add all the symbols.
 */
static struct object *load_object(FILE * fp, off_t off, int lib, const char *path)
{
	int i;
	uint8_t type;
	char name[17];
	struct object *o = new_object();
	struct symbol **sp;
	int nsym;
	uint16_t value;

	o->path = path;
	o->off = off;
	processing = o;	/* For error reporting */

	xfseek(fp, off);
	if (fread(&o->oh, sizeof(o->oh), 1, fp) != 1 || o->oh.o_magic != MAGIC_OBJ || o->oh.o_symbase == 0) {
		/* A library may contain other things, just ignore them */
		if (lib)
			return NULL;
		else	/* But an object file must be valid */
			error("bad object file");
	}
	compatible_obj(&o->oh);
	/* Load up the symbols */
	nsym = (o->oh.o_dbgbase - o->oh.o_symbase) / S_SIZE;
	if (nsym < 0||nsym > 65535)
		error("bad object file");
	/* Allocate the symbol entries */
	o->syment = (struct symbol **) xmalloc(sizeof(struct symbol *) * nsym);
	o->nsym = nsym;
restart:
	xfseek(fp, off + o->oh.o_symbase);
	sp = o->syment;
	for (i = 0; i < nsym; i++) {
		type = fgetc(fp);
		if (!(type & S_UNKNOWN) && (type & S_SEGMENT) > ZP)
			error("bad symbol");
		fread(name, 16, 1, fp);
		name[16] = 0;
		value = fgetc(fp) + (fgetc(fp) << 8);
		/* In library mode we look for a symbol that means we will load
		   this object - and then restart wih lib = 0 */
		if (lib) {
			if (is_undefined(name)) {
				if (verbose)
					printf("importing for '%s'\n", name);
				lib = 0;
				goto restart;
			}
		} else
			*sp++ = find_alloc_symbol(o, type, name, value);
	}
	/* If we get here with lib set then this was a library module we didn't
	   in fact require */
	if (lib) {
		free_object(o);
		processing = NULL;
		return NULL;
	}
	insert_object(o);
	/* Make sure all the files are the same architeture */
	if (arch) {
		if (o->oh.o_arch != arch)
			error("wrong architecture");
	} else
		arch = o->oh.o_arch;
	/* The CPU features required is the sum of all the flags in the objects */
	arch_flags |= o->oh.o_cpuflags;
	processing = NULL;
	return o;
}

/*
 *	Once all the objects are loaded this function walks the list and
 *	assigns each object file a base address for each segment. We do
 *	this by walking the list once to find the total size of code/data/bss
 *	and then a second time to set the offsets.
 */
static void set_segment_bases(void)
{
	struct object *o;
	uint16_t pos[4];
	int i;

	/* We are doing a simple model here without split I/D for now */
	for (i = 1; i < 4; i++)
		size[i] = 0;
	/* Now run through once computing the basic size of each segment */
	for (o = objects; o != NULL; o = o->next) {
		for (i = 1; i < 4; i++) {
			size[i] += o->oh.o_size[i];
			if (size[i] < o->oh.o_size[i])
				error("segment too large");
		}
	}
	/* We now know where to put the binary */
	if (ldmode != LD_RFLAG) {
		/* Creating a binary - put the segments together */
		if (split_id)
			base[2] = size[1];
		else {
			/* Single image. Check if we are aligning */
			if (align)
				base[2] = ((base[1] + size[1] + align - 1)/align) * align;
			if (base[2] < base[1])
				error("image too large");
		}
		base[3] = base[2] + size[2];

		/* ZP if any is assumed to be set on input */

		if (base[3] < base[2] || base[3] + size[3] < base[3])
			error("image too large");
		/* Whoopee it fits */
		/* Insert the linker symbols */
		insert_internal_symbol("__code", CODE, 0);
		insert_internal_symbol("__data", DATA, 0);
		insert_internal_symbol("__bss", BSS, 0);
		insert_internal_symbol("__end", BSS, size[3]);
		insert_internal_symbol("__code_size", ABSOLUTE, size[1]);
		insert_internal_symbol("__data_size", ABSOLUTE, size[2]);
		insert_internal_symbol("__bss_size", ABSOLUTE, size[3]);
	}
	/* Now set the base of each object appropriately */
	memcpy(&pos, &base, sizeof(pos));
	for (o = objects; o != NULL; o = o->next) {
		o->base[0] = 0;
		for (i = 1; i < 4; i++) {
			o->base[i] = pos[i];
			pos[i] += o->oh.o_size[i];
		}
	}
	/* At this point we have correctly relocated the base for each object. What
	   we have yet to do is to relocate the symbols. Internal symbols are always
	   created as absolute with no definedby */
	for (i = 0; i < NHASH; i++) {
		struct symbol *s = symhash[i];
		while (s != NULL) {
			uint8_t seg = s->type & S_SEGMENT;
			/* base will be 0 for absolute */
			if (s->definedby)
				s->value += s->definedby->base[seg];
			else
				s->value += base[seg];
			/* FIXME: check overflow */
			s = s->next;
		}
	}
	/* We now know all the base addresses and all the symbol values are
	   corrected. Everything needed for relocation is present */
}

/*
 *	Write a word to the target in the correct endianness
 */
static void target_put(struct object *o, uint16_t value, uint16_t size, FILE *op)
{
	if (size == 1)
		fputc(value, op);
	else {
		if (o->oh.o_flags&OF_BIGENDIAN) {
			fputc(value >> 8, op);
			fputc(value, op);
		} else {
			fputc(value, op);
			fputc(value >> 8, op);
		}
	}
}

/*
 *	Read a work from the target in the correct endianness. For
 *	better or worse all our relocation streams are always little endian
 *	while the instruction stream being relocated is of necessity native
 *	endian.
 */
static uint16_t target_get(struct object *o, uint16_t size, FILE *ip)
{
	if (size == 1)
		return fgetc(ip);
	else {
		if (o->oh.o_flags & OF_BIGENDIAN)
			return (fgetc(ip) << 8) + fgetc(ip);
		else
			return fgetc(ip) + (fgetc(ip) << 8);
	}
}

/*
 *	Relocate the stream of input from ip to op
 *
 * We support three behaviours
 * LD_ABSOLUTE: all symbols are resolved and the relocation quoting is removed
 * LD_RFLAG: quoting is copied, internal symbols are resovled, externals kept
 * LD_RELOC: a relocation stream is output with no remaining symbol relocations
 *	     and all internal relocations resolved.
 */
static void relocate_stream(struct object *o, int segment, FILE * op, FILE * ip)
{
	int c;
	uint8_t size;
	uint16_t r;
	struct symbol *s;

	processing = o;

	while ((c = fgetc(ip)) != EOF) {
		uint8_t code = (uint8_t) c;
		uint8_t optype;
		uint8_t overflow = 1;
		uint8_t high = 0;

		/* Unescaped material is just copied over */
		if (code != REL_ESC) {
			fputc(code, op);
			dot++;
			continue;
		}
		code = fgetc(ip);
		if (code == REL_EOF) {
			processing = NULL;
			return;
		}
		/* Escaped 0xDA byte. Just copy it over, and if in absolute mode
		   remove the escaped byte */
		if (code == REL_REL) {
			if (ldmode != LD_ABSOLUTE)
				fputc(REL_ESC, op);
			fputc(REL_REL, op);
			dot++;
			continue;
		}

		if (code == REL_ORG) {
			if (ldmode != LD_ABSOLUTE) {
				fprintf(stderr, "%s: absolute addressing requires '-b'.\n", o->path);
				exit(1);
			}
			if (segment != ABSOLUTE) {
				fprintf(stderr, "%s: cannot set address in non absolute segment.\n", o->path);
				exit(1);
			}
			dot = fgetc(ip);
			dot |= fgetc(ip) << 8;
			xfseek(op, dot);
			continue;
		}
		if (code == REL_OVERFLOW) {
			overflow = 0;
			code = fgetc(ip);
		}
		if (code == REL_HIGH) {
			high = 1;
			code = fgetc(ip);
		}
		/* Relocations */
		size = ((code & S_SIZE) >> 4) + 1;

		/* Simple relocation - adjust versus base of a segment of this object */
		if (code & REL_SIMPLE) {
			uint8_t seg = code & S_SEGMENT;
			/* Check entry is valid */
			if (seg == ABSOLUTE || seg > ZP || size > 2)
				error("invalid reloc");
			/* If we are not building an absolute then keep the tag */
			if (ldmode != LD_ABSOLUTE) {
				fputc(REL_ESC, op);
				if (!overflow)
					fputc(REL_OVERFLOW, op);
				if (high)
					fputc(REL_HIGH, op);
				fputc(code, op);
			}
			/* Relocate the value versus the new segment base and offset of the
			   object */
			r = target_get(o, size, ip);
			r += o->base[seg];
			if (overflow && (r < o->base[seg] || (size == 1 && r > 255)))
				error("relocation exceeded");
			if (high && ldmode == LD_ABSOLUTE) {
				r >>= 8;
				size = 1;
			}
			target_put(o, r, size, op);
			dot += size;
			continue;
		}
		optype = code & REL_TYPE;
		/* Symbolic relocations - may be inter-segment and inter-object */
		switch(optype)
		{
			default:
				error("invalid reloc type");
			case REL_SYMBOL:
			case REL_PCREL:
				r = fgetc(ip);
				r |= fgetc(ip) << 8;
				/* r is the symbol number */
				if (r >= o->nsym)
					error("invalid reloc sym");
				s = o->syment[r];
				fprintf(stderr, "relocating sym %d (%s : %x)\n", r, s->name, s->type);
				if (s->type & S_UNKNOWN) {
					if (ldmode != LD_RFLAG) {
						if (processing)
							fprintf(stderr, "%s: Unknown symbol '%.16s'.\n", o->path, s->name);
						err |= 1;
					}
					if (ldmode != LD_ABSOLUTE) {
						/* Rewrite the record with the new symbol number */
						fputc(REL_ESC, op);
						if (!overflow)
							fputc(REL_OVERFLOW, op);
						if (high)
							fputc(REL_HIGH, op);
						fputc(code, op);
						fputc(s->number, op);
						fputc(s->number >> 8, op);
					}
					/* Copy the bytes to relocate */
					fputc(fgetc(ip), op);
					if (size == 2 || optype == REL_PCREL)
						fputc(fgetc(ip), op);
				} else {
					/* Get the relocation data */
					r = target_get(o, optype == REL_PCREL ? 2 : size, ip);
					/* Add the offset from the output segment base to the
					   symbol */
					r += s->value;
					if (optype == REL_PCREL) {
						int16_t off = r;
						if (obj_flags & OF_WORDMACHINE)
							off -= dot / 2;
						else
							off -= dot;
						if (overflow && size == 1 && (off < -128 || off > 127))
							error("relocation exceeded");
						r = (uint16_t)off;
					} else {
						/* Check again */
						if (overflow && (r < s->value || (size == 1 && r > 255)))
							error("relocation exceeded");
					}
					/* If we are not fully resolving then turn this into a
					   simple relocation */
					if (ldmode != LD_ABSOLUTE && optype != REL_PCREL) {
						fputc(REL_ESC, op);
						if (!overflow)
							fputc(REL_OVERFLOW, op);
						if (high)
							fputc(REL_HIGH, op);
						fputc(REL_SIMPLE | (s->type & S_SEGMENT) | (size - 1) << 4, op);
					}
					if (ldmode == LD_ABSOLUTE && high) {
						r >>= 8;
						size = 1;
					}
				}
				target_put(o, r, size, op);
				dot += size;
				break;
		}
	}
	error("corrupt reloc stream");
}

/*
 *	Open an object file and seek to the right location in case it is
 *	a library module.
 */
static FILE *openobject(struct object *o)
{
	FILE *fp = xfopen(o->path, "r");
	xfseek(fp, o->off);
	return fp;
}

/*
 *	Write out all the segments of the output file with any needed
 *	relocations performed. We write out the code and data segments but
 *	BSS and ZP must always be zero filled so do not need writing.
 */
static void write_stream(FILE * op, int seg)
{
	struct object *o = objects;

	while (o != NULL) {
		FILE *ip = openobject(o);	/* So we can hide library gloop */
		if (verbose)
			printf("Writing %s#%ld:%d\n", o->path, o->off, seg);
		xfseek(ip, o->off + o->oh.o_segbase[seg]);
		dot = o->base[seg];
		/* In absolute mode we place segments wherever they should
		   be in the binary space */
		/* FIXME: to support ORG we will need to add a reloc type that
		   indicates an arbitrary dot change. We also then need the
		   assembler to generate the right symbol behaviours */
		if (ldmode == LD_ABSOLUTE)
			xfseek(op, dot);
		relocate_stream(o, seg, op, ip);
		xfclose(ip);
		o = o->next;
	}
	if (ldmode != LD_ABSOLUTE) {
		fputc(REL_ESC, op);
		fputc(REL_EOF, op);
	}
}

/*
 *	Write out the target file including any necessary headers if we are
 *	writing out a relocatable object. At this point we don't have any
 *	support for writing out a nice loader friendly format, and that
 *	may want to be a separate tool that consumes a resolved relocatable
 *	binary and generates a separate relocation block in the start of
 *	BSS.
 */
static void write_binary(FILE * op, FILE *mp)
{
	static struct objhdr hdr;
	hdr.o_arch = arch;
	hdr.o_cpuflags = arch_flags;
	hdr.o_flags = obj_flags;
	hdr.o_segbase[0] = sizeof(hdr);
	hdr.o_size[0] = size[0];
	hdr.o_size[1] = size[1];
	hdr.o_size[2] = size[2];
	hdr.o_size[3] = size[3];

	rewind(op);
	if (ldmode != LD_ABSOLUTE)
		fwrite(&hdr, sizeof(hdr), 1, op);
	/* For LD_RFLAG number the symbols for output, for othe forms
	   check for unknowmn symbols and error them out */
	if (ldmode != LD_ABSOLUTE)
		renumber_symbols();
	if (ldmode == LD_RFLAG && size[0]) {
		fprintf(stderr, "Cannot build a relocatable binary including absolute segments.\n");
		exit(1);
	}
	write_stream(op, ABSOLUTE);
	write_stream(op, CODE);
	hdr.o_segbase[1] = ftell(op);
	write_stream(op, DATA);
	if (!strip) {
		hdr.o_symbase = ftell(op);
		write_symbols(op);
	}
	hdr.o_dbgbase = ftell(op);
	hdr.o_magic = MAGIC_OBJ;
	/* TODO: needs a special pass
	if (dbgsyms )
		copy_debug_all(op, mp);*/ 
	if (err == 0) {
		if (ldmode != LD_ABSOLUTE) {
			xfseek(op, 0);
			fwrite(&hdr, sizeof(hdr), 1, op);
		} else	/* FIXME: honour umask! */
			fchmod (fileno(op), 0755);
	}
}

/*
 *	Scan through all the object modules in this ar archive and offer
 *	them to the linker.
 */
static void process_library(const char *name, FILE *fp)
{
	static struct ar_hdr ah;
	off_t pos = SARMAG;
	unsigned long size;

	while(1) {
		xfseek(fp, pos);
		if (fread(&ah, sizeof(ah), 1, fp) == 0)
			break;
		if (ah.ar_fmag[0] != 96 || ah.ar_fmag[1] != '\n')
			break;
		size = atol(ah.ar_size);
#if 0 /* TODO */
		if (strncmp(ah.ar_name, ".RANLIB") == 0)
			process_ranlib();
#endif
		pos += sizeof(ah);
		if (!have_object(pos, name))
			load_object(fp, pos, 1, name);
		pos += size;
		if (pos & 1)
			pos++;
	}
}

/*
 *	This is called for each object module and library passed on the
 *	command line and in the order given. We process them in that order
 *	including libraries, so you need to put termcap.a before libc.a etc
 *	or you'll get errors.
 */
static void add_object(const char *name, off_t off, int lib)
{
	static char x[SARMAG];
	FILE *fp = xfopen(name, "r");
	if (off == 0 && !lib) {
		/* Is it a bird, is it a plane ? */
		fread(x, SARMAG, 1, fp);
		if (memcmp(x, ARMAG, SARMAG) == 0) {
			/* No it's a library. Do the library until a
			   pass of the library resolves nothing. This isn't
			   as fast as we'd like but we need ranlib support
			   to do faster */
			do {
				progress = 0;
				xfseek(fp, SARMAG);
				process_library(name, fp);
			/* FIXME: if we counted unresolved symbols we might
			   be able to exit earlier ? */
			} while(progress);
			xfclose(fp);
			return;
		}
	}
	xfseek(fp, off);
	load_object(fp, off, lib, name);
	xfclose(fp);
}

/*
 *	Process the arguments, open the files and run the entire show
 */
int main(int argc, char *argv[])
{
	int opt;
	FILE *bp, *mp = NULL;

	arg0 = argv[0];

	while ((opt = getopt(argc, argv, "rbvtsiu:o:m:A:B:C:D:")) != -1) {
		switch (opt) {
		case 'r':
			ldmode = LD_RFLAG;
			break;
		case 'b':
			ldmode = LD_ABSOLUTE;
			strip = 1;
			break;
		case 'v':
			printf("FuzixLD 0.1.1\n");
			break;
		case 't':
			verbose = 1;
			break;
		case 'o':
			outname = optarg;
			break;
		case 'm':
			mapname = optarg;
			break;
		case 'u':
			insert_internal_symbol(optarg, -1, 0);
			break;
		case 's':
			strip = 1;
			break;
		case 'i':
			split_id = 1;
			break;
		case 'A':
			align = strtoul(optarg, NULL, 0);
			break;
		case 'B':
			base[1] = strtoul(optarg, NULL, 0);
			break;
		case 'C':
			base[1] = strtoul(optarg, NULL, 0);
			break;
		case 'D':
			base[2] = strtoul(optarg, NULL, 0);
			break;
		default:
			fprintf(stderr, "%s: name ...\n", argv[0]);
			exit(1);
		}
	}
	if (outname == NULL)
		outname = "a.out";

	while (optind < argc) {
		/* FIXME: spot libraries and handle here */
		if (verbose)
			printf("Loading %s\n", argv[optind]);
		add_object(argv[optind], 0, 0);
		optind++;
	}
	if (verbose)
		printf("Computing memory map.\n");
	set_segment_bases();
	if (verbose)
		printf("Writing output.\n");

	bp = xfopen(outname, "w");
	if (mapname) {
		mp = xfopen(mapname, "w");
		if (verbose)
			printf("Writing map file.\n");
		write_map_file(mp);
	}
	write_binary(bp,mp);
	xfclose(bp);
	if (mp)
		fclose(mp);
	exit(err);
}
