#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define EMULATOR_SIZE	0x0900

static char path[] = "/usr/lib/cpm/emulator.bin";

void writes(const char *p)
{
  write(2, p, strlen(p));
}

/* Stack space is valuable constant space is almost free */
struct stat st;
unsigned char buf[16];
unsigned char *basep;
char pathbuf[512];
uint8_t image[EMULATOR_SIZE];
char *pathp;
int fd;
int len, len2;
uint8_t *fcb = (uint8_t *)0x5C;
uint8_t *tail = (uint8_t *)0x81;
uint8_t *tailsize = (uint8_t *)0x80;
uint8_t *cpmtop;
char **argp;
int nfcb;


/*
 *	Take the loaded emulator, stuff it as high up as possible with the
 *	stack pointer below it.
 */
void to_emulator(void) __naked
{
__asm
                ld hl,(__call_sys+1)	; syscall addr
                ld (0x31),hl		; save it (0x31 for historic reasons FIXME)
		ld hl,(_image + 6)	; code size (and copy size)
		ld b,h
		ld c,l			; Save copy size
		; Don't bother adding in BSS/DATA we don't have any and
		; we leave the reloc table in image
		ex de,hl
		; DE is now the total image size space we need
		ld hl,(_cpmtop)
		or a
		sbc hl,de		; Proposed binary base
		ld l,#0			; Align on a page boundary
		ex de,hl		; DE is now the image destination

		ld hl,#_image
		push de
		ldir
		pop de
                push de
		;
		;	Relocator
		;	DE = relocation address
		;
		ld hl,#_image
		ld bc,(_image + 6)
		add hl,bc		; Find the base of the relocations
		ld b,#0

		push de
		exx
		pop de		; relocation base into DE and alt DE
                exx
		ld b,#0		; code base
		ex de,hl	; de is relocations as loop swaps

		;
		;	Taken from the loader
		;
relnext:
		; Read each relocation byte from image and apply it to the
		; CP/M emulator binary
		ld a,(de)
		inc de
		; 0 means done, 255 means skip 254, 254 or less means
		; skip that many and relocate  (runs are 255,255,....)
		or a		; 0 ?
		jr z, relocdone
		ld c,a
		inc a		; 255 ?
		jr z, relocskip
		add hl,bc
		ld a,(hl)
		exx
		add d
		exx
		ld (hl),a
		jr relnext
relocskip:	add hl,bc
		dec hl
		jr relnext
relocdone:
		;
		;	Relocation done
		;
                pop de	; base of relocated image
                ld hl,#16
                add hl,de
                jp (hl)	; entry point

__endasm;
}


void prepare_fcb(char *p)
{
  int n = 0;
  char c = toupper(*p);
  /* We don't parse user codes : FIXME ? */
  if (p[1] == ':' && c <='A' && c >='Z') {
    *fcb = c - 'A' + 1;
    p += 2;
  }
  while(*p && !isspace(*p) && *p != '.' && n < 8)
    fcb[++n] = *p++;
  if (*p == '.') {
    p++;
    n = 8;
    while(*p && !isspace(*p) && *p != '.' && n < 11)
      fcb[++n] = *p++;
  }
  fcb += 16;
}

int main(int argc, char *argv[])
{
  uint8_t top;
  if (argc < 2) {
    writes("runcpm app.com [args...]\n");
    exit(1);
  }

  /* Quick idiot trap */
  if ((uint16_t)main >= 0x1000U) {
    writes("runcpm: platform does not support CP/M emulation.\n");
    exit(1);
  }

  if (((uint8_t *)sbrk(0)) + 32768 + EMULATOR_SIZE >= &top) {
    writes("runcpm: insufficient memory.\n");
    exit(1);
  }

  fd = open(path, O_RDONLY);
  if (fd == -1) {
    writes("runcpm: emulator.bin missing\n");
    exit(1);
  }
  if (read(fd, image, EMULATOR_SIZE) < 2200) {
    writes("runcpm: emulator.bin too short\n");
    exit(1);
  }
  if (image[0] != 0xA9 || image[1] != 0x80) {
    writes("runcpm: emulator.bin invalid\n");  
    exit(1);
  }

  close(fd);

  /* FIXME: worth having a CP/M search path ? */

  pathp = strrchr(argv[1], '/');
  if (pathp == NULL)
    pathp = argv[1];
  pathp = strrchr(argv[1], '.');

  /* Has a .com extension so honour it */
  if (pathp && strcasecmp(pathp, ".com") == 0)
    fd = open(argv[1], O_RDONLY);
  else {
    len = strlen(argv[1]);
    pathp = pathbuf + strlcpy(pathbuf, argv[1], sizeof(pathbuf));
    strlcpy(pathp, ".com", sizeof(pathbuf) - len);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
      strlcpy(pathp, ".COM", sizeof(pathbuf) - len);
      fd = open(path, O_RDONLY);
      if (fd == -1) {
        writes("runcpm: unable to find .COM file\n");
        exit(1);
      }
    }
  }
  if (fd == -1 || fstat(fd, &st) == -1) {
    perror(argv[1]);
    exit(1);
  }
  if (!S_ISREG(st.st_mode) || st.st_size < 256) {
    writes(argv[1]);
    writes(" does not appear to be a CP/M .COM file\n");
    exit(1);
  }
  /* Make sure we pass the binary as fd 3 */
  if (fd != 3) {
    dup2(fd, 3);
    close(fd);
  }
  /* Now build the FCB and argument area in C so we can keep as much in portable
     discardable code as we can */
  memset(fcb, 0, 16);
  memset(fcb + 1, ' ', 11);

  argp = argv + 2;
  len = 0;

  /* Linearize arguments that will fit */
  while(*argp) {
    len2 = strlen(*argp);
    if (len + len2 > 126)
      break;
    if (nfcb++ < 2)
      prepare_fcb(*argp);
    memcpy(tail, *argp, len2);
    tail += len2;
    *tail++ = ' ';
    len += len2 + 1;
    argp++;
  }
  *tailsize = len;

  /* CP/M apps don't respect our normal rules of the road. Make sure we
     brk right up high so that the OS resource handling behaves. */
  cpmtop = &top - 560;
  if (brk(cpmtop)) {
	writes("runcpm: unable to allocate workspace\n");
	exit(1);
  }
  /* If this works it blows us away and we never return */
  to_emulator();
  writes("runcpm: exec failure\n");
  exit(1);
}
