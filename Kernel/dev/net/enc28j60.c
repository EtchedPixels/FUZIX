/*
 *	Core support for the ENC28J60 Ethernet over SPI
 */

#include <kernel.h>
#include <enc28j60.h>
#include <printf.h>

/* Bank 0 */
#define ERDPT		0x00
#define EWRPT		0x02
#define ETXST		0x04
#define ETXND		0x06
#define ERXST		0x08
#define ERXND		0x0A
#define ERXRDPT		0x0C
#define ERDWRPT		0x0E
#define EDMAST		0x10
#define EDMAND		0x12
#define EDMADST		0x14
#define EDMACS		0x16

/* Bank 1 */
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
#define EPMCS		0x10
#define	EPMO		0x14
#define ERXFCON		0x18
#define EPKTCNT		0x19

/* Bank 2 */
#define MACON1		0x00
#define MACON2		0x01
#define MACON3		0x02
#define MACON4		0x03
#define MABBIPG		0x04
#define MAIPG		0x06
#define MACLCON1	0x08
#define MACLCON2	0x09
#define MAMXFL		0x0A
#define MICMD		0x12
#define MIREGADR	0x14
#define MIWR		0x16
#define MIRD		0x18

/* Bank 3 */
#define MAADR5		0x00
#define MAADR6		0x01
#define MAADR3		0x02
#define MAADR4		0x03
#define MAADR1		0x04
#define MAADR2		0x05
#define EBSTSD		0x06
#define EBSTCON		0x07
#define EBSTCSL		0x08
#define MISTAT		0x0A
#define EREVID		0x12
#define ECOCON		0x15
#define EFLOCON		0x17
#define EPAUSE		0x18

/* All banks */
#define EIE		0x1B
#define EIR		0x1C
#define ESTAT		0x1D
#define ECON2		0x1E
#define ECON1		0x1F


#define RCR		0x00
#define RBM		0x20
#define WCR		0x40
#define WBM		0x7A
#define BFS		0x80
#define BFC		0xA0
#define RESET		0xFF

/* Private bits for the driver not hardware */
#define	SLOW		0x8000
#define R16		0x4000

static uint16_t enc_rxnext;
static uint8_t enc_rxpending;
static uint8_t enc_rxheader[8];

/* And this chunk is core code */

static void enc_command(uint8_t reg, uint8_t cmd)
{
    enc_select();
    spi_transmit_byte(reg);
    spi_transmit_byte(cmd);
    enc_deselect();
}

static void enc_bank(uint8_t bank)
{
    if (bank != 3)
        enc_command(BFC|ECON1, 0x03);
    if (bank != 0)
        enc_command(BFS|ECON1, bank);
}


static void enc_command16(uint8_t reg, uint16_t cmd)
{
    enc_command(reg, cmd);
    enc_command(reg + 1, cmd >> 8);
}

static uint8_t enc_read(uint16_t reg)
{
    uint8_t r;
    /* Some registers are fast some are slow, we need to handle this */
    enc_select();
    spi_transmit_byte(reg);
    if (reg & SLOW)
        spi_transmit_byte(0);		/* Padding byte */
    /* Check if TX value matters */
    r = spi_receive_byte();
    enc_deselect();
    return r;
}

static void enc_phycmd16(uint8_t phyreg, uint16_t phyval)
{
    enc_bank(0x02);
    enc_command(WCR|MIREGADR, phyreg);
    enc_command16(WCR|MIWR, phyval);
    /* Now wait for MISTAT */
    enc_bank(0x03);
    while(enc_read(SLOW|MISTAT) & 1);
}

static uint16_t enc_phyread(uint8_t phyreg)
{
    uint16_t ret;
    enc_bank(0x02);
    enc_command(WCR|MIREGADR, phyreg);
    enc_command(WCR|MICMD, 0x01);
    enc_bank(0x03);
    while(enc_read(SLOW|MISTAT) & 1);
    enc_bank(0x02);
    enc_command(WCR|MICMD, 0x00);
    ret = enc_read(SLOW|MIRD);
    ret |= enc_read(SLOW|(MIRD + 1)) << 8;
    /* Back to bank 0 */
    enc_bank(0x00);
    return ret;
}

static const uint16_t cmdtab[] = {
    R16|WCR|ERXST,	0x0000,	/* ERXST */
    R16|WCR|ERXRDPT,	0x0000,	/* ERXRDPT */
    R16|WCR|ERXND,	0x0BFF,	/* ERXND */
    R16|WCR|ETXST,	0x0C00,	/* ETXST */
    R16|WCR|ETXND,	0x11FF,	/* EXTND */
    BFS|ECON1,		0x01,	/* Bank 1 */
    R16|WCR|EPMM0,	0x303F,	/* EPMM0/1 */
    R16|WCR|EPMM2,	0xF7F9,	/* EPMCS */
    BFC|ERXFCON,	0x01,	/* ERXFCON (_BCEN) */
    BFC|ECON1,		0x01,
    BFS|ECON1,		0x02,	/* Bank 2 */
    WCR|MACON2,		0x00,	/* MACON2 */
    WCR|MACON1, 	0x0D,	/* MACON1 */
    BFS|MACON3,		0x32,	/* MACON3 */
    R16|WCR|MAIPG,	0x0C12,	/* MAIPG */
    WCR|MABBIPG,	0x12,	/* MABBIPG */
    R16|WCR|MAMXFL,	0x05DC,	/* MAMXFL */
    BFS|ECON1,		0x03,	/* Bank 3 */
    R16|WCR|MAADR1,	0xAAAA,/* Mac address */
    R16|WCR|MAADR3,	0xC0AA,
    R16|WCR|MAADR5,	0xEEFF,
    BFS|ECON1,		0x03,	/* Bank 0 */
    BFS|EIE,		0xC0,	/* EIE */
    BFS|ECON1,		0x04,	/* ECON1 rx enable */
    0,0
};

int enc_init(void)
{
    const uint16_t *p = cmdtab;
    enc_reset();
    enc_command(0xFF, 0xFF);
    enc_nap_1ms();
    /* Wait until the clock is reported as stable */
    while(!(enc_read(0x1D) & 1));
    enc_command(0x9F, 0x00);
    while(*p) {
        uint16_t r = *p++;
        if (r & R16)
            enc_command16(r, *p++);
        else
            enc_command(r, *p++);
        p++;
    }
    enc_phycmd16(0x10, 0x0100);
    kprintf("enc28j60: revision %02x\n", enc_read(0x12));
    return 0;
}    

int enc_write_packet(uint8_t *p, uint16_t len)
{
    uint8_t r;

    enc_bank(0x00);
    r = enc_read(ECON1);
    if (!(r & 0x08)) {	/* TXRTS idle ? */
        enc_command16(WCR|EWRPT, 0x0C00);	/* Set TX pointer back */
        enc_command16(WCR|ETXND, 0x0C00 + len);	/* Tx end mark */
        enc_command(WBM, 0x07);	/* Begin transfer */
        enc_select();
        /* FIXME: make this caller provided for fast SPI */
        while(len--)
            spi_transmit_byte(*p++);
        enc_deselect();
        enc_command(BFS|ECON1, 0x08);	/* Fire... */
        return len;
    }
    /* Do gunked up clear check */
    r = enc_read(EIR);
    if (!(r & 1)) {
        /* Do a TX reset */
        enc_command(BFS|ECON1, 0x80);
        enc_command(BFC|ECON1, 0x80);
    }
    return 0;
}

int enc_link_up(void)
{
    uint8_t r = enc_phyread(0x11);
    return r & 4;	/* 0 = down */
}

/* Performs the clean up after a packet is received. Will be called in
   bank 0 */
static void enc_read_next(void)
{
    if (enc_rxnext > 0x0BFF)
        enc_rxnext = 0x0BFF;
    enc_command16(WCR|ERXND, enc_rxnext);
    enc_command(BFS|ECON2, 0x40);
    enc_rxpending = 0;
}

int enc_read_begin(void)
{
    int16_t len;

    /* Don't re-enter while a packet receive is pending */
    if (enc_rxpending)
        return 0;

    enc_bank(0x01);
    if (!enc_read(EPKTCNT))
        return 0;
    /* We have something */
    enc_bank(0x00);
    /* Set EDPRT */
    enc_command16(WCR|ERDPT, enc_rxnext);
    enc_read_block(enc_rxheader, 6);
    enc_rxnext = enc_rxheader[0] | (enc_rxheader[1] << 8);
    /* Next two bytes are length (+4) */
    len = enc_rxheader[2] | (enc_rxheader[3] << 8);
    /* And then the status byte */
    if (!(enc_rxheader[4] & 0x80)) {
        /* Bad frame */
        enc_read_next();
        return -1;
    }
    /* Ok good frame */
    enc_rxpending = 1;
    return 1;
}

/* Called after read begin before other rx activity and when we know where
   it is going in user space */
void enc_read_complete(uint8_t *buf, uint16_t len)
{
    enc_read_block_user(buf, len);
    enc_read_next();
}
