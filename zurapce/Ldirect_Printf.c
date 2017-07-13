/** @file
	16階調表示時の pceFontPrintf() の実装.
	Ldirect.c に実装しているとヘッダインクルード順の関係で
	以下エラーが出てしまうため、別ファイルで定義。
	C:\usr\PIECE\include/stdarg.h:25: redefinition of `va_list'
	C:\usr\PIECE\include/piece.h:349: `va_list' previously declared here
	
	@author zurachu
*/

#include <stdarg.h>
#include <piece.h>
#include <smcvals.h>

unsigned int new_pceFontPrintf( const char *fmt, ... )
{
	// システムで確保している画面バッファを文字列展開に利用
	extern unsigned char _def_vbuff[];
	int result;
	va_list ap;

	va_start( ap, fmt );
	result = vsprintf( _def_vbuff, fmt, ap );
	va_end( ap );

	pceFontPutStr( _def_vbuff ); // フック済みの想定
	return result;
}

