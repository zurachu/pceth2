#include "FontExtend.h"

/** @file
	拡張フォント関連の実装.
	@author zurachu
*/

#include <piece.h>
#include "vector.h"

/// 拡張フォントテーブル構造体
struct FONT_TABLE
{
	unsigned short code; ///< 文字コード
	unsigned char const* data; ///< フォントデータ参照
};

/// !?
static unsigned char const s_font_interrobang[] =
{
	0x00, 0x02, 0x38,
	0x24, 0x42, 0x44,
	0x20, 0x82, 0x10,
	0x21, 0x00, 0x00,
	0x21, 0x00, 0x00,
};

/// !!
static unsigned char const s_font_exclamation[] =
{
	0x00, 0x02, 0x10,
	0x21, 0x02, 0x10,
	0x21, 0x02, 0x10,
	0x21, 0x00, 0x00,
	0x21, 0x00, 0x00,
};

/// ハートマーク
static unsigned char const s_font_heart[] =
{
	0x00, 0x03, 0x18,
	0x7b, 0xc7, 0xfc,
	0x7f, 0xc3, 0xf8,
	0x1f, 0x00, 0xe0,
	0x04, 0x00, 0x00,
};

/// ～（２キャラ）の左側
static unsigned char const s_font_wave_left[] =
{
	0x00, 0x00, 0x00,
	0x00, 0x01, 0xf8,
	0x60, 0x40, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

/// ～（２キャラ）の右側
static unsigned char const s_font_wave_right[] =
{
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x81, 0x87, 0xe0,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

/// 拡張フォントテーブル
static struct FONT_TABLE const s_font_table[] = 
{
	{ 0xF040, s_font_interrobang },
	{ 0xF041, s_font_exclamation },
	{ 0xF042, s_font_heart },
	{ 0xF046, s_font_wave_left },
	{ 0xF047, s_font_wave_right },
};

/// 通常の pceFontGetAdrs を退避
static unsigned char const* (*old_pceFontGetAdrs)( unsigned short code ) = NULL;

unsigned char const* FontExtend_GetAdrs( unsigned short code )
{
	static int const table_num = sizeof(s_font_table) / sizeof(s_font_table[0]);
	int i;
	for( i = 0; i < table_num; i++ )
	{
		if( code == s_font_table[ i ].code )
		{
			return s_font_table[ i ].data;
		}
	}
	return old_pceFontGetAdrs
			? old_pceFontGetAdrs( code )
			: pceFontGetAdrs( code );
}

void FontExtend_Hook_GetAdrs( void )
{
	if( !old_pceFontGetAdrs )
	{
		old_pceFontGetAdrs = pceVectorSetKs( KSNO_FontGetAdrs, FontExtend_GetAdrs );
	}
}

void FontExtend_Unhook_GetAdrs( void )
{
	if( old_pceFontGetAdrs )
	{
		pceVectorSetKs( KSNO_FontGetAdrs, old_pceFontGetAdrs );
		old_pceFontGetAdrs = NULL;
	}
}

