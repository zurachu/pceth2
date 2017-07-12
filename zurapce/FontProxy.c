#include "FontProxy.h"

/** @file
	フォント設定の代行処理の実装.
	@author zurachu
*/

#include <piece.h>
#include "vector.h"

static int s_type = 0; ///< 種類
static int s_tx_color = 3; ///< 文字色
static int s_bk_color = 0; ///< 背景色

/// 通常の pceFontSet 系関数を退避
static void (*old_pceFontSetType)( int type ) = NULL;
static void (*old_pceFontSetTxColor)( int color ) = NULL;
static void (*old_pceFontSetBkColor)( int color ) = NULL;

void FontProxy_SetType( int type )
{
	s_type = type;
	old_pceFontSetType ? old_pceFontSetType( type ) : pceFontSetType( type );
}

int FontProxy_GetType( void )
{
	return s_type;
}

void FontProxy_SetTxColor( int color )
{
	s_tx_color = color;
	old_pceFontSetTxColor ? old_pceFontSetTxColor( color ) : pceFontSetTxColor( color );
}

int FontProxy_GetTxColor( void )
{
	return s_tx_color;
}

void FontProxy_SetBkColor( int color )
{
	s_bk_color = color;
	old_pceFontSetBkColor ? old_pceFontSetBkColor( color ) : pceFontSetBkColor( color );
}

int FontProxy_GetBkColor( void )
{
	return s_bk_color;
}

void FontProxy_Hook_Set( void )
{
	if( !old_pceFontSetType && !old_pceFontSetTxColor && !old_pceFontSetBkColor )
	{
		old_pceFontSetType = pceVectorSetKs( KSNO_FontSetType, FontProxy_SetType );
		old_pceFontSetTxColor = pceVectorSetKs( KSNO_FontSetTxColor, FontProxy_SetTxColor );
		old_pceFontSetBkColor = pceVectorSetKs( KSNO_FontSetBkColor, FontProxy_SetBkColor );
	}
}

void FontProxy_Unhook_Set( void )
{
	if( old_pceFontSetType && old_pceFontSetTxColor && old_pceFontSetBkColor )
	{
		pceVectorSetKs( KSNO_FontSetBkColor, old_pceFontSetBkColor );
		old_pceFontSetBkColor = NULL;
		pceVectorSetKs( KSNO_FontSetTxColor, old_pceFontSetTxColor );
		old_pceFontSetTxColor = NULL;
		pceVectorSetKs( KSNO_FontSetType, old_pceFontSetType );
		old_pceFontSetType = NULL;
	}
}

