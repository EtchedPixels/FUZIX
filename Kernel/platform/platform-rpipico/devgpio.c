#include <kernel.h>
#include <kdata.h>
#include <gpio.h>
#include "picosdk.h"

// Example on how to access raspberry pi pico GPIO.
// Code inspired by the answer in this issue:
// https://github.com/EtchedPixels/FUZIX/issues/946#issuecomment-1345529209
int gpio_ioctl(uarg_t request, char *data) {
  const uint8_t num_pins = 28;
  static struct gpioreq gr;
  
  if (request == GPIOC_COUNT) {
    return num_pins;
  }

  if (uget(data, &gr, sizeof(struct gpioreq)) == -1) {
    return -1;
  }

  if (gr.pin >= num_pins) {
    udata.u_error = ENODEV;
    return -1;
  }

  switch (request) {
    case GPIOC_SETBYTE:
      break;
    case GPIOC_SET:
      kprintf("pin: %d\n", gr.pin);
      kprintf("value: %d\n", gr.val);
      gpio_init(gr.pin);
      gpio_set_dir(gr.pin, GPIO_OUT);
      gpio_put(gr.pin, gr.val != 0);
      break;
    default:
      udata.u_error = ENODEV;
      return -1;
  }

  return 0;
}