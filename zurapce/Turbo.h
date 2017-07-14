#if !defined( ZURAPCE_TURBO_H )
#define ZURAPCE_TURBO_H

/** @file
	48 MHz 動作関連.
	ささお氏の「P/ECEを48MHzモードで動かすための小細工 ver0.9b」を取り込み。
	http://junkbox.info/piece/
@code
//---------------------------------------------------------------------
//  - このソースは改造・再配布など自由にお使いください。許可は不要です。
//  - 動作確認は、BIOS ver1.18 2002.01.16 上で行っています。
//  - 使用は自己責任でお願いします。
//  - 現時点では、48MHzモードにおけるLCD/サウンド再生/1msタイマの正しい
//    動作と、48MHzモードから24MHzモードに復帰した場合に正常にUSBアクセ
//    スが行えることのみ確認しています。
//  - ソースを送っていただければ、上記以外についても対応していきます。
//---------------------------------------------------------------------
//  pceCPUSetSpeed()命令と併用する場合には、Turbo_Init/Exitを必ず以下の順で
//  行ってください。
//
//  pceCPUSetSpeed(n); の後に Turbo_Init();
//  Turbo_Exit(); の後に pceCPUSetSpeed(n);
//---------------------------------------------------------------------
@endcode

	@author zurachu
*/

/**
	48 MHz モードに切り替えます.
*/
void Turbo_Init( void );

/**
	標準 ( 24 MHz ) モードに切り替えます.
	pceAppExit() には 必ず Turbo_Exit(); を書くようにしてください
*/
void Turbo_Exit( void );

#endif // !defined( ZURAPCE_TURBO_H )
