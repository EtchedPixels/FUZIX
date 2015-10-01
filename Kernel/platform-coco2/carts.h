#define CART_DRAGONDOS	1		/* DragonDOS floppy */
#define CART_DELTADOS	2		/* DeltaDOS floppy */
#define CART_RSDOS	3		/* RSDOS Cartridge */
#define CART_ORCH90	4		/* Orchestra Sound */

extern int cart_find(int id);
extern uint8_t carttype[4];
extern uint8_t cartslots;
extern uint8_t bootslot;
extern uint8_t system_id;
