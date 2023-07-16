#ifndef __GPIO_H
#define __GPIO_H

/* Ports are named A B etc in the docs so this is easier to read. Mask so
   lower case doesn't produce weird bugs */
#define GPIO_PORT(n) (((n) & ~0x20) - 'A')

#define GPIO_PINFUN_I    0U
#define GPIO_PINFUN_O    1U
#define GPIO_PINFUN_ODI  2U
#define GPIO_PINFUN_ODO  3U
#define GPIO_PINFUN_ODIO 4U
#define GPIO_PINFUN_PFIO 5U
#define GPIO_PINFUN_AIO  6U

#define GPIO_PAD_STD     0U
#define GPIO_PAD_STDWPU  1U
#define GPIO_PAD_STDWPD  2U
#define GPIO_PAD_OD      3U
#define GPIO_PAD_ODWPU   4U
#define GPIO_PAD_ODWPD   5U
#define GPIO_PAD_ANALOG  6U

void gpio_write(unsigned int port, unsigned int pin, unsigned int onoff);
void gpio_setup_pin(unsigned int port, unsigned int func, unsigned int pin,
                    unsigned int pad, unsigned int strength,
                    unsigned int alt, unsigned int value);

#endif /* __GPIO_H */
