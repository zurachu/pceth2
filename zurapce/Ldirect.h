#if !defined( ZURAPCE_LDIRECT_H )
#define ZURAPCE_LDIRECT_H

/** @file
	16階調描画関連.
	液晶の残像を利用して、5枚の画像を高速に切り替え、16階調の画像を表示しているようにみせかけます。
	さらに、通常の4階調描画を別レイヤーとして重ね合わせて表示できます。
	
	処理が重いですが、コードを高速 RAM に配置することで高速化できます。
	project.mak の USE_LDIRECT_ON_FRAM := true を有効にして、
	pceAppInit() の先頭で FramObject_Init() を呼び出して下さい。
	
	@see @ref pgx_format

	@author zurachu
*/

#include <piece.h>

#define LDIRECT_PAGE_NUM	5 ///< 切り替えページ数
#define LDIRECT_COLOR_NUM	16 ///< 階調数（LDIRECT_PAGE_NUM * 3 + 1）

/** @page pgx_format 16階調画像フォーマット
	PIECE_BMP の拡張フォーマットです。

	構造体は PIECE_BMP をそのまま使うため、
	通常の PIECE_BMP と同様に PieceBmp_Construct() で初期化できます。
	元データとしては、 16階調 BMP を pgd16cnv で変換したものを渡してください。
	https://github.com/zurachu/pgd16cnv

@code
	typedef struct {
		DWORD head; // 識別子 'PBEX'（既存アプリの誤動作防止）
		DWORD fsize; // ファイル長
		BYTE bpp; // bit 深度 (4)
		BYTE mask; // マスクの bit 深度 (1)
		int w; // X 幅 8 ピクセル単位厳守
		int h; // Y 高さ
		DWORD buf_size; // BMP サイズ = ( bpp + mask ) * w * h / 8
	} PBMP_FILEHEADER;
	
	typedef struct {
		PBMP_FILEHEADER	header;
		BYTE *buf; // 4 BIT 1 ピクセル
		BYTE *mask; // 1 BIT 1 ピクセル
	} PIECE_BMP;
@endcode
*/

/** @page support_capturing Windows側キャプチャソフトの16階調対応
	Windows側でP/ECEの画面をキャプチャするソフトで、16階調描画に対応させる方法です。

	- P/ECE monitor（nsawa氏） http://www.piece-me.org/

	  実行後、右クリックメニューで「16階調モード」にチェックを入れてください。

	- pceCaps（まかべひろし氏） リンク切れ

	  readme.txt の通り、 usbcapt.h をインクルードして、
	  Ldirect_Init() の前に usbCaptureInit() を、
	  Ldirect_Exit() の後に usbCaptureRelease() を呼び出してください。

	  pcecaps.exe 実行後は、 File→option→MultiColorで以下の設定を行ってください。
	  - 1フレームの時間間隔: pceAppSetProcPeriod() の値（うまく表示されない場合は調整が必要かも）
	  - フレーム枚数: 5
*/

/**
	初期化.
	仮想画面バッファ、16階調用描画バッファ、4階調用描画バッファ、
	LCD ダイレクト転送用バッファ（5枚）を確保します。
	また、 pceLCDSetBuffer() をフックして、戻り値で仮想画面バッファの代わりに
	4階調用描画バッファのアドレスを返すようにします。
	（これにより、既存の4階調描画関数は4階調用描画バッファに描き込まれます）
	16階調用描画バッファのアドレスは Ldirect_Buffer() で取り出せます。
	仮想画面バッファのアドレスは取り出せませんが、直接アクセスする必要はないはずです。
	@attention
		Lcd_Init() とは排他的になっているので、両方を呼び出さないようにしてください。
	@retval 1 バッファ確保成功
	@retval 0 バッファ確保失敗
*/
int Ldirect_Init( void );

/**
	終了時処理.
	pceLCDSetBuffer() のフックをもとに戻します。
*/
void Ldirect_Exit( void );

/**
	16階調用描画バッファのアドレスを取得.
	バッファの構造は、通常の4階調用仮想画面バッファと同じです。
	1ドット1バイト（下位4ビットのみ使用＝16色）
	横128ドット×縦88ドット = 11246バイト
@verbatim
Ldirect_Buffer()→+---------... ...------------+
                  |                            |
                  |                            |  ↑
                  .                            .
                  .                            .  Y 88 ドット
                  .                            .
                  |                            |  ↓
                  |                            |
                  +---------... ...------------+
                         ← X 128 ドット→
@endverbatim
*/
BYTE* Ldirect_Buffer( void );

/**
	LCD ダイレクト転送用バッファを更新します.
	実際には、内部で更新フラグを立てるだけで、
	Ldirect_Trans() で16階調用描画バッファと4階調用描画バッファの内容を
	仮想画面バッファに重ね描きし、仮想画面バッファを1フレームに1枚ずつ
	LCD ダイレクト転送用バッファに転送します。
*/
void Ldirect_Update( void );

/**
	Ldirect_Trans() で4階調用描画バッファの内容を仮想画面バッファに
	重ね描きするかどうかを設定します.
	@param visible フラグ
*/
void Ldirect_VBuffView( BOOL visible );

/**
	LCD ダイレクト転送.
	LCD ダイレクト転送用バッファを切り替えて、 LCD バッファに転送します。
	毎フレーム呼び出して下さい。
	Ldirect_Update() が呼び出されてからまだ仮想画面バッファが更新されていない場合、
	16階調用描画バッファと4階調用描画バッファの内容を仮想画面に重ね描きします。
	また、仮想画面バッファの内容がこのフレームで使用される LCD ダイレクト転送用
	バッファに反映されていない場合、反映してから転送します。
*/
void Ldirect_Trans( void );

/**
	4階調用描画バッファを透過色（COLOR_MASK）で塗り潰す.
	@param x 左上 X 座標
	@param y 左上 Y 座標
	@param w 幅
	@param h 高さ
*/
void Ldirect_VBuffClear( int x, int y, int w, int h );

/**
	16階調用描画バッファに16階調画像を描画.
	@param p 16階調 PIECE_BMP へのポインタ
	@param dx 描画先左上 X 座標
	@param dy 描画先左上 Y 座標
	@param sx 描画元左上 X 座標
	@param sy 描画元左上 Y 座標
	@param width 幅
	@param height 高さ
	@retval 0 描画なし
	@retval 1 描画完了
	
	@see @ref pgx_format
*/
int Ldirect_DrawObject( PIECE_BMP const* p, int dx, int dy, int sx, int sy
						, int width, int height );

/**
	16階調用描画バッファに点を描画.
	@param color 色
	@param x X座標
	@param y Y座標
*/
void Ldirect_Point( BYTE color, int const x, int const y );

/**
	16階調用描画バッファに矩形を描画.
	@param color 色
	@param x 左上X座標
	@param y 左上Y座標
	@param width 幅
	@param height 高さ
*/
void Ldirect_Paint( BYTE color, int x, int y, int width, int height );

#endif	// !defined( ZURAPCE_LDIRECT_H )
