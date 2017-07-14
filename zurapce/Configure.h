#if !defined( ZURAPCE_CONFIGURE_H )
#define ZURAPCE_CONFIGURE_H

/** @file
	設定保存関連.
	PIECE API リファレンス内
	pceLCDSetBright() および
	pceWaveSetMasterAtt() の説明に
@verbatim
注意！
このパラメータはユーザがシステムメニューで調整・設定する
パラメータなので、通常のアプリケーションが使用するのは
好ましくありません。
@endverbatim
	とあるので、アプリケーション内でこれを変更する場合、終了時に起動前の設定に戻すための実装。
	
	@author zurachu
*/

/**
	初期化.
	起動前のコントラスト、音量設定を保存します。
*/
void Configure_Init( void );

/**
	終了.
	起動前のコントラスト、音量設定に戻します。
*/
void Configure_Exit( void );

#endif // !defined( ZURAPCE_CONFIGURE_H )
