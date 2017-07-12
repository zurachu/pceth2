#include "Terminal.h"

/** @file
	Šg’£’[ŽqŠÖ˜A‚ÌŽÀ‘•.
	@author zurachu
*/

#define P0CFP (*((unsigned char*)0x402D0))
#define P0    (*((unsigned char*)0x402D1))
#define P0IOC (*((unsigned char*)0x402D2))
#define P1CFP (*((unsigned char*)0x402D4))
#define P1    (*((unsigned char*)0x402D5))
#define P1IOC (*((unsigned char*)0x402D6))
#define CFEX  (*((unsigned char*)0x402DF))

void Terminal_Init( unsigned char p1
				  , unsigned char p2
				  , unsigned char p3
				  , unsigned char p4
				  , unsigned char p5 )
{
	CFEX  = ( CFEX  & 0xFE ) | 0x00;
	P0CFP = ( P0CFP & 0x8F ) | 0x00;
	P1CFP = ( P1CFP & 0xEB ) | 0x00;
	P0IOC = ( P0IOC & 0x8F ) | ( ( ( p3 & 1 ) << 6 ) | ( ( p4 & 1 ) << 5 ) | ( ( p5 & 1 ) << 4 ) );
	P1IOC = ( P1IOC & 0xEB ) | ( ( ( p1 & 1 ) << 4 ) | ( ( p2 & 1 ) << 2 ) );
}

void Terminal_Set( int port, unsigned char data )
{
	switch( port )
	{
		case 1: P1 = ( P1 & 0xEF ) | ( ( data & 1 ) << 4 ); break;
		case 2: P1 = ( P1 & 0xFB ) | ( ( data & 1 ) << 2 ); break;
		case 3: P0 = ( P0 & 0xBF ) | ( ( data & 1 ) << 6 ); break;
		case 4: P0 = ( P0 & 0xDF ) | ( ( data & 1 ) << 5 ); break;
		case 5: P0 = ( P0 & 0xEF ) | ( ( data & 1 ) << 4 ); break;
	}
}

int Terminal_Get( int port )
{
	switch( port )
	{
		case 1: return ( P1 >> 4 ) & 1;
		case 2: return ( P1 >> 2 ) & 1;
		case 3: return ( P0 >> 6 ) & 1;
		case 4: return ( P0 >> 5 ) & 1;
		case 5: return ( P0 >> 4 ) & 1;
	}
	return 0;
}

