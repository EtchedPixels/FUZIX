/* Defines for the dwtime RTC module */

/* call to initialize dwtime subsystem
   Warning: this funtion is in the discard section, so please call it
   before freeing .discard
*/
int dwtime_init(void);
