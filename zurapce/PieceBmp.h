#if !defined( ZURAPCE_PIECE_BMP_H )
#define ZURAPCE_PIECE_BMP_H

/** @file
	PIECE_BMP 関連.
	@author zurachu
*/

#include <piece.h> // PIECE_BMP

/**
	PIECE_BMP の初期化.
	BMP コンバータで変換したデータを、PIECE_BMP に登録します。
	@param p PIECE_BMP 構造体へのポインタ
	@param source 元データ
*/
void PieceBmp_Construct( PIECE_BMP* p, unsigned char* source );

/**
	PIECE_BMP の描画.
	@param p PIECE_BMP 構造体へのポインタ
	@param dx 描画先左上 X 座標
	@param dy 描画先左上 Y 座標
	@param sx 描画元左上 X 座標
	@param sy 描画元左上 Y 座標
	@param width 幅
	@param height 高さ
	@param param パラメータ
	@return pceLCDDrawObject() の戻り値（0:描画なし, 1:描画完了）
*/
int PieceBmp_Draw( PIECE_BMP* p, int dx, int dy, int sx, int sy
				, int width, int height, int param );

/**
	一定サイズの複数の画像を縦横に並べた PIECE_BMP.
*/
struct UnitedPieceBmp
{
	PIECE_BMP bmp; ///< PIECE_BMP 構造体
	int width; ///< 画像１つの幅
	int height; ///< 画像１つの高さ
};
typedef struct UnitedPieceBmp UnitedPieceBmp;

/**
	UnitedPieceBmp の初期化.
	@param p UnitedPieceBmp 構造体へのポインタ
	@param source 元データ
	@param width 画像１つの幅
	@param height 画像１つの高さ
*/
void UnitedPieceBmp_Construct( UnitedPieceBmp* p
	, unsigned char* source, int width, int height );

/**
	分割数を指定しての UnitedPieceBmp の初期化.
	@param p UnitedPieceBmp 構造体へのポインタ
	@param source 元データ
	@param divx X 方向の分割数
	@param divy Y 方向の分割数
*/
void UnitedPieceBmp_ConstructByDivision( UnitedPieceBmp* p
	, unsigned char* source, int divx, int divy );

/**
	UnitedPieceBmp の分割数を取得.
	@param p UnitedPieceBmp 構造体へのポインタ
*/
int UnitedPieceBmp_NumDivision( UnitedPieceBmp const* p );

/**
	UnitedPieceBmp の分割数を X, Y 方向それぞれ個別に取得.
	@param p UnitedPieceBmp 構造体へのポインタ
	@param [out] x X 方向の分割数格納先
	@param [out] y Y 方向の分割数格納先
*/
void UnitedPieceBmp_NumDivisionByXY( UnitedPieceBmp const* p, int* x, int* y );

/**
	UnitedPieceBmp の描画.
	@param p UnitedPieceBmp 構造体へのポインタ
	@param dx 描画先左上 X 座標
	@param dy 描画先左上 Y 座標
	@param index 描画する画像のインデックス番号.
	@param param パラメータ
	@return pceLCDDrawObject() の戻り値（0:描画なし, 1:描画完了）
	index は左上から 0,1,... と進み、右端まで行ったら下の行の左端から数えます。
@verbatim
X 方向４分割、 Y 方向２分割の場合
+-+-+-+-+
|0|1|2|3|
+-+-+-+-+
|4|5|6|7|
+-+-+-+-+
@endverbatim
*/
int UnitedPieceBmp_Draw( UnitedPieceBmp* p
	, int dx, int dy, int index, int param );

/**
	X, Y 方向で個別にインデックスを指定して UnitedPieceBmp の描画.
	@param p UnitedPieceBmp 構造体へのポインタ
	@param dx 描画先左上 X 座標
	@param dy 描画先左上 Y 座標
	@param ix X 方向インデックス
	@param iy Y 方向インデックス
	@param param パラメータ
	@return pceLCDDrawObject() の戻り値（0:描画なし, 1:描画完了）
*/
int UnitedPieceBmp_DrawByXY( UnitedPieceBmp* p
	, int dx, int dy, int ix, int iy, int param );

#endif // !defined( ZURAPCE_PIECE_BMP_H )
