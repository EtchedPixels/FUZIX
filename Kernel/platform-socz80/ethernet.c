/*
 *	SocZ80 ethernet driver for the Ethernet Wing
 *
 *	The ethernet wing is an ENC28J60 attached to the SocZ80 SPI and GPIO.
 *	At the moment there is no extra VHDL logic to support interrupts.
 *
 *	http://ww1.microchip.com/downloads/en/DeviceDoc/39662e.pdf
 *
 *	(C) 2015 Alan Cox, based on my CP/M 3 ether.asm tool
 */


#include <kernel.h>
#include <kdata.h>
#include <deveth.h>
#include <printf.h>

__sfr __at 0x38 eth_chipselect;
__sfr __at 0x39 eth_status;
__sfr __at 0x3a eth_tx;
__sfr __at 0x3b eth_rx;
__sfr __at 0x3c eth_divisor;
__sfr __at 0x3d eth_gpio;
__sfr __at 0x3e eth_spimode;

static int eth_present;

static uint8_t ewait, enowait;

/* In bank 0 */
#define ERDPTL		0x00
#define ERDPTH		0x01
#define EWRPTL		0x02
#define EWRPTH		0x03
#define ETXSTL		0x04
#define ETXSTH		0x05
#define ETXNDL		0x06
#define ETXHDL		0x07
#define ERXSTL		0x08
#define ERXSTH		0x09
#define ERXNDL		0x0A
#define ERXNDH		0x0B
#define ERXRDPTL	0x0C
#define ERXRDPTH	0x0D
#define ERXWRPTL	0x0E
#define ERXWRPTH	0x0F
#define EDMASTL		0x10
#define EDMASTH		0x11
#define EDMANDL		0x12
#define EDMANDH		0x13
#define EDMADSTL	0x14
#define EDMADSTH	0x15
#define EDMACSL		0x16
#define EDMACSH		0x17
/* In all banks */
#define EIE		0x1B
#define EIR		0x1C
#define ESTAT		0x1D
#define ECON2		0x1E
#define ECON1		0x1F
/* In bank 1 */
#define EHT0		0x00
#define EHT1		0x01
#define EHT2		0x02
#define EHT3		0x03
#define EHT4		0x04
#define EHT5		0x05
#define EHT6		0x06
#define EHT7		0x07
#define EPMM0		0x08
#define EPMM1		0x09
#define EPMM2		0x0A
#define EPMM3		0x0B
#define EPMM4		0x0C
#define EPMM5		0x0D
#define EPMM6		0x0E
#define EPMM7		0x0F
#define EPMCSL		0x10
#define EPMCSH		0x11
#define EPMOL		0x14
#define EPMOH		0x15
#define ERXFCON		0x18
#define EPKTCNT		0x19
/* In bank 2 */
#define MACON1		0x00
#define MACON2		0x01
#define MACON3		0x02
#define MACON4		0x03
#define MABBIPG		0x04
#define MAIPGL		0x06
#define MAIPGH		0x07
#define MACLCON1	0x08
#define MACLCON2	0x09
#define MAMXFLL		0x0A
#define MAMXFLH		0x0B
#define MICMD		0x12
#define MIREGADR	0x14
#define MIWRL		0x16
#define MIWRH		0x17
#define MIRDL		0x18
#define MIRDH		0x19
/* In bank 3 */
#define MAADR5		0x00
#define MAADR6		0x01
#define MAADR3		0x02
#define MAADR4		0x03
#define MAADR1		0x04
#define MAADR2		0x05
#define EBSTSD		0x06
#define EBSTCON		0x07
#define EBSTCSL		0x08
#define EBSTCSH		0x09
#define MISTAT		0x0A
#define EREVID		0x12
#define ECOCON		0x15
#define EFLOCON		0x17
#define EPAUSL		0x18
#define EPAUSH		0x19

/* Bit codes for the top bit control */
#define RCR		0x00
#define RBM		0x3A
#define WCR		0x40
#define WBM		0x7A
#define BFS		0x80
#define BFC		0xA0
#define RST		0xFF

#define REG_SLOW	0x100	/* Driver only flag */

struct encrxhdr {
	uint16_t next;		/* Little endian */
	uint16_t length;
	uint16_t status;	/* Status - high 16 */
#define FRAME_GOOD	0x80	/* Double check 0x80 or 0x01 */
};

static uint8_t ethernap(void) __naked
{
	__asm
		push bc
		ld bc,#0x00F8	; 0x0028 ought to work - check and tune
	dellp:	djnz dellp	;255 *13 + 8(3323 clocks) per cycle
		dec c
		jr nz, dellp
		pop bc
		ret
	__endasm;
}

/* Don't inline these, the call/return is budgeted into the delay
   requirements! */
static void eth_select(void)
{
	eth_chipselect = 0xFE;
}

static void eth_deselect(void)
{
	eth_rx;
	eth_rx;
	eth_chipselect = 0xFF;
}

/* Write a command to the registers */
static void eth_cmd(uint8_t reg, uint8_t val)
{
	eth_select();
	eth_tx = reg;
	eth_tx = val;
	eth_deselect();
}

/* Write a 16bit command to a register pair */
static void eth_cmd16(uint8_t reg, uint16_t val)
{
	eth_cmd(reg++, val & 0xFF);
	eth_cmd(reg, val >> 8);
}

static uint8_t eth_get(uint16_t reg)
{
	uint8_t r;
	eth_select();
	eth_tx = reg & 0xFF;
	eth_tx = 0;
	if (reg & REG_SLOW)
		eth_tx = 0;
	r = eth_rx;
	eth_deselect();
	return r;
}

/* Select banks */
static void eth_bank(uint8_t bank)
{
	if (bank != 3)
		eth_cmd(BFC | ECON1, 0x03);	/* Clear bank bits */
	if (bank)
		eth_cmd(BFS | ECON1, bank);	/* Set needed bits */
}


static void phy_cmd16(uint8_t r, uint16_t cmd)
{
	eth_bank(2);
	eth_cmd(WCR | MIREGADR, r);
	eth_cmd16(WCR | MIWRL, cmd);
	eth_bank(3);
	while (eth_get(REG_SLOW | MISTAT) & 1);
}

static uint16_t phy_read(uint8_t r)
{
	eth_bank(2);
	eth_cmd(WCR | MIREGADR, r);
	eth_cmd(WCR | MICMD, 0x01);
	eth_bank(3);
	while (eth_get(REG_SLOW | MISTAT) & 1);
	eth_bank(2);
	eth_cmd(WCR | MICMD, 0x00);
	return eth_get(REG_SLOW | MIRDL) | (eth_get(REG_SLOW | MIRDH) <<
					    8);
}

/* Reset the controller by waggling the GPIO */
static void ethernet_reset(void)
{
	eth_gpio = 0;
	ethernap();
	eth_gpio = 1;
}


/*
 *	Wait until free and then send a packet. We don't have an interrupt
 *	mechanism for the SPI ethernet on the SocZ80 although we could add
 *	one via a GPIO if it turns out useful
 *
 *	Caution: We could have two threads of execution doing a read and
 *	a write. We won't pre-empt mid write but we may have a read poll
 *	running, so we must shut that off
 */
int eth_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	uint8_t *packet = (uint8_t *)udata.u_offset;
	uint16_t len = udata.u_count;

	used(minor);
	used(rawflag);
	used(flag);

	if (len > 1514) {
		udata.u_error = EFBIG;
		return -1;
	}

	/* Stop any read polling */
	enowait++;

	eth_bank(0);
	while (eth_get(ECON1) & 0x8) {
		/* Not ready */
		if (!(eth_get(EIR) & 2)) {
			/* Errata fix */
			eth_cmd(BFS | ECON1, 0x80);
			eth_cmd(BFC | ECON1, 0x80);
		}
		_sched_yield();
		/* FIXME: check link up */
		/* May have switched bank */
		eth_bank(0);
	}
	/*
	 *    Load the packet up
	 */
	eth_cmd16(WCR | EWRPTL, 0x0C00);
	eth_cmd16(WCR | ETXNDL, 0x0C00 + len);
	eth_cmd(WBM, 0x07);
	eth_chipselect = 0xFE;
	eth_tx = WBM;

	while (len--)
		eth_tx = ugetc(packet++);
	eth_deselect();
	/*
	 *    Start transmit
	 */
	eth_cmd(BFS | ECON1, 0x08);

	/* Permit read polling to resume if active */
	enowait--;

	return len;
}

/*
 *	Check with the phy if the link is up
 */
int eth_up(void)
{
	return phy_read(0x11) & 4;
}

static void eth_readpkt(uint8_t * packet, int len)
{
	uint8_t c;
	eth_select();
	eth_tx = RBM;
	while (len--) {
		eth_tx = 0;
		c = eth_rx;	/* SDCC 3.4.0 will miscompile I/O as arguments */
		uputc(c, packet++);
	}
	eth_deselect();
}

static uint16_t eth_rxnext = 0;

int eth_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
	static struct encrxhdr eth;
	uint8_t *packet = (uint8_t *)udata.u_offset;
	uint16_t len = udata.u_count;
	int r = -EIO;

	used(flag);
	used(rawflag);
	used(minor);

	/* Check carrier ?? */

	/* Wait for packet: bank must always be 1 before we set ewait */
	eth_bank(1);

	/* We have no IRQ so its polled */
	while (eth_get(EPKTCNT) == 0) {
		ewait = 1;
		/* We will task switch to something else or idle and
		   in doing so enable interrupts for other tasks */
		if (psleep_flags(&eth_present, flag))
			return -1;
	}
	/* Ensure ewait is clear before we re-enable interrupts */
	ewait = 0;

	eth_bank(0);
	eth_cmd16(WCR | ERDPTL, eth_rxnext);
	eth_readpkt((uint8_t *)&eth, 6);
	eth_rxnext = eth.next;
	if (eth.status & FRAME_GOOD) {
		len = min(len, eth.length - 4);
		eth_readpkt(packet, len);
		r = len;
	}
	if (eth_rxnext > 0x0BFF)
		eth_rxnext = 0x0BFF;
	eth_cmd16(WCR | ERXRDPTL, eth_rxnext);
	eth_cmd(BFS | ECON2, 0x40);
	return r;
}

void eth_poll(void)
{
	if (!ewait || enowait)
		return;
	/* if we are waiting for ethernet we are in bank 1, see
	   eth_read */
	if (eth_get(EPKTCNT))
		wakeup(&eth_present);
}

int eth_open(uint8_t minor, uint16_t flag)
{
	used(flag);
	if (minor || !eth_present)
		return -ENXIO;
	return 0;
}

int eth_close(uint8_t minor)
{
	used(minor);
	return 0;
}

int eth_ioctl(uint8_t minor, uarg_t arg, char *data)
{
	used(minor);
	used(arg);
	used(data);
	return -1;
}

/* Initialize the Ethernet wing */
void deveth_init(void)
{
	eth_divisor = 6;	/* 9.3MHz (device limit is 10) */
	if (eth_divisor == 0xFF)	/* Hardware absent */
		return;
	eth_spimode = 0;
	eth_chipselect = 0xff;
	eth_gpio = 1;
	eth_cmd(RST | ECON1, 0xff);
	ethernap();

	/* Wait for the NIC to stabilize */
	while (!(eth_get(ESTAT) & 1));

	eth_bank(0);
	eth_cmd16(WCR | ERXSTL, 0);
	eth_cmd16(WCR | ERXRDPTL, 0);
	eth_cmd16(WCR | ERXNDL, 0x0BFF);
	eth_cmd16(WCR | ETXSTL, 0x0C00);
	eth_cmd16(WCR | ETXNDL, 0x11FF);
	eth_bank(1);
	eth_cmd16(WCR | EPMM0, 0x303F);
	eth_cmd16(WCR | EPMCSL, 0xF7F9);
	eth_cmd(BFS | ERXFCON, 0x01);
	eth_bank(2);
	eth_cmd(WCR | MACON2, 0x00);
	if (eth_get(REG_SLOW | MACON2)) {
		kputs("eth: stuck in reset.\n");
		return;
	}
	eth_cmd(WCR | MACON1, 0x0d);
	if (eth_get(REG_SLOW | MACON1) != 0x0d) {
		kputs("eth: macon1 fail.\n");
		return;
	}
	eth_cmd(BFS | MACON3, 0x32);
	eth_cmd16(WCR | MAIPGL, 0x0C12);
	eth_cmd(WCR | MABBIPG, 0x12);
	eth_cmd16(WCR | MAMXFLL, 0x05DC);
	if (eth_get(REG_SLOW | MACON1) != 0x0d) {
		kputs("eth: macon1 fail2.\n");
		return;
	}
	eth_bank(3);
	/* Set the mac to AAAAAAC0FFEE for now FIXME */
	eth_cmd16(WCR | MAADR5, 0xAAAA);
	eth_cmd16(WCR | MAADR3, 0xC0AA);
	eth_cmd16(WCR | MAADR1, 0xEEFF);
	phy_cmd16(0x01, 0x1000);
	eth_bank(0);
	eth_cmd(BFS | EIE, 0xc0);
	eth_cmd(BFS | ECON1, 0x04);
	eth_present = 1;
	kputs("eth: ethernet detected.\n");
}
