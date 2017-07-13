#include "Ldirect.h"

/** @file
	16階調表示関連の高速 RAMに に配置する実装.
	
	@author zurachu
*/

#include <piece.h>
#include <string.h>

extern BYTE* s_vbuff;
extern BYTE* s_16buff;
extern BYTE* s_4buff;
extern BYTE* s_dbuff[ LDIRECT_PAGE_NUM ];

extern BOOL s_vbuff_view;
extern BOOL s_lcd_update;

/// 残り更新ページ数カウント
static int s_dbuff_update_count = 0;

/**
	4階調用描画バッファから仮想画面バッファにマスクあり転送.
*/
void trans_4buff_to_vbuff( void )
{
	static DWORD const s_color_bits = 0x03030303;
	static DWORD const s_mask_bits = ( COLOR_MASK << 24 ) | ( COLOR_MASK << 16 ) | ( COLOR_MASK << 8 ) | COLOR_MASK;
	int i;
	DWORD color, mask;
	DWORD* vbuff_ptr = (DWORD*)s_vbuff;
	DWORD const* b4buff_ptr = (DWORD*)s_4buff;
	for( i = 0; i < DISP_X * DISP_Y / sizeof(DWORD); i += 1 )
	{
		color = ( *b4buff_ptr & s_color_bits );
		color |= color << 2;
		mask = ( ( *b4buff_ptr & s_mask_bits ) >> 2 ) * 0xFF;
		*vbuff_ptr &= mask;
		*vbuff_ptr++ |= color & ~mask;
		++b4buff_ptr;
	}
}

/**
	仮想画面バッファから指定ページのダイレクト転送用バッファにマスクなし転送.
	@param page ダイレクト転送用バッファのページ番号
*/
static void trans_vbuff_to_dbuff( int page )
{
	static WORD const s_color_table[ LDIRECT_PAGE_NUM ][ LDIRECT_COLOR_NUM ] =
	{
		{ 0x000, 0x100, 0x000, 0x100, 0x100, 0x100, 0x001, 0x100, 0x001, 0x001, 0x001, 0x101, 0x001, 0x101, 0x101, 0x101 },
		{ 0x000, 0x000, 0x100, 0x000, 0x100, 0x100, 0x100, 0x001, 0x100, 0x001, 0x001, 0x001, 0x101, 0x001, 0x101, 0x101 },
		{ 0x000, 0x000, 0x000, 0x100, 0x100, 0x100, 0x100, 0x100, 0x001, 0x001, 0x001, 0x001, 0x001, 0x101, 0x101, 0x101 },
		{ 0x000, 0x000, 0x100, 0x000, 0x100, 0x100, 0x100, 0x001, 0x100, 0x001, 0x001, 0x001, 0x101, 0x001, 0x101, 0x101 },
		{ 0x000, 0x000, 0x000, 0x100, 0x000, 0x100, 0x100, 0x100, 0x001, 0x100, 0x001, 0x001, 0x001, 0x101, 0x001, 0x101 },
		//    0,     1,     2,     3,     4,     5,     6,     7,     8,     9,     A,     B,     C,     D,     E,     F
	};
	int xx, yy;
	DWORD* dbuff_ptr = (DWORD*)s_dbuff[ page ];
	BYTE const* vbuff_ptr = s_vbuff;
	BYTE const* vbuff_ptr1 = vbuff_ptr + DISP_X;
	WORD const* const color_table_ptr = s_color_table[ page ];
	DWORD c, c1;

	for( xx = 0; xx < DISP_X / 8; xx += 1 )
	{
		for( yy = 0; yy < DISP_Y / 2; yy += 1 )
		{
			c = color_table_ptr[ *vbuff_ptr++ ];
			c |= color_table_ptr[ *vbuff_ptr++ ] << 1;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 2;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 3;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 4;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 5;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 6;
			c |= color_table_ptr[ *vbuff_ptr++ ] << 7;
			c1 = color_table_ptr[ *vbuff_ptr1++ ];
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 1;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 2;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 3;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 4;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 5;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 6;
			c1 |= color_table_ptr[ *vbuff_ptr1++ ] << 7;
			*dbuff_ptr++ = c | ( c1 << 16 );
			vbuff_ptr += DISP_X * 2 - 8;
			vbuff_ptr1 += DISP_X * 2 - 8;
		}
		vbuff_ptr += -( DISP_X * DISP_Y ) + 8;
		vbuff_ptr1 += -( DISP_X * DISP_Y ) + 8;
	}
}

void Ldirect_Trans( void )
{
	static int page = 0;

	if( s_lcd_update )
	{
		memcpy( s_vbuff, s_16buff, DISP_X * DISP_Y );
		if( s_vbuff_view )
		{
			trans_4buff_to_vbuff();
		}
		s_dbuff_update_count = LDIRECT_PAGE_NUM;
	}
	if( s_dbuff_update_count )
	{
		trans_vbuff_to_dbuff( page );
		s_dbuff_update_count--;
	}
	pceLCDTransDirect( s_dbuff[ page ] );
	page = ( page + 1 ) % LDIRECT_PAGE_NUM;
}

int Ldirect_DrawObject( PIECE_BMP const* p, int dx, int dy, int sx, int sy
						, int width, int height )
{
	int const mask_bit = p->header.mask;
	int const sw = p->header.w;
	int const sh = p->header.h;
	int xx, yy, x_bit;
	BYTE* const buff_base = p->buf;
	BYTE* const mask_base = p->mask;
	BYTE* lbuff_ptr;
	BYTE* buff_ptr;
	BYTE* mask_ptr;
	BYTE b = 0, m = 0;
	
	if( dx >= DISP_X || dy >= DISP_Y ) return 0;
	if( dx < 0 ) { width += dx; sx -= dx; dx = 0; }
	if( dy < 0 ) { height += dy; sy -= dy; dy = 0; }
	if( sx < 0 ) { width += sx; sx = 0; }
	if( sy < 0 ) { height += sy; sy = 0; }
	if( width <= 0 || height <= 0 ) return 0;
	if( width > sw ) { width = sw; }
	if( height > sh ) { height = sh; }
	if( dx + width > DISP_X ) { width = DISP_X - dx; }
	if( dy + height > DISP_Y ) { height = DISP_Y - dy; }

	lbuff_ptr = s_16buff + ( dx + DISP_X * dy );

	for( yy = 0; yy < height; yy += 1 )
	{
		buff_ptr = buff_base + ( ( sx + sw * ( sy + yy ) ) >> 1 );
		mask_ptr = mask_base + ( ( sx + sw * ( sy + yy ) ) >> 3 );
		x_bit = sx & 7;
		if( x_bit & 1 ) { b = *buff_ptr++ << 4; }
		if( x_bit ) { m = ( mask_bit )? ( *mask_ptr++ << x_bit ) : 0; }
		for( xx = 0; xx < width; xx += 1 )
		{
			if( !( x_bit & 1 ) ) { b = *buff_ptr++; }
			if( !x_bit ) { m = ( mask_bit )? *mask_ptr++ : 0; }
			*lbuff_ptr++ = ( m & 0x80 )? *lbuff_ptr : ( b >> 4 );
			x_bit = ( x_bit + 1 ) & 7;
			b <<= 4;
			m <<= 1;
		}
		lbuff_ptr += DISP_X - width;
	}
	return 1;
}

