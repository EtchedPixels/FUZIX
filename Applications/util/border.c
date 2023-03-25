#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <libgen.h>

static const char *cmd;

static void error(const char *e)
{
    write(1, cmd, strlen(cmd));
    write(1, ": ", 2);
    write(1, e, strlen(e));
    write(1, ".\n", 2);
    exit(1);
}

struct ctab {
    const char *name;
    uint8_t code;
};

static struct ctab ctab[] = {
    { "black", 0 },
    { "blue", 1 },
    { "red", 2 },
    { "magneta", 3 },
    { "green", 4 },
    { "cyan", 5 },
    { "brown", 6 },
    { "grey", 7 },
    { "gray", 7 },
    { "light grey", 7 },
    { "light gray", 7 },
    { "dark grey", 8 },
    { "dark gray", 8 },
    { "light blue", 9 },
    { "light red", 10 },
    { "light magneta", 11 },
    { "light green", 12 },
    { "light cyan", 13 },
    { "yellow", 14 },
    { "white", 15 }
};

static uint8_t hexbits(const char *p)
{
    uint8_t n = *p - '0';
    if (n > 9)
        n -= 'A' - '9' + 1;
    if (n > 15)
        error("invalid hex code");
    return n;
}

/* Map modern HTML style colours to IGRB */
static uint8_t makergbi(const char *p)
{
    uint8_t r = hexbits(p);
    uint8_t g = hexbits(p + 2);
    uint8_t b = hexbits(p + 4);
    uint8_t c;
    c = (b >> 7) | ((r >> 6) & 2) | ((g >> 5) & 4);
    if (g & 0x80)
        c |= 0x08;
    if (r & b & 0x80)
        c |= 0x08;
    return c;
}

static uint8_t colourmap(const char *p)
{
    struct ctab *n = ctab;
    unsigned v;
    if (isdigit(*p)) {
        v = atoi(p);
        if (v < 0 || v > 31)
            return -1;
        return v;
    }
    if (*p == '#')
        return makergbi(p+1);
    /* Try names */
    while(n->name) {
        if (strcasecmp(n->name, p) == 0)
            return n->code;
        n++;
    }
    return 0xFF; 
}

static int colour_or_vtcode(uint8_t colour, int ioc, const char *vt)
{
    if (ioctl(0, ioc, &colour) == -1) {
        write(1, vt, 2);
        colour |= 0x40;		/* ASCII bit */
        write(1, &colour, 1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    uint8_t n;
    cmd = basename(argv[0]);
    if (argc != 2)
        error("colour required");
    if (ioctl(0, VTSIZE, NULL) == -1)
        error("not a console");
    n = colourmap(argv[1]);
    if (n == 0xFF)
        error("unknown colour");

    if (strcmp(cmd, "border") == 0) {
        if (ioctl(0, VTBORDER, &n) < 0)
            error("unable to set colour");
        return 0;
    }
    if (strcmp(cmd, "ink") == 0)
        return colour_or_vtcode(n, VTINK, "\033b");
    if (strcmp(cmd, "paper") == 0)
        return colour_or_vtcode(n, VTPAPER, "\033c");
    error("unknown command name");
}
