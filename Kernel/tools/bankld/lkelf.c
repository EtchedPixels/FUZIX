/* lkelf.c - Create an executable ELF/DWARF file

   Copyright (C) 2004 Erik Petrich, epetrich at users dot sourceforge dot net

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <asxxxx_config.h>

#include "aslink.h"

static int execStartMSB;
static int execStartLSB;
static char execStartMSBfound;
static char execStartLSBfound;

typedef TYPE_UDWORD Elf32_Addr;
typedef TYPE_WORD Elf32_Half;
typedef TYPE_UDWORD Elf32_Off;
typedef TYPE_DWORD Elf32_Sword;
typedef TYPE_UDWORD Elf32_Word;

enum
{
  EI_MAG0 = 0,
  EI_MAG1,
  EI_MAG2,
  EI_MAG3,
  EI_CLASS,
  EI_DATA,
  EI_VERSION,
  EI_PAD,
  EI_NIDENT = 16
};

enum
{
  ELFMAG0 = 0x7f,
  ELFMAG1 = 'E',
  ELFMAG2 = 'L',
  ELFMAG3 = 'F'
};

enum
{
  ET_NONE = 0,
  ET_REL,
  ET_EXEC,
  ET_DYN,
  ET_CORE
};

/* These e_machine values are from "Motorola 8- and 16-bit Embedded */
/* Application Binary Interface (M8/16EABI)" version 2.0 */
enum
{
  EM_NONE = 0,
  EM_68HC05 = 72,
  EM_68HC08 = 71,
  EM_68HC11 = 70,
  EM_68HC12 = 53,
  EM_68HC16 = 69
};

enum
{
  EV_NONE = 0,
  EV_CURRENT
};

enum
{
  ELFCLASSNONE = 0,
  ELFCLASS32,
  ELFCLASS64
};

enum
{
  ELFDATANONE = 0,
  ELFDATA2LSB,
  ELFDATA2MSB
};

enum
{
  SHT_NULL = 0,
  SHT_PROGBITS,
  SHT_SYMTAB,
  SHT_STRTAB,
  SHT_RELA,
  SHT_HASH,
  SHT_DYNAMIC,
  SHT_NOTE,
  SHT_NOBITS,
  SHT_REL,
  SHT_SHLIB,
  SHT_DYNSYM
};

enum
{
  SHF_WRITE = (1 << 0),
  SHF_ALLOC = (1 << 1),
  SHF_EXECINSTR = (1 << 2),
};

enum
{
  PT_NULL = 0,
  PT_LOAD
};

enum
{
  PF_X = (1 << 0),
  PF_W = (1 << 1),
  PF_R = (1 << 2)
};

typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct
{
  Elf32_Word sh_name;
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct
{
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;

typedef struct strtabString
{
  char * string;
  struct strtabString * prev;
  struct strtabString * next;
  Elf32_Word index;
} strtabString;

typedef struct
{
  strtabString * first;
  strtabString * last;
} strtabList;

static strtabList shstrtab;


typedef struct listEntry
{
  void * item;
  struct listEntry * prev;
  struct listEntry * next;
} listEntry;

typedef struct
{
  listEntry * first;
  listEntry * last;
  int count;
} listHeader;



static void
listAdd (listHeader * lhp, void * item)
{
  listEntry * lep;

  lep = (listEntry *)new (sizeof (*lep));
  lep->item = item;
  lep->prev = lhp->last;
  if (lep->prev)
    lep->prev->next = lep;

  lhp->last = lep;
  if (!lhp->first)
    lhp->first = lep;

  lhp->count++;
}

static listHeader *
listNew (void)
{
  listHeader * lhp;

  lhp = (listHeader *)new (sizeof (*lhp));

  return lhp;
}


#if 0
static Elf32_Word
strtabFind (strtabList * strtab, char * str)
{
  strtabString * sp;
  sp = strtab->first;

  while (sp)
    {
      if (!strcmp (str, sp->string))
        return sp->index;
      sp = sp->next;
    }

  return 0;
}
#endif

/*-------------------------------------------------------------------*/
/* strtabFindOrAdd - Finds a string in a string table or adds the    */
/*   string if it does not already exist. Returns the offset of the  */
/*   string in the table.                                            */
/*-------------------------------------------------------------------*/
static Elf32_Word
strtabFindOrAdd (strtabList * strtab, char * str)
{
  strtabString * sp;
  sp = strtab->first;

  while (sp)
    {
      if (!strcmp (str, sp->string))
        return sp->index;
      sp = sp->next;
    }

  sp = (strtabString *)new (sizeof(*sp));
  if (strtab->last)
    sp->index = strtab->last->index + 1 + strlen (strtab->last->string);
  else
    sp->index = 1;
  sp->string = new (1+strlen (str));
  strcpy (sp->string, str);

  sp->prev = strtab->last;
  if (sp->prev)
    sp->prev->next = sp;
  strtab->last = sp;
  if (!strtab->first)
    strtab->first = sp;

  return sp->index;
}

/*-------------------------------------------------------------------*/
/* fputElfStrtab - writes a string table to a file                   */
/*-------------------------------------------------------------------*/
static void
fputElfStrtab (strtabList *strtab, FILE *fp)
{
  strtabString * sp;

  fputc (0, fp);        /* index 0 must be the null character */

  sp = strtab->first;
  while (sp)
    {
      fputs (sp->string, fp);
      fputc (0, fp);
      sp = sp->next;
    }
}

/*-------------------------------------------------------------------*/
/* fputElf32_Word - writes an Elf32_Word value to a file             */
/*-------------------------------------------------------------------*/
static void
fputElf32_Word (Elf32_Word x, FILE *fp)
{
  if (hilo == 0)
    {
      fputc (x & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 24) & 0xff, fp);
    }
  else
    {
      fputc ((x >> 24) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc (x & 0xff, fp);
    }
}

/*-------------------------------------------------------------------*/
/* fputElf32_Off - writes an Elf32_Off value to a file               */
/*-------------------------------------------------------------------*/
static void
fputElf32_Off (Elf32_Off x, FILE *fp)
{
  if (hilo == 0)
    {
      fputc (x & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 24) & 0xff, fp);
    }
  else
    {
      fputc ((x >> 24) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc (x & 0xff, fp);
    }
}

/*-------------------------------------------------------------------*/
/* fputElf32_Addr - writes an Elf32_Addr value to a file             */
/*-------------------------------------------------------------------*/
static void
fputElf32_Addr (Elf32_Addr x, FILE *fp)
{
  if (hilo == 0)
    {
      fputc (x & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 24) & 0xff, fp);
    }
  else
    {
      fputc ((x >> 24) & 0xff, fp);
      fputc ((x >> 16) & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
      fputc (x & 0xff, fp);
    }
}

/*-------------------------------------------------------------------*/
/* fputElf32_Half - writes an Elf32_Half value to a file             */
/*-------------------------------------------------------------------*/
static void
fputElf32_Half (Elf32_Half x, FILE *fp)
{
  if (hilo == 0)
    {
      fputc (x & 0xff, fp);
      fputc ((x >> 8) & 0xff, fp);
    }
  else
    {
      fputc ((x >> 8) & 0xff, fp);
      fputc (x & 0xff, fp);
    }
}

/*------------------------------------------------------------------------*/
/* fputElf32_Ehdr - writes an Elf32_Ehdr struct (ELF header) to a file    */
/*------------------------------------------------------------------------*/
static void
fputElf32_Ehdr (Elf32_Ehdr * ehdr, FILE * fp)
{
  int i;

  for (i=0; i<EI_NIDENT; i++)
    fputc (ehdr->e_ident[i], fp);

  fputElf32_Half (ehdr->e_type, fp);
  fputElf32_Half (ehdr->e_machine, fp);
  fputElf32_Word (ehdr->e_version, fp);
  fputElf32_Addr (ehdr->e_entry, fp);
  fputElf32_Off (ehdr->e_phoff, fp);
  fputElf32_Off (ehdr->e_shoff, fp);
  fputElf32_Word (ehdr->e_flags, fp);
  fputElf32_Half (ehdr->e_ehsize, fp);
  fputElf32_Half (ehdr->e_phentsize, fp);
  fputElf32_Half (ehdr->e_phnum, fp);
  fputElf32_Half (ehdr->e_shentsize, fp);
  fputElf32_Half (ehdr->e_shnum, fp);
  fputElf32_Half (ehdr->e_shstrndx, fp);
}

/*-------------------------------------------------------------------------*/
/* fputElf32_Ehdr - writes an Elf32_Shdr struct (section header) to a file */
/*-------------------------------------------------------------------------*/
static void
fputElf32_Shdr (Elf32_Shdr * shdr, FILE * fp)
{
  fputElf32_Word (shdr->sh_name, fp);
  fputElf32_Word (shdr->sh_type, fp);
  fputElf32_Word (shdr->sh_flags, fp);
  fputElf32_Addr (shdr->sh_addr, fp);
  fputElf32_Off (shdr->sh_offset, fp);
  fputElf32_Word (shdr->sh_size, fp);
  fputElf32_Word (shdr->sh_link, fp);
  fputElf32_Word (shdr->sh_info, fp);
  fputElf32_Word (shdr->sh_addralign, fp);
  fputElf32_Word (shdr->sh_entsize, fp);
}

/*-------------------------------------------------------------------------*/
/* fputElf32_Ehdr - writes an Elf32_Phdr struct (segment header) to a file */
/*-------------------------------------------------------------------------*/
static void
fputElf32_Phdr (Elf32_Phdr * phdr, FILE * fp)
{
  fputElf32_Word (phdr->p_type, fp);
  fputElf32_Off (phdr->p_offset, fp);
  fputElf32_Addr (phdr->p_vaddr, fp);
  fputElf32_Addr (phdr->p_paddr, fp);
  fputElf32_Word (phdr->p_filesz, fp);
  fputElf32_Word (phdr->p_memsz, fp);
  fputElf32_Word (phdr->p_flags, fp);
  fputElf32_Word (phdr->p_align, fp);
}


/*--------------------------------------------------------------------------*/
/* elfGenerateAbs - generates segments and sections for an absolute area.   */
/*   This is a little more complicated than a relative area since it may    */
/*   contain noncontiguous regions.                                         */
/*--------------------------------------------------------------------------*/
static void
elfGenerateAbs (struct area *ap, listHeader * segments, listHeader * sections)
{
  Elf32_Addr ofs;
  Elf32_Addr addr;
  Elf32_Word size;
  Elf32_Phdr * phdrp;
  Elf32_Shdr * shdrp;

  if (!ap->a_image)
    {
      return;
    }

  ofs = 0;
  for (;;)
    {
      /* Find the start of a contiguously */
      /* used region within this area */
      while (ofs < ap->a_imagesize && !ap->a_used[ofs])
        ofs++;
      if (ofs >= ap->a_imagesize)
        return;

      /* Find the end of the region */
      addr = ap->a_addr + ofs;
      while (ofs < ap->a_imagesize && ap->a_used[ofs])
        ofs++;
      size = ap->a_addr + ofs - addr;

      /* create a segment header for this region if loadable */
      if (!(ap->a_flag & A_NOLOAD))
        {
          phdrp = (Elf32_Phdr *)new (sizeof (*phdrp));
          phdrp->p_type = PT_LOAD;
          phdrp->p_offset = ftell (ofp);
          phdrp->p_vaddr = addr;
          phdrp->p_paddr = addr;
          phdrp->p_filesz = size;
          phdrp->p_memsz = size;
          phdrp->p_flags = PF_R;
          if (ap->a_flag & A_CODE)
            phdrp->p_flags |= PF_X;
          phdrp->p_align = 1;
          listAdd (segments, phdrp);
        }

      /* create a section header for this region */
      shdrp = (Elf32_Shdr *)new (sizeof (*shdrp));
      shdrp->sh_name = strtabFindOrAdd (&shstrtab, ap->a_id);
      shdrp->sh_type = SHT_PROGBITS;
      shdrp->sh_flags = 0;
      if (!(ap->a_flag & A_NOLOAD))
        shdrp->sh_flags |= SHF_ALLOC;
      if (ap->a_flag & A_CODE)
        shdrp->sh_flags |= SHF_EXECINSTR;
      shdrp->sh_addr = addr;
      shdrp->sh_offset = ftell (ofp);
      shdrp->sh_size = size;
      shdrp->sh_link = 0;
      shdrp->sh_info = 0;
      shdrp->sh_addralign = 0;
      shdrp->sh_entsize = 0;
      listAdd (sections, shdrp);

      fwrite (&ap->a_image[addr-ap->a_addr], 1, size, ofp);
    }
}

/*--------------------------------------------------------------------------*/
/* elfGenerateRel - generates a segment and section for a relative area.    */
/*--------------------------------------------------------------------------*/
static void
elfGenerateRel (struct area *ap, listHeader * segments, listHeader * sections)
{
  Elf32_Phdr * phdrp;
  Elf32_Shdr * shdrp;

  if (!ap->a_image)
    {
      return;
    }

  /* create a segment header for this area if loadable */
  if (!(ap->a_flag & A_NOLOAD))
    {
      phdrp = (Elf32_Phdr *)new (sizeof (*phdrp));
      phdrp->p_type = PT_LOAD;
      phdrp->p_offset = ftell (ofp);
      phdrp->p_vaddr = ap->a_addr;
      phdrp->p_paddr = ap->a_addr;
      phdrp->p_filesz = ap->a_size;
      phdrp->p_memsz = ap->a_size;
      phdrp->p_flags = PF_R;
      if (ap->a_flag & A_CODE)
        phdrp->p_flags |= PF_X;
      phdrp->p_align = 1;
      listAdd (segments, phdrp);
    }

  /* create a section header for this area */
  shdrp = (Elf32_Shdr *)new (sizeof (*shdrp));
  shdrp->sh_name = strtabFindOrAdd (&shstrtab, ap->a_id);
  shdrp->sh_type = SHT_PROGBITS;
  shdrp->sh_flags = 0;
  if (!(ap->a_flag & A_NOLOAD))
    shdrp->sh_flags |= SHF_ALLOC;
  if (ap->a_flag & A_CODE)
    shdrp->sh_flags |= SHF_EXECINSTR;
  shdrp->sh_addr = ap->a_addr;
  shdrp->sh_offset = ftell (ofp);
  shdrp->sh_size = ap->a_size;
  shdrp->sh_link = 0;
  shdrp->sh_info = 0;
  shdrp->sh_addralign = 0;
  shdrp->sh_entsize = 0;
  listAdd (sections, shdrp);

  fwrite (ap->a_image, 1, ap->a_size, ofp);
}

/*--------------------------------------------------------------------------*/
/* elfGenerate - generates the complete ELF file                            */
/*--------------------------------------------------------------------------*/
static void
elfGenerate (void)
{
  listHeader * sections = listNew();
  listHeader * segments = listNew();
  struct area *ap;
  Elf32_Ehdr ehdr;
  Elf32_Shdr * shdrp;
  Elf32_Phdr * phdrp;
  listEntry * lep;
  int i;
  Elf32_Word shstrtabName;

  /* create the null section header for index 0 */
  shdrp = (Elf32_Shdr *)new (sizeof (*shdrp));
  shdrp->sh_name = 0;
  shdrp->sh_type = SHT_NULL;
  shdrp->sh_flags = 0;
  shdrp->sh_addr = 0;
  shdrp->sh_offset = 0;
  shdrp->sh_size = 0;
  shdrp->sh_link = 0;
  shdrp->sh_info = 0;
  shdrp->sh_addralign = 0;
  shdrp->sh_entsize = 0;
  listAdd (sections, shdrp);

  /* Initialize the ELF header */
  for (i=0; i<EI_NIDENT; i++)
    ehdr.e_ident[i] = 0;
  ehdr.e_ident[EI_MAG0] = ELFMAG0;
  ehdr.e_ident[EI_MAG1] = ELFMAG1;
  ehdr.e_ident[EI_MAG2] = ELFMAG2;
  ehdr.e_ident[EI_MAG3] = ELFMAG3;
  ehdr.e_ident[EI_CLASS] = ELFCLASS32;
  if (hilo == 0)
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
  else
    ehdr.e_ident[EI_DATA] = ELFDATA2MSB;
  ehdr.e_ident[EI_VERSION] = 1;
  ehdr.e_type = ET_EXEC;
  ehdr.e_machine = EM_68HC08; /* FIXME: get rid of hardcoded value - EEP */
  ehdr.e_phentsize = sizeof (*phdrp);
  ehdr.e_shentsize = sizeof (*shdrp);
  ehdr.e_ehsize = sizeof (ehdr);
  ehdr.e_phnum = 0;
  ehdr.e_shnum = 0;
  ehdr.e_shstrndx = 0;
  ehdr.e_version = 1;
  ehdr.e_entry = 0;
  if (execStartMSBfound && execStartLSBfound)
    ehdr.e_entry = (execStartMSB << 8) + execStartLSB;

  /* Write out the ELF header as a placeholder; we will update */
  /* it with the final values when everything is complete */
  fputElf32_Ehdr (&ehdr, ofp);

  /* Iterate over the linker areas to generate */
  /* the ELF sections and segments */
  ap = areap;
  while (ap)
    {
      if (ap->a_size)
        {
          if (ap->a_flag & A3_ABS)
            elfGenerateAbs (ap, segments, sections);
          else
            elfGenerateRel (ap, segments, sections);
        }
      ap = ap->a_ap;
    }

  /* Create the string table section after the other sections */
  shdrp = (Elf32_Shdr *)new (sizeof (*shdrp));
  shdrp->sh_name = strtabFindOrAdd (&shstrtab, ".shstrtab");
  shdrp->sh_type = SHT_STRTAB;
  shdrp->sh_flags = 0;
  shdrp->sh_addr = 0;
  shdrp->sh_offset = ftell (ofp);
  shdrp->sh_size = shstrtab.last->index + strlen (shstrtab.last->string) + 1;
  shdrp->sh_link = 0;
  shdrp->sh_info = 0;
  shdrp->sh_addralign = 0;
  shdrp->sh_entsize = 0;
  listAdd (sections, shdrp);
  fputElfStrtab (&shstrtab, ofp);

  /* Find the index of the section string table */
  /* header and save it in the ELF header */
  ehdr.e_shstrndx = 0;
  shstrtabName = shdrp->sh_name;
  lep = sections->first;
  while (lep)
    {
      shdrp = lep->item;
      if (shdrp->sh_name == shstrtabName)
        break;
      ehdr.e_shstrndx++;
      lep = lep->next;
    }

  /* Write out the segment headers */
  ehdr.e_phnum = segments->count;
  ehdr.e_phoff = ftell (ofp);
  lep = segments->first;
  while (lep)
    {
      phdrp = lep->item;
      fputElf32_Phdr (phdrp, ofp);
      lep = lep->next;
    }

  /* Write out the section headers */
  ehdr.e_shnum = sections->count;
  ehdr.e_shoff = ftell (ofp);
  lep = sections->first;
  while (lep)
    {
      shdrp = lep->item;
      fputElf32_Shdr (shdrp, ofp);
      lep = lep->next;
    }

  /* All the values in the ELF header have now been computed; write */
  /* over the placeholder header with the final values */
  fseek (ofp, 0, SEEK_SET);
  fputElf32_Ehdr (&ehdr, ofp);
  fseek (ofp, 0, SEEK_END);
}

/*--------------------------------------------------------------------------*/
/* elf - incrementally called by the linker core to generate ELF file data. */
/*    The parameter is nonzero when there is data available and zero when   */
/*    the linker is finished.                                               */
/*--------------------------------------------------------------------------*/
void
elf (int i)
{
  a_uint address;

  /* Buffer the data until we have it all */
  if (i)
    {
      if (hilo == 0)
        address = rtval[0] + (rtval[1] << 8); /* little endian order */
      else
        address = rtval[1] + (rtval[0] << 8); /* big endian order */

      /* If this area doesn't have an image buffer, create one */
      if (!ap->a_image)
        {
          if (ap->a_flag & A3_ABS)
            ap->a_imagesize = ap->a_addr + ap->a_size;
          else
            ap->a_imagesize = ap->a_size;
          ap->a_image = new (ap->a_imagesize);
          if (ap->a_flag & A3_ABS)
            ap->a_used = new (ap->a_imagesize);
        }

      /* Copy the data into the image buffer */
      for (i = 2; i < rtcnt ; i++)
        {
          if (rtflg[i])
            {
              if (address-ap->a_addr >= ap->a_imagesize)
                {
                  a_uint newsize;

                  if (ap->a_flag & A3_ABS)
                    {
                      newsize = ap->a_imagesize;
                      while (address-ap->a_addr >= newsize)
                        newsize = (newsize & ~4095)+4096;
                      ap->a_image = (char *) realloc (ap->a_image, newsize);
                      ap->a_used = (char *) realloc (ap->a_used, newsize);
                      if (!ap->a_image || !ap->a_used)
                        {
                          fprintf (stderr, "Out of space!\n");
                          lkexit (ER_FATAL);
                        }
                      memset (ap->a_image+ap->a_imagesize, 0, newsize-ap->a_imagesize);
                      memset (ap->a_used+ap->a_imagesize, 0, newsize-ap->a_imagesize);
                      ap->a_imagesize = newsize;
                    }
                  else
                    {
                      fprintf (stderr, "Unexpected area %s overflow. Address = 0x%x but allocated range is 0x%x - 0x%x\n",
                               ap->a_id, address, ap->a_addr, ap->a_addr+ap->a_imagesize-1);
                      lkexit (ER_FATAL);
                    }
                }
              ap->a_image[address-ap->a_addr] = rtval[i];
              if (ap->a_used)
                ap->a_used[address-ap->a_addr] = 1;

              /* Make note of the reset vector */
              if (!(ap->a_flag & A_NOLOAD))
                {
                  if (address == 0xfffe)
                    {
                      execStartMSB = rtval[i];
                      execStartMSBfound = 1;
                    }
                  if (address == 0xffff)
                    {
                      execStartLSB = rtval[i];
                      execStartLSBfound = 1;
                    }
                }
              address++;
            }
        }
    }
  else
    elfGenerate();
}
