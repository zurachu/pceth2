#if !defined( ZURAPCE_FONT_EXTEND_H )
#define ZURAPCE_FONT_EXTEND_H

/** @file
	拡張フォント関連.
	API をフックすることで、pceFont 系 API に拡張フォントを追加します。
	拡張フォントは、AQUAPLUS 製ゲームの仕様に準じる予定です。
@verbatim
	拡張フォントの文字コード
	- 0xF040 - 「!?」
	- 0xF041 - 「!!」
	- 0xF042 - ハートマーク
	- 0xF046 - ２文字分の「～」の左側
	- 0xF047 - ２文字分の「～」の右側
@endverbatim
	
	@author zurachu
*/

/**
	フォントのアドレスを取得.
	拡張フォント文字コードと照合した後、通常の pceFontGetAdrs() を呼びます。
	@param code 文字コード
	@return フォントのアドレス
*/
unsigned char const* FontExtend_GetAdrs( unsigned short code );

/**
	フォントアドレス取得に FontExtend_GetAdrs() を使うよう、API をフックします.
*/
void FontExtend_Hook_GetAdrs( void );

/**
	フォントアドレス取得に通常の pceFontGetAdrs() を使うよう、API のフックを解除します.
*/
void FontExtend_Unhook_GetAdrs( void );

#endif // !defined( ZURAPCE_FONT_EXTEND_H )

