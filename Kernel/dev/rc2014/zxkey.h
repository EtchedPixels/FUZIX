#ifndef __RC2014_ZXKEY_DOT_H__
#define __RC2014_ZXKEY_DOT_H__

void zxkey_poll(void);
void zkey_queue_key(uint8_t, uint8_t);
void zxkey_init(void);
uint16_t zxkey_scan(void);

#define KEY_ROWS	5
#define KEY_COLS	8

extern uint8_t keymap[5];
extern uint8_t keyboard[5][8];
extern uint8_t shiftkeyboard[5][8];

extern uint8_t inputtty, outputtty;

#endif
