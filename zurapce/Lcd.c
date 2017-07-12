#include "Lcd.h"

/**
	LCD 関連の実装.
	@author zurachu
*/

#include <piece.h>
#include <string.h>

/// 仮想画面バッファ
static unsigned char* s_vbuff = NULL;

/// LCD 更新フラグ
static BOOL s_lcd_update;

int Lcd_Init( void )
{
	pceLCDDispStop();
	s_vbuff = pceHeapAlloc( DISP_X * DISP_Y );
	if( s_vbuff )
	{
		memset( s_vbuff, 0, DISP_X * DISP_Y );
		pceLCDSetBuffer( s_vbuff );
		s_lcd_update = TRUE;
	}
	pceLCDDispStart();
	return s_vbuff? 1 : 0;
}

void Lcd_Update( void )
{
	s_lcd_update = TRUE;
}

void Lcd_Trans( void )
{
	if( s_lcd_update )
	{
		pceLCDTrans();
		s_lcd_update = FALSE;
	}
}

