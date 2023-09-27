#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <blkdev.h>
#include <carts.h>

/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

extern uint8_t internal32k;
extern uint16_t framedet;
extern uint8_t sys_hz;

void pagemap_init(void)
{
	int i;

	/* kernel uses 4 5 6 7 0 1 2 3F */
	for (i = 0x3e; i > 7; i--)
		pagemap_add(i);
	pagemap_add(3);	/* was used while loading, replaced by 3F */
}

uint8_t plt_param(char *p)
{
	if (strcmp(p, "over") == 0 || strcmp(p, "overclock") == 0) {
		*((volatile uint8_t *)0xFFD7) = 0;
		return 1;
	}
	return 0;
}

static const char *sysname[] = {"Dragon", "COCO", "COCO3", "Unknown"};

struct cart_rom_id {
	uint16_t hash;
	uint8_t id;
	const char *name;
};

static const char empty[] = "(empty)";

struct cart_rom_id carts[] = {
	{ 0x72B0, CART_DRAGONDOS, "DragonDOS" },
	{ 0x9063, CART_DELTADOS, "DeltaDOS" },
	{ 0xB400, 0, empty },
	{ 0xC248, CART_RSDOS, "RS-DOS" },
	/* This one is an oddity - we need to do more to know what it drives
	   The great thing is we can extract the MMIO addresses from it */
	{ 0xB61B, CART_HDBDOS, "HDBDOS" },
	{ 0xCF55, CART_HDBDOS, "HDBDOS" },
	{ 0xE1BA, CART_ORCH90, "Orchestra-90 CC" },
	{ 0x933E, CART_SDBOOT, "SDBOOT" },
	{ 0x0000, 0, "No ROM" }
};

struct hdb_rom_id {
	char key[2];
	uint8_t id;
	const char *name;
};
        
static struct hdb_rom_id hdb[] = {
        { "ID", CART_IDE, "IDE-CHS" },
        { "LB", CART_IDE, "IDE-LBA" },
        { "TC", CART_TC3, "TC^3" },
        { "KE", CART_KENTON, "KENTON" },
        { "LR", CART_LRTECH, "LRTECH" },
        { "HD", CART_HDII, "HD-II" },
        { "4-", CART_4N1, "4-N-1" },
        { "DW", CART_DRIVEWIRE, "Drivewire" },
        { "BE", CART_BECKER, "Becker" },
        { "J&", CART_JMCP, "J&M CP" },
        {}
};

/* Find a cartridge or it's slot */
int cart_find(int id)
{
	int i;
	for (i = 0; i < id; i++) {
		if (carttype[i] == id)
			return i;
	}
	return -1;
}

static struct cart_rom_id *cart_lookup(uint16_t hash)
{
	struct cart_rom_id *cart = carts;
	do {
		if (cart->hash == hash)
			return cart;
	} while(cart++->hash);
	return NULL;
}

static inline int keycmp(uint16_t *k1, uint16_t *k2)
{
	return (*k1 == *k2);
}

static struct hdb_rom_id *hdb_lookup(uint16_t key)
{
	struct hdb_rom_id *id = hdb;
	do {
		if (keycmp(&key, (uint16_t *)id->key))
			return id;
	} while(id++->id);
	return NULL;
}

void map_init(void)
{
	uint8_t i;
	uint8_t bslot = 0;
	uint16_t hash;
	struct cart_rom_id *rom;

	if (framedet >= 0x0500)
		sys_hz = 5;
	else
		sys_hz = 6;
	kprintf("%d0Hz %s system.\n", sys_hz, sysname[system_id]);

	if (mpi_present()) {
		kputs("MPI cartridge detected.\n");
		cartslots = 4;
		bootslot = mpi_set_slot(0);
		bslot = bootslot & 3;
	}
	for (i = 0; i < cartslots; i++) {
		mpi_set_slot((i << 4) | i);
		hash = cart_hash();
		rom = cart_lookup(hash);
		if (rom) {
			kprintf("%d:%c%s",
			        i, i == bslot ? '*':' ',
				rom->name);
			carttype[i] = rom->id;
		}
		else
			kprintf("%d:%cUnknown(%x)",
				i, i == bslot ? '*':' ', hash);
				
		/* The analysis needs to be in asm as we need to page in
		   the ROM to peer at it */
		if (rom->id == CART_HDBDOS) {
                        uint16_t t = cart_analyze_hdb();
                        struct hdb_rom_id *hdb = hdb_lookup(t);
                        if (hdb) {
				kprintf(" %s ID %d",
					hdb->name, hdb_id);
                                if (hdb->id == CART_IDE)
                                        kprintf(" MMIO %x", hdb_port);
                                carttype[i] = hdb->id;
                                cartaddr[i] = hdb_port;
                        } else
                                kprintf("??%x\n", t);
                }
                kputchar('\n');
	}
	mpi_set_slot(bootslot);
	tty_detect();
}
