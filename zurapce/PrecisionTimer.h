#if !defined( ZURAPCE_PRECISION_TIMER_H )
#define ZURAPCE_PRECISION_TIMER_H

/** @file
	高精度タイマー関連.
	@author zurachu
*/

/// 高精度タイマー構造体.
/// メンバを直接触らないこと。
struct PrecisionTimer
{
	unsigned long count; ///< 前回計測値
};
typedef struct PrecisionTimer PrecisionTimer;

/**
	高精度タイマーの初期化.
	現在の高精度タイマー値を獲得し、時間計測の基準とします。
	@param p
*/
void PrecisionTimer_Construct( PrecisionTimer* p );

/**
	高精度タイマーのカウント.
	現在の高精度タイマー値を獲得し、前回計測値との差を返します。
	また、現在の高精度タイマー値を次回時間計測の基準とします。
	pceAppProc() 毎に呼び出して、処理落ちの確認に用いるなどして下さい。
	@param p
	@return 前回計測時からの経過時間 [μsec]
	@warning pceTimerGetPrecisionCount() の仕様で、約 65 秒以上の時間の測定には使えません。
*/
unsigned long PrecisionTimer_Count( PrecisionTimer* p );

#endif // !defined( ZURAPCE_PRECISION_TIMER_H )

