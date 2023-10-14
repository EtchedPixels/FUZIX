extern uint8_t kdataseg;

extern void rabbit_spi_tx(uint8_t c) __z88dk_fastcall;
extern uint8_t rabbit_spi_rx(void);
extern void rabbit_spi_slow(void);
extern void rabbit_spi_fast(void);
extern void rabbit_read_rtc(uint8_t *ptr) __z88dk_fastcall;
extern void rabbit_bop_watchdog(uint8_t code) __z88dk_fastcall;

#define GCSR		0x00
#define RTCCR		0x01
#define RTC0R		0x02
#define RTC1R		0x03
#define RTC2R		0x04
#define RTC3R		0x05
#define RTC4R		0x06
#define RTC5R		0x07
#define WDTCR		0x08
#define WDTTR		0x09
#define GCM0R		0x0A
#define GCM1R		0x0B
#define GOCR		0x0E
#define GCDR		0x0F
#define MMIDR		0x10
#define STACKSEG	0x11
#define DATASEG		0x12
#define SEGSIZE		0x13
#define MB0CR		0x14
#define MB1CR		0x15
#define MB2CR		0x16
#define MB3CR		0x17
#define SPD0R		0x20
#define SPD1R		0x21
#define SPD2R		0x22
#define SPSR		0x23
#define SPCR		0x24
#define GCPU		0x2E
#define GREV		0x2F
#define PADR		0x30
#define PBDR		0x40
#define PCDR		0x50
#define PCFR		0x55
#define PDDR		0x60
#define PDCR		0x64
#define PDFR		0x65
#define PDDCR		0x66
#define PDDDR		0x67
#define PDB0R		0x68
#define PDB1R		0x69
#define PDB2R		0x6A
#define PDB3R		0x6B
#define PDB4R		0x6C
#define PDB5R		0x6D
#define PDB6R		0x6E
#define PDB7R		0x6F
#define PEDR		0x70
#define PECR		0x74
#define PEFR		0x75
#define PEDDR		0x77
#define PEB0R		0x78
#define PEB1R		0x79
#define PEB2R		0x7A
#define PEB3R		0x7B
#define PEB4R		0x7C
#define PEB5R		0x7D
#define PEB6R		0x7E
#define PEB7R		0x7F
#define IB0CR		0x80
#define IB1CR		0x81
#define IB2CR		0x82
#define IB3CR		0x83
#define IB4CR		0x84
#define IB5CR		0x85
#define IB6CR		0x86
#define IB7CR		0x87
#define I0CR		0x98
#define I1CR		0x99
#define TACSR		0xA0
#define TACR		0xA2
#define TAT1R		0xA3
#define TAT4R		0xA9
#define TAT5R		0xAB
#define TAT6R		0xAD
#define TAT7R		0xAF
#define TBCSR		0xB0
#define TBCR		0xB1
#define TBM1R		0xB2
#define TBL1R		0xB3
#define TBM2R		0xB4
#define TBL2R		0xB5
#define TBCMR		0xBE
#define TBCLR		0xBF
#define SADR		0xC0
#define SAAR		0xC1
#define SALR		0xC2
#define SASR		0xC3
#define SACR		0xC4
#define SBDR		0xD0
#define SBAR		0xD1
#define SBLR		0xD2
#define SBSR		0xD3
#define SBCR		0xD4
#define SCDR		0xE0
#define SCAR		0xE1
#define SCLR		0xE2
#define SCSR		0xE3
#define SCCR		0xE4
#define SDDR		0xF0
#define SDAR		0xF1
#define SDLR		0xF2
#define SDSR		0xF3
#define SDCR		0xF4

#define GREV_R2000			0x00
#define GREV_R2000A			0x01
#define GREV_R2000B			0x02
#define GREV_R2000C			0x03

#define GCPU_R2000			0x01

/* This lives in the flash in a protected (mostly) space and is written
   into the board at manufacture */

struct SysIDBlock {
    uint16_t tableVersion;		/* ?? */
    uint16_t productID;			/* Part Number */
    uint16_t vendorID;			/* 1 */
    uint8_t timestamp[7];
    uint32_t flashID;			/* Vendor/Product ID */
    uint16_t flashType;			/* Write method */
    uint16_t flashSize;			/* 4K pages */
    uint16_t sectorSize;		/* bytes */
    uint16_t numSectors;
    uint16_t flashSpeed;		/* ns */
    uint32_t flash2ID;
    uint16_t flash2Type;
    uint16_t flash2Size;
    uint16_t sector2Size;
    uint16_t num2Sectors;
    uint16_t flash2Speed;
    uint32_t ramID;
    uint16_t ramSize;			/* 4K pages */
    uint16_t ramSpeed;			/* ns */
    uint16_t cpuID;
    uint32_t crystalFreq;		/* HZ */
    uint8_t macAddr[6];			/* If allocated */
    uint8_t serialNumber[24];
    char productName[30];		/* \0 terminated */
    char reserved;
    uint32_t idBlockSize;		/* Size of SysIDBlock */
    uint16_t userBlockSize;
    uint16_t userBlockLoc;		/* Offset from this one */
    int idBlockCRC;			/* CRC with this field 0 */
    char marker[6];			/* 55 AA 55 AA 55 AA */
};
