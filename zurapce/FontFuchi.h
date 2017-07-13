#if !defined( ZURAPCE_FONT_FUCHI_H )
#define ZURAPCE_FONT_FUCHI_H

/** @file
	縁取りフォント関連.
	仮想画面に文字を縁取りしたフォントを表示します。
	使用方法は、ほぼ pceFont 系関数互換です。

	処理が多少重いですが、コードを高速 RAM に配置することで高速化できます。
	project.mak の USE_FONT_FUCHI_ON_FRAM := true を有効にして、
	pceAppInit() の先頭で FramObject_Init() を呼び出して下さい。

	@author zurachu
*/

/**
	文字表示座標を指定.
	@param x x座標
	@param y y座標
*/
void FontFuchi_SetPos( int x, int y );

/**
	次の文字表示座標を取得.
	@param [out] x x座標. NULL の場合取得しない
	@param [out] y y座標. NULL の場合取得しない
*/
void FontFuchi_GetPos( int* x, int* y );

/**
	文字表示範囲を指定.
	@param left 左端x座標
	@param top 上端y座標
	@param right 右端x座標
	@param bottom 下端y座標
*/
void FontFuchi_SetRange( int left, int top, int right, int bottom );

/**
	文字種類を指定.
	@param type 種類（０～２）
*/
void FontFuchi_SetType( int type );

/**
	文字色を指定.
	@param color 色（０～３）
*/
void FontFuchi_SetTxColor( int color );

/**
	縁取り色を指定.
	@param color 色（０～３）
*/
void FontFuchi_SetBdColor( int color );

/**
	縁取りフォントを仮想画面に表示.
	@param x x座標
	@param y y座標
	@param code 文字コード
	@return 
			- b0-7 x方向の幅
			- b8-15 y方向の幅
	@warning
	ユーザ側で FontProxy を利用しない場合、フォント種類が上書きされてしまいます。
*/
unsigned short FontFuchi_Put( int x, int y, unsigned short code );

/**
	縁取り文字列を仮想画面に書き込みます.
	@param p 文字列のアドレス。0 で終端。
	@return もし最後の１バイトがシフト JIS の第１バイトだった場合そのコードを返します。
			その他の場合は 0 です。
	@warning
	ユーザ側で FontProxy を利用しない場合、フォント種類が上書きされてしまいます。
*/
int FontFuchi_PutStr( char const* p );

/**
	縁取り文字列を仮想画面に書き込みます（書式指定）.
	@param fmt 書式文字列
	@return 出力したバイト数
	@warning
	ユーザ側で FontProxy を利用しない場合、フォント種類が上書きされてしまいます。
*/
int FontFuchi_Printf( char const* fmt, ... );

#endif // !defined( ZURAPCE_FONT_FUCHI_H )

