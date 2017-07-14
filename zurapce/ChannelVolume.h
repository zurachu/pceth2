#if !defined( ZURAPCE_CHANNEL_VOLUME_H )
#define ZURAPCE_CHANNEL_VOLUME_H

/** @file
	各 ch の音量フェード処理関連.
	@author zurachu
*/

/**
	初期化.
	音量フェードを利用する場合、最初に呼び出して下さい。
	直前に実行していたアプリの ch 毎の音量設定を引き継がないよう、
	初期化時に ch 毎の音量を最大に戻します。
*/
void ChannelVolume_Init( void );

/**
	終了.
	フェードで ch の音量が下がったままで、次に実行するアプリで
	ch 毎の音量を設定していない場合、設定が引き継がれて音が聞こえなく
	なってしまう場合があるので、終了時に ch 毎の音量を最大に戻します。
*/
void ChannelVolume_Exit( void );

/**
	状態の更新.
	全 ch のフェード状態を1フレーム進めます。
	pceAppProc() 毎に1回か、定期的に呼び出してください。
*/
void ChannelVolume_Update( void );

/**
	現在の音量からフェードを開始します.
	@param ch チャンネル
	@param vol 最終音量
	@param fade_time フェード時間（フレーム数）
*/
void ChannelVolume_Fade( int ch, int vol, int fade_time );

/**
	フェードインします.
	最小音量から最大音量に向かってフェードを開始します。
	@param ch チャンネル
	@param fade_time フェード時間（フレーム数）
*/
void ChannelVolume_FadeIn( int ch, int fade_time );

/**
	フェードアウトします.
	現在の音量から最小音量に向かってフェードを開始します。
	@param ch チャンネル
	@param fade_time フェード時間（フレーム数）
*/
void ChannelVolume_FadeOut( int ch, int fade_time );

#endif // !defined( ZURAPCE_CHANNEL_VOLUME_H )
