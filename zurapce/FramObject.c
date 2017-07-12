#include "FramObject.h"

/** @file
	高速 RAM 配置オブジェクト関連の実装.
*/

#include <string.h>

void FramObject_Init( void )
{
	extern unsigned long __START_00001000[]; // 転送先
	extern unsigned long __SIZEOF_00001000[]; // コードサイズ
	extern unsigned long __START_FramObject_Top_code[]; // 転送元
	memcpy( __START_00001000, __START_FramObject_Top_code, (int)__SIZEOF_00001000 );
}

