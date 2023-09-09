/*
 *	Labels look like symbols except that they don't really behave
 *	like them in terms of scope. Handle them on their own
 */

#include <stdint.h>
#include "compiler.h"

#define L_DECLARED	0x8000

struct label {
    uint16_t name;
    uint16_t line;
};

struct label labels[MAXLABEL];
struct label *labelp;

void init_labels(void)
{
    labelp = labels;
}

static void new_label(unsigned n)
{
    if (labelp == &labels[MAXLABEL])
        error("too many goto labels");
    labelp->name = n;
    labelp->line = line_num;
    labelp++;
}

static struct label *find_label(unsigned n)
{
    struct label *p = labels;
    n &= 0x7FFF;
    while(p < labelp) {
        if (n == (p->name & 0x7FFF))
            return p;
        p++;
    }
    return NULL;
}

void use_label(unsigned n)
{
    if (find_label(n))
        return;
    new_label(n & 0x7FFF);
}

void add_label(unsigned n)
{
    struct label *l = find_label(n);
    if (l && (l->name & L_DECLARED))
        error("duplicate label");
    if (l == NULL)
        new_label(n | L_DECLARED);
    else
        l->name |= L_DECLARED;
}

void check_labels(void)
{
    struct label *p = labels;
    while(p < labelp) {
        if (!(p->name & L_DECLARED))
            errorline(p->line, "unknown label");
        p++;
    }
}
