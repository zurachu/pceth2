#if !defined( ZURAPCE_LINEAR_PARAMETER_H )
#define ZURAPCE_LINEAR_PARAMETER_H

/** @file
	線形時間変化パラメータ.
	初期値から最終値まで変化中の値を、線形補間で求めます。
	@author zurachu
*/

/// 線形時間変化パラメータ構造体.
/// メンバを直接触らないこと。
struct LinearParameter
{
	int start; ///< 初期値
	int diff; ///< 初期値と最終値の差分
	int frame_count; ///< フレーム数カウンタ
	int frame_max; ///< 最終値に達するフレーム数
};
typedef struct LinearParameter LinearParameter;

/**
	設定.
	@param p インスタンス
	@param start 初期値
	@param goal 最終値
	@param frame 最終値に達するフレーム数
	@warning (goal-start)*frame がオーバーフローする場合、誤動作を起こします。
*/
void LinearParameter_Set( LinearParameter* p, int start, int goal, int frame );

/**
	現在の値を取得.
	@param p インスタンス
	@return 現在の値
*/
int LinearParameter_Get( LinearParameter const* p );

/**
	状態の更新.
	フレーム数カウンタを１進めます。
	@param p インスタンス
*/
void LinearParameter_Update( LinearParameter* p );

#endif // !defined( ZURAPCE_LINEAR_PARAMETER_H )
