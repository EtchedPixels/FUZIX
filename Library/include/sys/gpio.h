#ifndef _GPIO_H
#define _GPIO_H

struct gpioreq {
    uint8_t pin;
    uint8_t val;
};

struct gpio {
    uint8_t pinmask;
    uint8_t group;
    uint8_t flags;
#define GPIO_BIT_CONFIG	1	/* Per bit direction setting */
#define GPIO_GRP_CONFIG	2	/* Per group direction setting */
    uint8_t wmask;
    uint8_t wdata;
    uint8_t imask;
    uint8_t name[8];		/* So you can work out the relationship */
};

#define GPIOC_SETBYTE	0x0530
#define GPIOC_SET	0x0531
#define GPIOC_CLR	0x0532
#define GPIOC_GETBYTE	0x0533
#define GPIOC_SETRW	0x0534
#define GPIOC_GETINFO	0x0535
#define GPIOC_COUNT	0x0536

#endif
