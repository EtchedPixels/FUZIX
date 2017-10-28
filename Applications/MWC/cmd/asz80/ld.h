
/* Symbols live in a set of hashed lists indexed by name. Only one
   instance of each name ever exists. */
struct symbol
{
    struct symbol *next;
    struct object *definedby;
    char name[16];
    uint16_t value;
    uint8_t type;
    uint8_t flags;
};

struct object {
    struct object *next;
    struct symbol **syment;
    /* We might want to store a subset of this */
    struct objheader oh;
    uint16_t base[4];	/* Base address we select for this object */
    int nsym;
};

#define NHASH	64

struct symbol *symhash[NHASH];
struct object *objects, *otail;
extern uint16_t base[4], size[4];