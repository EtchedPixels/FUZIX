#ifndef _APPLE_H
#define _APPLE_H

extern uint8_t kernel_flag;
extern uint8_t cols;
extern uint8_t card_present;
extern uint8_t prodos_slot;
extern uint8_t pascal_slot;
extern uint8_t model;

#define APPLE_UNKNOWN	0
#define APPLE_IIE	1
#define APPLE_IIC	2

extern uint8_t pascal_op(uint16_t);
extern uint8_t dos_op(uint8_t);
extern uint8_t pascal_cmd[9];

#endif
