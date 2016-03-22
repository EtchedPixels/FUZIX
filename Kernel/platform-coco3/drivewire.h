/*

    Low Level Drivewire defs

*/



/* A Basic dw_transaction, good for everything (disk,vports,time,etc...)  */
 uint16_t dw_transaction( char *send, uint16_t scnt,
			  char *recv, uint16_t rcnt );
