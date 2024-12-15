/*
 *	Taken from picogpio as the API is common so there isn't
 *	really a reason to keep this in the pico stuff.
 *
 *	Partly rewritten to be cleaner and extended a bit
 *
 *	Should be extended to find pins by name and also to support
 *	configuring write directions
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/gpio.h>
#include <sys/ioctl.h>

static union {
	struct gpioreq gr;
	struct gpio info;
} gpio;
static struct gpioreq greq;

static uint_fast8_t get_8bit(const char *name, const char *str, const char *err)
{
	char *endptr;
	unsigned long val = strtoul(str, &endptr, 10);

	/* check string was a number and within the range of uint8_t */
	if (*endptr == '\0' && val <= 255)
		return val;
	fprintf(stderr, "%s: <%s> must be between 0 and 255\n", name, err);
	exit(1);
}

static void gpio_query(int fd, unsigned pin)
{
	gpio.gr.pin = pin;
	if (ioctl(fd, GPIOC_GETINFO, &gpio) == -1) {
		perror("GPIO info query failed");
		exit(1);
	}
}

static int display_one_gpio(int i, int fd)
{
	unsigned m;
	unsigned j;
	int r;
		
	m = 0x100;
	
	gpio_query(fd, i);

	printf("%-3d : %-8.8s (%02X) ", i, gpio.info.name, gpio.info.group);
	for (j = 0; j < 8; j++) {
		m >>= 1;
		if (gpio.info.pinmask & m) {
			if (gpio.info.wmask & m)
				putchar('W');
			else
				putchar('R');
		}
		else
			putchar('-');
	}
	if (gpio.info.wmask)
		printf(" W:%02X ", gpio.info.wdata);
	else
		printf("     ");

	if (gpio.info.wmask != 0xFF) {
		greq.pin = i;
		r = ioctl(fd, GPIOC_GETBYTE, &greq);
		if (r == -1) {
			perror("gpioc_getbyte");
			exit(1);
		}
		printf("  R:%02X ", r);
	} else
		printf("       ");
	switch(gpio.info.flags) {
	case 0:
		printf("fixed direction");
		break;
	case GPIO_BIT_CONFIG:
		printf("per bit direction");
		break;
	case GPIO_GRP_CONFIG:
		printf("per group direction");
		break;
	default:
		printf("(unknown %u)", gpio.info.flags);
	}
	putchar('\n');
}

static int display_all_gpio(int fd)
{
	int num_pins = ioctl(fd, GPIOC_COUNT, 0);
	int i;

	if (num_pins == -1) {
		perror("read GPIO count");
		return 1;
	}
	for (i = 0; i < num_pins; i += 8)
		display_one_gpio(i, fd);
	return 0;
}

static int set_output(const char *name, int fd, unsigned pin, unsigned long iop)
{
	uint_fast8_t pmask = 1 << (pin & 7);

	greq.pin = pin;
	greq.val = pmask;

	gpio_query(fd, pin);
	if ((gpio.info.wmask & pmask) == 0) {
		if (gpio.info.flags == 0) {
			printf("%s: GPIO %-.8s is read only.\n", name, gpio.info.name);
			return 1;
		}
		greq.val = gpio.info.wmask | pmask;
		if (ioctl(fd, GPIOC_SETRW, &greq) == -1) {
			perror("GPIO set direction");
			return 1;
		}
	}
	if (ioctl(fd, iop, &greq) != 0) {
		perror("GPIO change failed");
		return 1;
	}
	return 0;
}

static int set_input(const char *name, int fd, unsigned pin)
{
	uint_fast8_t pmask = 1 << (pin & 7);

	gpio_query(fd, pin);

	if (gpio.info.wmask & pmask) {
		if (gpio.info.flags == 0) {
			printf("%s: GPIO %-.8s is write only.\n", name, gpio.info.name);
			return 1;
		}
		greq.val = gpio.info.wmask & ~pmask;
		if (ioctl(fd, GPIOC_SETRW, &greq) == -1) {
			perror("GPIO set direction");
			return 1;
		}
	}
	return 0;
}

static unsigned usage(const char *name)
{
	fprintf(stderr, "%s {pin {0 | 1 | in}}\n", name);
	return 1;
}

int main(int argc, char **argv)
{
	int fd;
	uint_fast8_t pin;
	uint_fast8_t value;

	fd = open("/dev/gpio", O_RDWR, 0);
	if (!fd) {
		perror("Failed to open /dev/gpio");
		return 1;
	}

	if (argc == 1)
		return display_all_gpio(fd);

	pin = get_8bit(argv[0], argv[1], "pin");
	if (argc == 2)
		return display_one_gpio(pin, fd);
	if (argc == 3) {
		if (strcmp(argv[2], "1") == 0)
			return set_output(argv[0], fd, pin, GPIOC_SET);
		else if (strcmp(argv[2], "0") == 0)
			return set_output(argv[0], fd, pin, GPIOC_CLR);
		else if (strcmp(argv[2], "in") == 0)
			return set_input(argv[0], fd, pin);
	}
	return usage(argv[0]);
}
