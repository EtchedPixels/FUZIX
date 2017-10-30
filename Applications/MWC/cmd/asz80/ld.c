/*
 *	Ld symbol loading support. For the moment we don't do anything clever
 *	to avoid memory wastage. We may want to move to fixed size symbol
 *	records in obj files to speed things up.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <ar.h>

#include "obj.h"
#include "ld.h"

static char *arg0;
static struct object *processing;
static struct object *objects, *otail;
static struct symbol *symhash[NHASH];
static uint16_t base[4];
static uint16_t size[4];
#define LD_RELOC	0
#define LD_RFLAG	1
#define LD_ABSOLUTE	2
static uint8_t ldmode = LD_RELOC;
static uint8_t split_id;
static uint8_t arch;
static uint16_t arch_flags;
static uint8_t verbose;
static int err;
static int dbgsyms = 1;
static int strip = 0;
static const char *mapname;
static const char *outname;

static void error(const char *p)
{
	if (processing)
		fprintf(stderr, "While processing: %s\n", processing->path);
	fputs(p, stderr);
	fputc('\n', stderr);
	err |= 2;

	exit(err | 2);
}

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

struct symbol *new_symbol(const char *name, int hash)
{
	struct symbol *s = xmalloc(sizeof(struct symbol));
	strncpy(s->name, name, 16);
	s->next = symhash[hash];
	symhash[hash] = s;
	return s;
}

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

static uint8_t hash_symbol(const char *name)
{
	int hash = 0;
	uint8_t n = 16;

	do {
		hash += *name++;
	} while (--n);
	return (hash&(NHASH-1));
}

/* Check if a symbol name is known but undefined. We use this to decide
   whether to incorporate a library module */
static int is_undefined(const char *name)
{
	int hash = hash_symbol(name);
	struct symbol *s = find_symbol(name, hash);
	if (s == NULL || !(s->type & S_UNKNOWN))
		return 0;
	/* This is a symbol we need */
	return 1;
}

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
	if (type & S_UNKNOWN)
		/* We are an external reference to a symbol. No work needed */
		return s;
	if (s->type & S_UNKNOWN) {
		/* We are referencing a symbol that was previously unknown but which
		   we define. Fill in the details */
		if ((s->type & S_SEGMENT) != S_ANY &&
			(s->type & S_SEGMENT) != (type & S_SEGMENT)) {
			fprintf(stderr, "Segment mismatch for symbol '%s'.\n", id);
			err |= 2;
		}
		s->type = type;
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

static void insert_internal_symbol(const char *name, int seg, uint16_t val)
{
	find_alloc_symbol(NULL, seg | S_PUBLIC, name, val);
}


/* Number the symbols that we will write out. We don't care about the mode
   too much. In valid reloc mode we won't have any S_UNKNOWN symbols anyway */
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

/* Fold all these symbol table walks into a helper */
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

static void write_map_file(FILE *fp)
{
	struct symbol *s;
	int i;
	for (i = 0; i < NHASH; i++) {
		for (s = symhash[i]; s != NULL; s=s->next)
			print_symbol(s, fp);
	}
}

struct object *load_object(FILE * fp, off_t off, int lib, const char *path)
{
	int i;
	uint8_t type;
	char name[17];
	struct object *o = new_object();
	struct symbol **sp;
	int nsym;
	uint16_t value;

	o->path = path;
	processing = o;	/* For error reporting */

	xfseek(fp, off);
	if (fread(&o->oh, sizeof(o->oh), 1, fp) != 1 || o->oh.o_magic != MAGIC_OBJ || o->oh.o_symbase == 0) {
		/* A library may contain other things, just ignore them */
		if (lib)
			return NULL;
		else	/* But an object file must be valid */
			error("bad object file");
	}
	/* Load up the symbols */
	nsym = (o->oh.o_dbgbase - o->oh.o_symbase) / S_SIZE;
	if (nsym < 0)
		error("bad object file");
	/* Allocate the symbol entries */
	o->syment = (struct symbol **) xmalloc(sizeof(struct symbol *) * nsym);
	o->nsym = nsym;
      restart:
	xfseek(fp, off + o->oh.o_symbase);
	sp = o->syment;
	for (i = 0; i < nsym; i++) {
		type = fgetc(fp);
		if (!(type & S_UNKNOWN) && (type & S_SEGMENT) > BSS)
			error("bad symbol");
		fread(name, 16, 1, fp);
		name[16] = 0;
		value = fgetc(fp) + (fgetc(fp) << 8);
		/* In library mode we look for a symbol that means we will load
		   this object - and then restart wih lib = 0 */
		if (lib) {
			if (is_undefined(name)) {
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
			base[2] = base[1] + size[1];
			if (base[2] < base[1])
				error("image too large");
		}
		base[3] = base[2] + size[2];

		if (base[3] < base[2] || base[3] + size[3] < base[3])
			error("image too large");
		/* Whoopee it fits */
		/* Insert the linker symbols */
		insert_internal_symbol("__code", CODE, 0);
		insert_internal_symbol("__data", DATA, 0);
		insert_internal_symbol("__bss", BSS, 0);
		insert_internal_symbol("__end", BSS, size[3]);
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
 *	Relocate the stream of input from ip to op
 *
 * We support three behaviours
 * LD_ABSOLUTE: all symbols are resolved and the relocation quoting is removed
 * LD_RFLAG: quoting is copied, internal symbols are resovled, externals kept
 * LD_RELOC: a relocation stream is output with no remaining symbol relocations
 *	     and all internal relocations resolved.
 */
static void relocate_stream(struct object *o, FILE * op, FILE * ip)
{
	int c;
	uint8_t size;
	uint16_t r;
	struct symbol *s;

	processing = o;

	while ((c = fgetc(ip)) != EOF) {
		uint8_t code = (uint8_t) c;
		/* Unescaped material is just copied over */
		if (code != REL_ESC) {
			fputc(code, op);
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
			continue;
		}
		/* Relocations */

		size = ((code & S_SIZE) >> 4) + 1;

		/* Simple relocation - adjust versus base of a segment of this object */
		if (code & REL_SIMPLE) {
			uint8_t seg = code & S_SEGMENT;
			/* Check entry is valid */
			if (seg == ABSOLUTE || seg > BSS || size > 2)
				error("invalid reloc");
			/* If we are not building an absolute then keep the tag */
			if (ldmode != LD_ABSOLUTE) {
				fputc(REL_ESC, op);
				fputc(code, op);
			}
			/* Relocate the value versus the new segment base and offset of the
			   object */
			r = fgetc(ip);
			if (size == 2)
				r |= fgetc(ip) << 8;
			r += o->base[seg];
			if (r < o->base[seg] || (size == 1 && r > 255))
				error("relocation exceeded");
			fputc(r, op);
			if (size == 2)
				fputc(r >> 8, op);
			continue;
		}
		/* Symbolic relocations - may be inter-segment and inter-object */
		if ((code & REL_TYPE) != REL_SYMBOL)
			error("invalid reloc type");
		r = fgetc(ip);
		r |= fgetc(ip) << 8;
		/* r is the symbol number */
		if (r >= o->nsym)
			error("invalid reloc sym");
		s = o->syment[r];
		if (s->type & S_UNKNOWN) {
			if (ldmode != LD_RFLAG) {
				if (processing)
				fprintf(stderr, "%s: Unknown symbol '%.16s'.\n", o->path, s->name);
				err |= 1;
			}
			if (ldmode != LD_ABSOLUTE) {
				/* Rewrite the record with the new symbol number */
				fputc(REL_ESC, op);
				fputc(code, op);
				fputc(s->value, op);
				fputc(s->value >> 8, op);
			}
			/* Copy the bytes to relocate */
			fputc(fgetc(ip), op);
			if (size == 2)
				fputc(fgetc(ip), op);
		} else {
			/* Get the relocation bytes. These hold the offset versus
			   the referenced symbol */
			r = fgetc(ip);
			if (size == 2)
				r |= fgetc(ip) << 8;
			/* Add the offset from the output segment base to the
			   symbol */
			r += s->value;
			/* Check again */
			if (r < s->value || (size == 1 && r > 255))
				error("relocation exceeded");
			/* If we are not fully resolving then turn this into a
			   simple relocation */
			if (ldmode != LD_ABSOLUTE) {
				fputc(REL_ESC, op);
				fputc(REL_SIMPLE | (s->type & S_SEGMENT) | (size - 1) << 4, op);
			}
			fputc(r, op);
			if (size == 2)
				fputc(r >> 8, op);
		}
	}
	error("corrupt reloc stream");
}

static FILE *openobject(struct object *o)
{
	FILE *fp = xfopen(o->path, "r");
	xfseek(fp, o->off);
	return fp;
}

static void write_stream(FILE * op, int seg)
{
	struct object *o = objects;

	while (o != NULL) {
		FILE *ip = openobject(o);	/* So we can hide library gloop */
		if (verbose)
			printf("Writing %s#%ld:%d\n", o->path, o->off, seg);
		xfseek(ip, o->off + o->oh.o_segbase[seg]);
		relocate_stream(o, op, ip);
		xfclose(ip);
		o = o->next;
	}
	if (ldmode != LD_ABSOLUTE) {
		fputc(REL_ESC, op);
		fputc(REL_EOF, op);
	}
}

static void write_binary(FILE * op, FILE *mp)
{
	static struct objhdr hdr;
	hdr.o_arch = arch;
	hdr.o_cpuflags = arch_flags;
	hdr.o_segbase[0] = sizeof(hdr);
	hdr.o_size[0] = size[0];
	hdr.o_size[1] = size[1];
	hdr.o_size[2] = size[2];

	rewind(op);
	if (ldmode != LD_ABSOLUTE)
		fwrite(&hdr, sizeof(hdr), 1, op);
	/* For LD_RFLAG number the symbols for output, for othe forms
	   check for unknowmn symbols and error them out */
	if (ldmode != LD_ABSOLUTE)
		renumber_symbols();
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
	if (err == 0 && ldmode != LD_ABSOLUTE) {
		xfseek(op, 0);
		fwrite(&hdr, sizeof(hdr), 1, op);
	}
}

/*
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
 *	1.	We need to support library objects. A library object is only
 *		linked if it provides a symbol someone requires
 *	2.	Libraries are stored in .a files so we need to support walking
 *		down a library (hence the offset argument to the loader)
 *	3.	For speed libraries can stat with an _RANLIB ar file node whic
 *		is an index of all the symbols by library module for speed.
 */

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
		load_object(fp, pos, 1, name);
		pos += size;
		if (pos & 1)
			pos++;
	}
}

static void add_object(const char *name, off_t off, int lib)
{
	static char x[SARMAG];
	FILE *fp = xfopen(name, "r");
	if (off == 0 && !lib) {
		/* Is it a bird, is it a plane ? */
		fread(x, SARMAG, 1, fp);
		if (memcmp(x, ARMAG, SARMAG) == 0) {
			/* No it's a library */
			process_library(name, fp);
			xfclose(fp);
			return;
		}
	}
	xfseek(fp, off);
	load_object(fp, off, lib, name);
	xfclose(fp);
}

int main(int argc, char *argv[])
{
	int opt;
	FILE *bp, *mp = NULL;

	arg0 = argv[0];

	while ((opt = getopt(argc, argv, "rbvtsio:m:c:")) != -1) {
		switch (opt) {
		case 'r':
			ldmode = LD_RFLAG;
			break;
		case 'b':
			ldmode = LD_ABSOLUTE;
		case 'v':
			printf("FuzixLD 0.1\n");
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
		case 's':
			strip = 1;
			break;
		case 'i':
			split_id = 1;
			break;
		case 'c':
			base[1] = atoi(optarg);
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
