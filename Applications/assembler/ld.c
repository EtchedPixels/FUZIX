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
 *	1.	For speed libraries can start with an _RANLIB ar file node which
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
static const char *libentry;		/* Library entry name if relevant */
static struct object *objects, *otail;	/* List of objects */
static struct symbol *symhash[NHASH];	/* Symbol has tables */
static uint16_t base[OSEG];		/* Base of each segment */
static uint16_t size[OSEG];		/* Size of each segment */
static uint16_t align = 1;		/* Alignment */
static uint16_t baseset[OSEG];		/* Did the user force this one */
#define LD_RELOC	0		/* Output a relocatable binary stream */
#define LD_RFLAG	1		/* Output an object module */
#define LD_ABSOLUTE	2		/* Output a linked binary */
#define LD_FUZIX	3		/* Output a Fuzix binary */
static uint8_t ldmode = LD_FUZIX;	/* Operating mode */
static uint8_t rawstream;		/* Outputting raw or quoted ? */

static uint8_t split_id;		/* True if code and data both zero based */
static uint8_t arch;			/* Architecture */
static uint16_t arch_flags;		/* Architecture specific flags */
static uint8_t verbose;			/* Verbose reporting */
static int err;				/* Error tracking */
static int dbgsyms = 1;			/* Set to dumb debug symbols */
static int strip = 0;			/* Set to strip symbols */
static int obj_flags = -1;		/* Object module flags for compat */
static const char *mapname;		/* Name of map file to write */
static const char *outname;		/* Name of output file */
static uint16_t dot;			/* Working address as we link */

static uint8_t progress;		/* Did we make forward progress ?
					   Used while library linking */

/*
 *	Report an error, and if possible give the object or library that
 *	we were processing.
 */
static void warning(const char *p)
{
	if (processing)
		fprintf(stderr, "While processing: %s", processing->path);
	if (libentry)
		fprintf(stderr, "(%.16s)", libentry);
	fputc('\n', stderr);
	fputs(p, stderr);
	fputc('\n', stderr);
	err |= 2;
}

static void error(const char *p)
{
	warning(p);
	exit(err);
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

static uint16_t xstrtoul(const char *p)
{
	char *r;
	unsigned long x = strtoul(p, &r, 0);
	if (r == p) {
		fprintf(stderr, "'%s' is not a valid numeric constant.\n", p);
		exit(1);
	}
	if (x > 65535) {
		fprintf(stderr, "'%s' is not in the range 0-65535.\n", p);
		exit(1);
	}
	return x;
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
	strncpy(s->name, name, NAMELEN);
	s->next = symhash[hash];
	symhash[hash] = s;
	if (verbose)
		printf("+%.*s\n", NAMELEN, name);
	return s;
}

/*
 *	Find a symbol in a given has table	
 */
struct symbol *find_symbol(const char *name, int hash)
{
	struct symbol *s = symhash[hash];
	while (s) {
		if (strncmp(s->name, name, NAMELEN) == 0)
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

	while(*name && n++ < NAMELEN)
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
	fprintf(stderr, "Segment mismatch for symbol '%.*s'.\n", NAMELEN, s->name);
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
/*FIXME         strlcpy(s->name, id, NAMELEN); */
		strncpy(s->name, id, NAMELEN);
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
		fprintf(stderr, "%.*s: multiply defined.\n", NAMELEN, id);
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
			fwrite(s->name, NAMELEN, 1, fp);
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
		c = "acdbzxs7"[s->type & S_SEGMENT];
		if (s->type & S_PUBLIC)
			c = toupper(c);
	}
	fprintf(fp, "%04X %c %.*s\n", s->value, c, NAMELEN, s->name);
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
	char name[NAMELEN + 1];
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
		if (lib) {
			free_object(o);
			processing = NULL;
			return NULL;
		}
		else	/* But an object file must be valid */
			error("bad object file");
	}
	compatible_obj(&o->oh);
	/* Load up the symbols */
	nsym = (o->oh.o_dbgbase - o->oh.o_symbase) / S_ENTRYSIZE;
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
		fread(name, NAMELEN, 1, fp);
		name[NAMELEN] = 0;
		value = fgetc(fp) + (fgetc(fp) << 8);
		if (!(type & S_UNKNOWN) && (type & S_SEGMENT) >= OSEG) {
			fprintf(stderr, "Symbol %s\n", name);
			if ((type & S_SEGMENT) == UNKNOWN)
				error("exported but undefined");
			else
				error("bad symbol");
		}
		/* In library mode we look for a symbol that means we will load
		   this object - and then restart wih lib = 0 */
		if (lib) {
			if (!(type & S_UNKNOWN) && is_undefined(name)) {
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
 *	Helper for layout computation. Add one segment after another
 *	ane ensure it fits. If a segment base is hand set don't touch it
 */

static void append_segment(int a, int b)
{
	if (baseset[a])
		return;
	base[a] = ((base[b] + size[b] + align - 1)/align) * align;
	if (base[a] < base[b])
		error("image too large");
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
	uint16_t pos[OSEG];
	int i;

	/* We are doing a simple model here without split I/D for now */
	for (i = 1; i < OSEG; i++)
		size[i] = 0;
	/* Now run through once computing the basic size of each segment */
	for (o = objects; o != NULL; o = o->next) {
		if (verbose)
			printf("%s:\n", o->path);
		for (i = 1; i < OSEG; i++) {
			if (verbose)
				printf("\t%c : %04X\n",
					"ACDBZXc?"[i], o->oh.o_size[i]);
			size[i] += o->oh.o_size[i];
			if (size[i] < o->oh.o_size[i])
				error("segment too large");
		}
	}

	if (verbose) {
		for (i = 1; i < 7; i++)
			printf("Segment %c Size %04X\n", "ACDBZXc"[i], size[i]);
	}
	/* We now know where to put the binary */
	if (ldmode == LD_RELOC) {
		/* Creating a binary - put the segments together */
		if (split_id && !baseset[2])
			base[2] = 0;
		else {
			append_segment(7, 1);
			append_segment(2, 7);
			append_segment(3, 2);
		}
	} else {
		/* FIXME: some kind of link scripts one day ? */
		/* Where to put stuff. Try and be helpful. This is a shade
		   Fuzix oriented */
		/* Default literals then data after code */
		append_segment(7, 1);
		append_segment(2, 7);
		append_segment(3, 2);
		/* ZP we leave alone */
		/* Discard after BSS */
		append_segment(5, 3);
		/* Common we can't really do much to guess a layout */
	}
	if (ldmode != LD_RFLAG) {
		/* ZP if any is assumed to be set on input */
		/* FIXME: check the literals fit .. make this a more sensible
		   overlap check loop ? */
		if (base[3] < base[2] || base[3] + size[3] < base[3])
			error("image too large");
		/* Whoopee it fits */
		/* Insert the linker symbols */
		/* FIXME: symbols for all OSEG segments */
		insert_internal_symbol("__code", CODE, 0);
		insert_internal_symbol("__data", DATA, 0);
		insert_internal_symbol("__bss", BSS, 0);
		insert_internal_symbol("__literal", LITERAL, 0);
		insert_internal_symbol("__end", BSS, size[3]);
		insert_internal_symbol("__zp", ZP, 0);
		insert_internal_symbol("__discard", DISCARD, 0);
		insert_internal_symbol("__common", COMMON, 0);
		insert_internal_symbol("__code_size", ABSOLUTE, size[CODE]);
		insert_internal_symbol("__data_size", ABSOLUTE, size[DATA]);
		insert_internal_symbol("__bss_size", ABSOLUTE, size[BSS]);
		insert_internal_symbol("__literal_size", ABSOLUTE, size[LITERAL]);
		insert_internal_symbol("__zp_size", ABSOLUTE, size[ZP]);
		insert_internal_symbol("__discard_size", ABSOLUTE, size[DISCARD]);
		insert_internal_symbol("__common_size", ABSOLUTE, size[COMMON]);
	}
	/* Now set the base of each object appropriately */
	memcpy(&pos, &base, sizeof(pos));
	for (o = objects; o != NULL; o = o->next) {
		o->base[0] = 0;
		for (i = 1; i < OSEG; i++) {
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
 *	Write a target byte with correct quoting if needed
 *
 *	We quote if we are outputing a new link binary (ld -r), or a
 *	relocatable.
 */

static void target_pquoteb(uint8_t v, FILE *op)
{
	if (v == REL_ESC && !rawstream) {
		fputc(v, op);
		fputc(REL_REL, op);
	} else
		fputc(v, op);
}

/*
 *	Write a word to the target in the correct endianness
 */
static void target_put(struct object *o, uint16_t value, uint16_t size, FILE *op)
{
	if (size == 1)
		target_pquoteb(value, op);
	else {
		if (o->oh.o_flags&OF_BIGENDIAN) {
			target_pquoteb(value >> 8, op);
			target_pquoteb(value, op);
		} else {
			target_pquoteb(value, op);
			target_pquoteb(value >> 8, op);
		}
	}
}

static int target_pgetb(FILE *ip)
{
	int c = fgetc(ip);
	if (c == EOF)
		error("unexpected EOF");
	return c;
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
		return target_pgetb(ip);
	else {
		if (o->oh.o_flags & OF_BIGENDIAN)
			return (target_pgetb(ip) << 8) + target_pgetb(ip);
		else
			return target_pgetb(ip) + (target_pgetb(ip) << 8);
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

//		if (ldmode == LD_ABSOLUTE && ftell(op) != dot) {
//			fprintf(stderr, "%ld not %d\n",
//				(long)ftell(op), dot);
//		}

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
		   remove the REL_REL marker */
		if (code == REL_REL) {
			if (!rawstream) {
				fputc(REL_ESC, op);
				fputc(REL_REL, op);
			} else
				fputc(REL_ESC, op);
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
			overflow = 0;
			code = fgetc(ip);
		}
		/* Relocations */
		size = ((code & S_SIZE) >> 4) + 1;


		/* Simple relocation - adjust versus base of a segment of this object */
		if (code & REL_SIMPLE) {
			uint8_t seg = code & S_SEGMENT;
			/* Check entry is valid */
			if (seg == ABSOLUTE || seg >= OSEG || size > 2) {
				fprintf(stderr, "%s invalid reloc %d %d\n",
					o->path, seg, size);
				error("invalid reloc");
			}
			/* If we are not building an absolute then keep the tag */
			if (!rawstream) {
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
//			fprintf(stderr, "Target is %x, Segment %d base is %x\n", 
//				r, seg, o->base[seg]);
			r += o->base[seg];
			if (overflow && (r < o->base[seg] || (size == 1 && r > 255))) {
				fprintf(stderr, "%d width relocation offset %d does not fit.\n", size, r);
				fprintf(stderr, "relocation failed at 0x%04X\n", dot);
				warning("relocation exceeded");
			}
			/* A high relocation had a 16bit input value we relocate versus
			   the base then chop down */
			if (high && rawstream) {
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
//				fprintf(stderr, "relocating sym %d (%s : %x)\n", r, s->name, s->type);
				if (s->type & S_UNKNOWN) {
					if (ldmode != LD_RFLAG) {
						if (processing)
							fprintf(stderr, "%s: Unknown symbol '%.*s'.\n", o->path, NAMELEN, s->name);
						err |= 1;
					}
					if (!rawstream) {
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
							error("byte relocation exceeded");
						r = (uint16_t)off;
					} else {
						/* Check again */
						if (overflow && (r < s->value || (size == 1 && r > 255))) {
							fprintf(stderr, "width %d relocation offset %d, %d, %d does not fit.\n", size, r-s->value, s->value, r);
							error("relocation exceeded");
						}
					}
					/* If we are not fully resolving then turn this into a
					   simple relocation */
					if (!rawstream && optype != REL_PCREL) {
						fputc(REL_ESC, op);
						if (!overflow)
							fputc(REL_OVERFLOW, op);
						if (high)
							fputc(REL_HIGH, op);
						fputc(REL_SIMPLE | (s->type & S_SEGMENT) | (size - 1) << 4, op);
					}
					if (rawstream && high) {
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
		if (verbose)
			printf("%s:Segment %d file seek base %d\n",
				o->path,
				seg, o->oh.o_segbase[seg]);
		dot = o->base[seg];
		/* For Fuzix we place segments in absolute space but don't
		   bother writing out the empty page before */
		if (ldmode == LD_FUZIX) 
			xfseek(op, dot - base[1]);
		/* In absolute mode we place segments wherever they should
		   be in the binary space */
		else if (ldmode == LD_ABSOLUTE)
			xfseek(op, dot);
		relocate_stream(o, seg, op, ip);
		xfclose(ip);
		o = o->next;
	}
	if (!rawstream) {
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
	uint8_t i;

	hdr.o_arch = arch;
	hdr.o_cpuflags = arch_flags;
	hdr.o_flags = obj_flags;
	hdr.o_segbase[0] = sizeof(hdr);
	hdr.o_size[0] = size[0];
	hdr.o_size[1] = size[1];
	hdr.o_size[2] = size[2];
	hdr.o_size[3] = size[3];

	rewind(op);
	if (!rawstream)
		fwrite(&hdr, sizeof(hdr), 1, op);
	/* For LD_RFLAG number the symbols for output, for othe forms
	   check for unknowmn symbols and error them out */
	if (!rawstream)
		renumber_symbols();
	if ((ldmode == LD_FUZIX || ldmode == LD_RFLAG) && size[0]) {
		fprintf(stderr, "Cannot build a Fuzix or relocatable binary including absolute segments.\n");
		exit(1);
	}
	if (ldmode != LD_FUZIX)
		write_stream(op, ABSOLUTE);
	write_stream(op, CODE);
	hdr.o_segbase[1] = ftell(op);
	write_stream(op, DATA);
	/* Absolute images may contain things other than code/data/bss */
	if (ldmode == LD_ABSOLUTE) {
		for (i = 4; i < OSEG; i++) {
			write_stream(op, i);
		}
	}
	else {
		/* ZP is ok in Fuzix but is not initialized in a defined way */
		for (i = ldmode == LD_FUZIX ? 5 : 4; i < OSEG; i++) {
			if (size[i]) {
				fprintf(stderr, "Unsupported data in non-standard segment %d.\n", i);
				break;
			}
		}
		if (!rawstream && !strip) {
			hdr.o_symbase = ftell(op);
			write_symbols(op);
		}
	}
	hdr.o_dbgbase = ftell(op);
	hdr.o_magic = MAGIC_OBJ;
	/* TODO: needs a special pass
	if (dbgsyms )
		copy_debug_all(op, mp);*/ 
	if (err == 0) {
		if (!rawstream) {
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
		if (strncmp(ah.ar_name, ".RANLIB", 8) == 0)
			process_ranlib();
#endif
		libentry = ah.ar_name;
		pos += sizeof(ah);
		if (!have_object(pos, name))
			load_object(fp, pos, 1, name);
		pos += size;
		if (pos & 1)
			pos++;
	}
	libentry = NULL;
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
	int i;
	FILE *bp, *mp = NULL;

	arg0 = argv[0];

	while ((opt = getopt(argc, argv, "rbvtsiu:o:m:A:B:C:D:S:X:Z:")) != -1) {
		switch (opt) {
		case 'r':
			ldmode = LD_RFLAG;
			break;
		case 'b':
			ldmode = LD_ABSOLUTE;
			strip = 1;
			break;
		case 'v':
			printf("FuzixLD 0.2.1\n");
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
			align = xstrtoul(optarg);
			if (align == 0)
				align = 1;
			break;
		case 'B':	/* BSS */
			base[3] = xstrtoul(optarg);
			baseset[3] = 1;
			break;
		case 'C':	/* CODE */
			base[1] = xstrtoul(optarg);
			baseset[1] = 1;
			break;
		case 'D':	/* DATA */
			base[2] = xstrtoul(optarg);
			baseset[2] = 1;
			break;
		case 'L':
			base[4] = xstrtoul(optarg);
			baseset[4] = 1;
			break;
		case 'S':	/* Shared/Common */
			base[6] = xstrtoul(optarg);
			baseset[6] = 1;
			break;
		case 'X':	/* DISCARD */
			base[5] = xstrtoul(optarg);
			baseset[5] = 1;
			break;
		case 'Z':	/* ZP / DP */
			base[4] = xstrtoul(optarg);
			baseset[4] = 1;
			break;
		default:
			fprintf(stderr, "%s: name ...\n", argv[0]);
			exit(1);
		}
	}
	if (outname == NULL)
		outname = "a.out";

	if (ldmode != LD_ABSOLUTE) {
		for (i = 0; i < OSEG; i++) {
			if (baseset[i]) {
				fprintf(stderr, "%s: cannot set addresses except in absolute mode.\n", argv[0]);
				exit(1);
			}
		}
	}
	if (ldmode == LD_ABSOLUTE && split_id) {
		fprintf(stderr, "%s: split I/D absolute is not yet supported.\n", argv[0]);
		/* TODO */
		exit(1);
	}
	if (ldmode == LD_FUZIX) {
		/* Fuzix binaries are for now 0x100 based */
		/* FIXME: this is really arch dependent so we ought to
		   have a table (and eventually do relocatables anyway) */
		baseset[CODE] = 1;
		base[CODE] = 0x0100;
		base[ZP] = 0x28;	/* Skip I/O space */
	}
	if (ldmode == LD_FUZIX || ldmode == LD_ABSOLUTE)
		rawstream = 1;
	while (optind < argc) {
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
		fclose(mp);
	}
	write_binary(bp,mp);
	xfclose(bp);
	exit(err);
}
