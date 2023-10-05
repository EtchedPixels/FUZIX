/*
 *	Helpers the RTC expects
 */

void ds3234_select(uint_fast8_t yesno);
void ds3234_write(uint_fast8_t byte);
uint_fast8_t ds3234_read(void);

uint_fast8_t ds3234_reg_read(uint_fast8_t r);
void ds3234_reg_write(uint_fast8_t r, uint_fast8_t val);
