extern uint8_t trs80_model;
#define TRS80_MODEL1	0
#define TRS80_MODEL3	1
#define LNW80		2	/* Need to handle LNW80 model II.. */
#define VIDEOGENIE	3	/* But colour genie is way too different.. */

extern uint8_t trs80_mapper;
#define MAP_SUPERMEM	0	/* Alpha Supermem and compatibles */
#define MAP_SELECTOR	1	/* Selector */

extern uint8_t trs80_udg;	/* User defined graphics module */
#define UDG_NONE	0
#define UDG_PCG80	1	/* Orcim PCG80 */
#define UDG_80GFX	2	/* Programma 80-Grafix */
#define UDG_MICROFIRMA	3	/* External box option for UK TRS80 Model 1 */

extern uint8_t video_lower;	/* Lowercase available */

/*
 *	Differences versus model 1
 *
 *	LNW80		Port 0xFE control for memory, extra graphics modes,
 *			(not currently supported). 4MHz CPU mode. Memory
 *			expansions include the selector.
 *	LNW80 II	Adds 4 banks of graphics memory it seems. Also adds
 *			port 0x1f which controls low system RAM
 *			(bit 1 - ROM off if set, bit 2 blocks IRQ if set)
 *	VideoGenie	Printer at 0xFD not memory mapped. The earlier
 *			expansion unit is a 3 slot S100 backplane so can be
 *			fitted with banked S100 memory (only the upper 32K
 *			seems to map to the S100). Some had lower case.
 *	Model 3		Floppy moves to 0xF0-F4/F7 except for status, Sound
 *			click moves to 0x90 printer to 0xFB. 0xEF control
 *			register, NMI control, Interrupt mask register.
 *			Interrupt latch doesn't ack timer and is inverted.
 *			Lower case !!
 *
 *	Note that the Colour Genie is a very different beast.
 *	
 */

extern void bufsetup(void);

/* Helpers for I/O as I/O is not usually mapped in kernel space */
extern void keyscan(void);
extern uint8_t anykey(void);
extern void vt_check_lower(void);
extern uint8_t ioread(uint16_t addr) __z88dk_fastcall;
