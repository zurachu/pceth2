#include "FontFuchi.h"

/** @file
	フォント関連の実装.
	@author zurachu
*/

#include <stdarg.h>
#include <piece.h>
#include <smcvals.h>

#include "FontProxy.h"

static int s_x = 1;
static int s_y = 1;
static int s_left = 1;
static int s_top = 1;
static int s_right = DISP_X - 1;
static int s_bottom = DISP_Y - 1;

#define FONT_TYPE_NUM (3) ///< フォント種類総数
static int s_type = 0; ///< フォント種類
static int const s_font_width[ FONT_TYPE_NUM ] = { 5, 8, 4 }; ///< フォント幅
static int const s_font_height[ FONT_TYPE_NUM ] = { 10, 16, 6 }; ///< フォント高さ

static int s_tx_color = 3; ///< 文字色
static int s_bd_color = 0; ///< 縁取り色

void FontFuchi_SetPos( int x, int y )
{
	s_x = x;
	s_y = y;
}

void FontFuchi_SetRange( int left, int top, int right, int bottom )
{
	s_left = left;
	s_top = top;
	s_right = right;
	s_bottom = bottom;
}

void FontFuchi_SetType( int type )
{
	s_type = type;
}

void FontFuchi_SetTxColor( int color )
{
	s_tx_color = color;
}

void FontFuchi_SetBdColor( int color )
{
	s_bd_color = color;
}

/**
	縁取りを仮想画面に表示.
	@param x x座標（実際は x-1 から表示される）
	@param y y座標（実際は y-1 から表示される）
	@param code 文字コード
	@return
		- b0-7 x方向の幅（フォント自体の幅）
		- b8-15 y方向の幅（フォント自体の幅）
*/
static unsigned short put_fuchi( int x, int y, unsigned short code )
{
	int xx, yy, dy;
	int const x_start = x - 1;
	int const y_start = y - 1;
	unsigned char const* p = pceFontGetAdrs( code );
	unsigned char* vbuff = pceLCDSetBuffer( INVALIDPTR ) + ( x_start + DISP_X * y_start );
	int width = s_font_width[ s_type ];
	int const height = s_font_height[ s_type ];
	int fuchi_width = width + 2;
	int const fuchi_height = height + 2;
	int x_end = x_start + fuchi_width;
	int y_end = y_start + fuchi_height;
	unsigned char const color = s_bd_color;
	unsigned int image[ 16 + 2 ];
	if( DISP_X < x_end ) { x_end = DISP_X; }
	if( DISP_Y < y_end ) { y_end = DISP_Y; }
	image[ 0 ] = image[ 1 ] = 0;
	
	if( (int)p & 0x80000000 ) // 半角
	{
		for( yy = 0; yy < height; yy++ )
		{
			unsigned int const line = *p++ << 4;
			image[ yy+2 ]  = line | ( line >> 1 ) | ( line >> 2 );
			image[ yy+1 ] |= image[ yy+2 ];
			image[ yy ]   |= image[ yy+2 ];
		}
	}
	else // 全角
	{
		width *= 2;
		fuchi_width = width + 2;
		x_end = x_start + fuchi_width;
		if( DISP_X < x_end ) { x_end = DISP_X; }
		
		for( yy = 0; yy < height; yy += 2 )
		{
			unsigned int const lines = ( ( ( *p << 8 ) | *(p+1) ) << 8 ) | *(p+2);
			unsigned int const line0 = ( lines >> 12 ) | ( lines >> 13 ) | ( lines >> 14 );
			unsigned int const line1 = lines | ( lines >> 1 ) | ( lines >> 2 );
			p += 3;
			image[ yy+3 ]  = line1;
			image[ yy+2 ]  = line0 | line1;
			image[ yy+1 ] |= line0 | line1;
			image[ yy ]   |= line0;
		}
	}
	
	for( yy = 0, dy = y_start; yy < fuchi_height && dy < y_end; yy++, dy++ )
	{
		if( dy < 0 )
		{
			vbuff += DISP_X;
			continue;
		}
		for( xx = x_start; xx < x_end; xx++ )
		{
			if( 0 <= xx && image[ yy ] & 0x800 ) { *vbuff = color; }
			vbuff++;
			image[ yy ] <<= 1;
		}
		vbuff += DISP_X - ( x_end - x_start );
	}
	return ( height << 8 ) + width;
}

/**
	フォントを仮想画面に表示（背景透過）.
	文字色、背景色の設定を変更したくないこと、背景透過と決まっていることから、
	pceFontPut() を簡単化したものを作成した。
	@param x x座標
	@param y y座標
	@param code 文字コード
	@return
		- b0-7 x方向の幅
		- b8-15 y方向の幅
*/
static unsigned short put_moji( int x, int y, unsigned short code )
{
	unsigned char const* p = pceFontGetAdrs( code );
	unsigned char* vbuff = pceLCDSetBuffer( INVALIDPTR ) + ( x + DISP_X * y );
	unsigned char const color = s_tx_color;
	int xx, yy;
	int width = s_font_width[ s_type ];
	int const height = s_font_height[ s_type ];
	int x_end = x + width;
	int y_end = y + height;
	if( DISP_X < x_end ) { x_end = DISP_X; }
	if( DISP_Y < y_end ) { y_end = DISP_Y; }
	
	if( (int)p & 0x80000000 ) // 半角
	{
		for( yy = y; yy < y_end; yy++ )
		{
			unsigned char line = *p++;
			if( yy < 0 )
			{
				vbuff += DISP_X;
				continue;
			}
			for( xx = x; xx < x_end; xx++ )
			{
				if( 0 <= xx && line & 0x80 ) { *vbuff = color; }
				vbuff++;
				line <<= 1;
			}
			vbuff += DISP_X - ( x_end - x );
		}
	}
	else // 全角
	{
		width *= 2;
		x_end = x + width;
		if( DISP_X < x_end ) { x_end = DISP_X; }
		
		for( yy = y; yy < y_end; yy += 2 )
		{
			BOOL const y1_is_inside = ( 0 <= yy + 1 && yy + 1 < DISP_Y );
			unsigned int lines = ( ( ( *p << 8 ) | *(p+1) ) << 8 ) | *(p+2);
			p += 3;
			if( yy < 0 )
			{
				vbuff += DISP_X;
				continue;
			}
			for( xx = x; xx < x_end; xx++ )
			{
				if( 0 <= xx )
				{
					if( lines & 0x800000 ) { *vbuff = color; }
					if( y1_is_inside && lines & 0x800 ) { *(vbuff+DISP_X) = color; }
				}
				vbuff++;
				lines <<= 1;
			}
			vbuff += DISP_X * 2 - ( x_end - x );
		}
	}
	return ( height << 8 ) + width;
}

unsigned short FontFuchi_Put( int x, int y, unsigned short code )
{
	int const now_type = FontProxy_GetType();	// 現在のフォント設定を退避
	unsigned short result = 0;

	FontProxy_SetType( s_type );
	put_fuchi( x, y, code );
	result = put_moji( x, y, code );
	
	FontProxy_SetType( now_type );	// フォント設定を復元
	return result;
}

/**
	FontFuchi_PutStr() の実装.
	@param type 0 なら put_fuchi()、それ以外なら put_moji()
	@param p 文字列のアドレス。0 で終端。
*/
static int put_str( int type, char const* p )
{
	union
	{
		unsigned short s;
		struct
		{
			unsigned char w;
			unsigned char h;
		} b;
	} m;
	unsigned char ch;
	int x = s_x;
	int y = s_y;
	m.b.h = s_font_height[ s_type ];

	while( 1 )
	{
		ch = *p++;
		if( !ch )
		{
			break;
		}
		else if( ( 0x81 <= ch && ch <= 0x9F ) || ( 0xE0 <= ch && ch <= 0xFC ) ) // 全角
		{
			if( *p )
			{
				unsigned short const code = ( ch << 8 ) | (unsigned char)*p++;
				m.s = ( type )? put_moji( x, y, code ) : put_fuchi( x, y, code );
				x += m.b.w;
			}
			else
			{
				break;
			}
		}
		else if( ch == '\n' ) // LF
		{
			goto CRLF;
		}
		else // 半角
		{
			m.s = ( type )? put_moji( x, y, ch ) : put_fuchi( x, y, ch );
			x += m.b.w;
		}
		if( s_right <= x )
		{
CRLF:
			x = s_left;
			y += m.b.h;
			if( s_bottom <= y )
			{
				y = s_top;
			}
		}
	}
	s_x = x;
	s_y = y;
	return ch;
}

int FontFuchi_PutStr( char const* p )
{
	int const now_type = FontProxy_GetType();	// 現在のフォント設定を退避
	int const now_x = s_x;
	int const now_y = s_y;
	int result = 0;

	FontProxy_SetType( s_type );
	put_str( 0, p );
	FontFuchi_SetPos( now_x, now_y );
	result = put_str( 1, p );

	FontProxy_SetType( now_type );	// フォント設定を復元
	return result;
}

int FontFuchi_Printf( char const* fmt, ... )
{
	// システムで確保している画面バッファを文字列展開に利用
	extern unsigned char _def_vbuff[];
	int result;
	va_list ap;

	va_start( ap, fmt );
	result = vsprintf( _def_vbuff, fmt, ap );
	va_end( ap );

	FontFuchi_PutStr( _def_vbuff );
	return result;
}
