#if !defined( ZURAPCE_FONT_PROXY_H )
#define ZURAPCE_FONT_PROXY_H

/** @file
	フォント設定の代行処理.
	pceFontSet 系 API で設定した内容を取得する API が公開されていないため、
	API をフックして設定値を保存しておき、取得できるようにします。
	@warning
	API をフックしない状態で pceFontSet～() を直接呼び出すと、
	設定内容の整合性が取れなくなるので注意して下さい。
	@author zurachu
*/

/**
	フォントの種類や表示属性を指定.
	@param type
		- bit0～6 : フォントの種類
			- 0 通常フォント 半角5×10ドット 全角10×10ドット
			- 1 拡大フォント 半角8×16ドット ボールド (ASCIIのみ)
			- 2 極小フォント 半角4× 6ドット (ASCIIのみ)
		- bit7 :
			 - 0 の時は、画面右端で自動改行する
			 - 1 の時は、自動改行しない
*/
void FontProxy_SetType( int type );

/**
	フォントの種類や表示属性を取得.
*/
int FontProxy_GetType( void );

/**
	文字色を指定.
	@param color 色（０～３）
*/
void FontProxy_SetTxColor( int color );

/**
	文字色を取得.
*/
int FontProxy_GetTxColor( void );

/**
	背景色を指定.
	@param color 色（０～３）FC_SPRITE の場合は透過
*/
void FontProxy_SetBkColor( int color );

/**
	背景色を取得.
*/
int FontProxy_GetBkColor( void );

/**
	フォント設定に FontProxy_Set～() を使うよう、API をフックします.
*/
void FontProxy_Hook_Set( void );

/**
	フォント設定に通常の pceFontSet～() を使うよう、API のフックを解除します.
*/
void FontProxy_Unhook_Set( void );

#endif // !defined( ZURAPCE_FONT_PROXY_H )

