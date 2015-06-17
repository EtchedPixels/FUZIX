#ifndef __TTYDW_DOT_H__
#define __TTYDW_DOT_H__

void dw_putc( uint8_t minor, unsigned char c );
void dw_vopen( uint8_t minor );
void dw_vclose( uint8_t minor );
void dw_vpoll( );
#endif
