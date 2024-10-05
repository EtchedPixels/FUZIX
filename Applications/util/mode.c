/*
 *	Video mode setting
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/graphics.h>

static struct display disp;
static char *appname;

static int force;
static int list;
static int info;
static int set;

static void format(int n) {
  switch(n) {
    case FMT_MONO_BW:
      printf("mono black on white");
      break;
    case FMT_MONO_WB:
      printf("mono white on black");
      break;
    case FMT_COLOUR4:
      printf("four colour");
      break;
    case FMT_TEXT:
      printf("text");
      break;
    case FMT_COLOUR16:
      printf("sixteen colour");
      break;
    case FMT_MONO_WB_TILE8:
      printf("8x8 tiled white on black mono");
      break;
    case FMT_6PIXEL_128:
      printf("2x3 block graphics, char 128 based");
      break;
    case FMT_4PIXEL_128:
      printf("2x2 block graphics, char 128 based");
      break;
    case FMT_PLANAR2:
      printf("two bitplane");
      break;
    case FMT_PLANAR4:
      printf("four bitplane");
      break;
    case FMT_SPECTRUM:
      printf("Sinclair Spectrum");
      break;
    case FMT_VDP:
      printf("VDP");
      break;
    case FMT_UBEE:
      printf("UBEE");
      break;
    case FMT_SAM2:
      printf("SAM2");
      break;
    case FMT_TIMEX64:
      printf("Timex interleaved");
      break;
    case FMT_AMSMONO:
      printf("Amstrad mono");
      break;
    case FMT_AMS4:
      printf("Amstrad four colour");
      break;
    case FMT_AMS16:
      printf("Amstrad 16 colour");
      break;
    case FMT_8PIXEL_MTX:
      printf("MTX symbols");
      break;
    case FMT_3BPP_U16:
      printf("3bpp word packed");
      break;
    case FMT_THOMSON_C16:
      printf("Thomson 16 colour");
      break;
    case FMT_THOMSON_TO7:
      printf("Thomson TO7");
      break;
    default:
      printf("Unknown(%d)", n);
  }
}

static void decode_mode_c(int c)
{
  if (!info) {
    printf("%d\n", disp.mode);
    return;
  }
  printf("Mode: %d%s\n", disp.mode, c == disp.mode ? " (Current)":"");
  printf("Size: %d x %d (%d x %d) [%d x %d]\n",
      disp.width, disp.height, disp.stride, disp.lines,
      disp.twidth, disp.theight);
  printf("Format: ");
  format(disp.format);
  printf("\nHardware: ");
  switch(disp.hardware) {
    case HW_UNACCEL:
      printf("unaccelerated framebuffer");
      break;
    case HW_VDP_9918A:
      printf("9918A");
      break;
    case HW_VDP_9938:
      printf("9938");
      break;
    case HW_TRS80GFX:
      printf("TRS80 Hi-Res Board");
      break;
    default:
      printf("Unknown (%d)", disp.hardware);
      break;
  }
  printf("\nFeatures: ");
  if (disp.features & GFX_MAPPABLE)
    printf("mappable ");
  if (disp.features & GFX_PALETTE)
    printf("palette ");
  if (disp.features & GFX_OFFSCREEN)
    printf("offscreen ");
  if (disp.features & GFX_VBLANK)
    printf("vblank ");
  if (disp.features & GFX_MULTIMODE)
    printf("multimode ");
  if (disp.features & GFX_PALETTE_SET)
    printf("setpalette ");
  if (disp.features & GFX_TEXT)
    printf("text ");
  printf("\n");
  if (disp.memory)
    printf("Memory: %dK\n", disp.memory);
  printf("Commands: ");
  if (disp.commands & GFX_DRAW)
    printf("draw ");
  if (disp.commands & GFX_RAW)
    printf("raw ");
  if (disp.commands & GFX_ADRAW)
    printf("adraw ");
  if (disp.commands & GFX_CLIP)
    printf("clip ");
  if (disp.commands & GFX_BLIT)
    printf("blit ");
  if (disp.commands & GFX_READ)
    printf("read ");
  if (disp.commands & GFX_PDRAW)
    printf("pdraw ");
  if (disp.commands & GFX_WRITE)
    printf("write ");
  if (disp.commands & GFX_AWRITE)
    printf("awrite ");
  if (disp.commands & GFX_EXG)
    printf("exchange ");
  if (disp.commands & GFX_SCROLL)
    printf("scroll ");
  printf("\n");
}

static void decode_mode(void)
{
  decode_mode_c(-1);
}

static void ioctl_err(void)
{
  if (errno == ENOTTY)
    fprintf(stderr, "%s: not graphics capable.\n", appname);
  else 
    fprintf(stderr, "%s: mode not supported.\n", appname);
  exit(1);
}

static void show_mode(int m) {
  disp.mode = m;

  if (ioctl(0, GFXIOC_GETMODE, &disp) < 0)
    ioctl_err();
  decode_mode();
}

static void show_current(void) {
  if (ioctl(0, GFXIOC_GETINFO, &disp) < 0)
    ioctl_err();
  decode_mode();
}

static void list_modes(void) {
  int i = 0;
  int c = disp.mode;
  
  if (ioctl(0, GFXIOC_GETINFO, &disp) < 0)
    ioctl_err();

  while(1) {
    disp.mode = i;
    if (ioctl(0, GFXIOC_GETMODE, &disp) < 0)
      break;
    decode_mode_c(c);
    i++;
  }
}

static void set_mode(int m) {
  struct winsize ws;
  disp.mode = m;
  if (ioctl(0, GFXIOC_GETMODE, &disp))
    ioctl_err();
  if (!(disp.features & GFX_TEXT) && !force) {
    fprintf(stderr, "%s: no text mode support, use -f to force set\n",
      appname);
    exit(1);
  }
  if (ioctl(0, GFXIOC_SETMODE, &disp) < 0)
    ioctl_err();
  ws.ws_xpixel = disp.width;
  ws.ws_ypixel = disp.height;
  ws.ws_col = disp.twidth;
  ws.ws_row = disp.theight;
  if (ioctl(0, TIOCSWINSZ, &ws) < 0)
    ioctl_err();
}

static void error(void)
{
  fprintf(stderr, "%s: [-f] [-l] [-s  mode] [-i mode mode ...]\n", appname);
  exit(1);
}

int main(int argc, char *argv[])
{
  char **argp = argv;
  appname = argv[0];
  
  while(*++argp && (*argp)[0] == '-') {
    switch((*argp)[1]) {
      case 'f':
        force = 1;
        break;
      case 's':
        set = 1;
        break;
      case 'i':
        info = 1;
        break;
      case 'l':
        list = 1;
        break;
      default:
        error();
    }
  }
  if (set) {
    int f = atoi(*argp++);
    if (*argp)
      error();
    set_mode(f);
    return 0;
  }
  if (list && !*argp) {
    list_modes();
    return 0;
  }
  if (!*argp)
    show_current();
  else while(*argp) {
    int f = atoi(*argp++);
    if (!list)
      info = 1;
    show_mode(f);
  }
  return 0;
}
