#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>
#include <blkdev.h>

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
	{ 0xE1BA, CART_ORCH90, "Orchestra-90 CC" },
	{ 0x0000, 0, "No ROM" }
};

static struct cart_rom_id *cart_lookup(uint16_t hash)
{
	struct cart_rom_id *cart = carts;
	do {
		if (cart->hash == hash)
			return cart;
	} while(cart++->hash);
	return NULL;
}

void map_init(void)
{
	uint8_t i;
	uint8_t bslot = 0;
	uint16_t hash;
	struct cart_rom_id *rom;

	kprintf("%s system.\n", sysname[system_id]);
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
			kprintf("%d: %s %c\n",
				i, rom->name,
				i == bootslot ? '*':' ');
			carttype[i] = rom->id;
		}
		else
			kprintf("%d: Unknown(%x) %c\n",
				i, hash,
				i == bslot ? '*':' ');
	}
	mpi_set_slot(bootslot);
	/* We put swap on the start of slice 0, but with the first 64K free
	   so we can keep the OS image linearly there */
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i + 128);
}
