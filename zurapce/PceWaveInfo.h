#if !defined( ZURAPCE_PCEWAVEINFO_H )
#define ZURAPCE_PCEWAVEINFO_H

/** @file
	PCEWAVEINFO 関連.
	@author zurachu
*/

#include <piece.h> // PCEWAVEINFO

/**
	PCEWAVEINFO の初期化.
	PCM コンバータで変換したデータを、PCEWAVEINFO に登録します。
	@param p PCEWAVEINFO 構造体へのポインタ
	@param source 元データ
*/
void PceWaveInfo_Construct( PCEWAVEINFO* p, unsigned char* source );

#endif // !defined( ZURAPCE_PCEWAVEINFO_H )
