#if !defined( ZURAPCE_PIECE_BMP_ANIMATION_H )
#define ZURAPCE_PIECE_BMP_ANIMATION_H

/** @file
	PIECE_BMP を使った連番アニメーション関連.
	@author zurachu
*/

#include "PieceBmp.h"

/**
	PIECE_BMP を使った連番アニメーション再生機構.
*/
struct PieceBmpAnimation
{
	UnitedPieceBmp* playing; ///< 再生中の画像
	int current_index; ///< 現在のインデックス
	int start_index; ///< 先頭のインデックス
	int end_index; ///< 最終のインデックス
	int delta; ///< 経過時間（更新周期に達する毎に次のインデックスに進みます）
	int update_period; ///< 更新周期
	BOOL loop; ///< ループするか否か
};
typedef struct PieceBmpAnimation PieceBmpAnimation;

/**
	連番アニメーションの再生を開始.
*/
void PieceBmpAnimation_Start( PieceBmpAnimation* p
	, UnitedPieceBmp* bmp, int start, int end, int update_period, BOOL loop );

/**
	画像全てを使った連番アニメーションの再生を開始.
	start = 0, end は bmp から自動で算出します。
*/
void PieceBmpAnimation_StartToEnd( PieceBmpAnimation* p
	, UnitedPieceBmp* bmp, int update_period, BOOL loop );

/**
	連番アニメーションを更新.
	@param p PieceBmpAnimation 構造体へのポインタ
	@param delta 前回呼び出しからの経過時間.
*/
void PieceBmpAnimation_Update(PieceBmpAnimation* p, int delta );

/**
	再生中の UnitePieceBmp を返す.
	@param p PieceBmpAnimation 構造体へのポインタ
	@return 再生中でない場合 NULL
*/
UnitedPieceBmp* PieceBmpAnimation_Playing( PieceBmpAnimation const* p );

/**
	PieceBmpAnimation が終了しているかどうか.
	終了＝非ループ再生で最終のインデックスに到達したか、非再生中か。
	@param p PieceBmpAnimation 構造体へのポインタ
*/
BOOL PieceBmpAnimation_IsEnd( PieceBmpAnimation const* p );

/**
	PieceBmpAnimation の描画.
	@param p PieceBmpAnimation 構造体へのポインタ
	@param dx 描画先左上 X 座標
	@param dy 描画先左上 Y 座標
	@param param パラメータ
	@return pceLCDDrawObject() の戻り値（0:描画なし, 1:描画完了）
*/
int PieceBmpAnimation_Draw( PieceBmpAnimation const* p
	, int dx, int dy, int param );

/**
	PieceBmpAnimation の再生をクリア.
*/
void PieceBmpAnimation_Clear( PieceBmpAnimation* p );

#endif // !defined( ZURAPCE_PIECE_BMP_ANIMATION_H )
