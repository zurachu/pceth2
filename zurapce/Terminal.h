#if !defined( ZURAPCE_TERMINAL_H )
#define ZURAPCE_TERMINAL_H

/**	@file
	拡張端子関連.
	P/ECE HAND BOOK Vol.2 で提唱された拡張端子への入出力を提供します。
	P/ECE HAND BOOK Vol.2 掲載ソースコードの間違いを修正して取り込み。

@verbatim

『P/ECE Hand Book VOL.2』の「LEDランプで拡張端子の使い方を知る」の記事を試していたところ、ちょっと間違いを見つけました。
081ページに掲載されているLED TESTアプリケーションのソースコード、29～30行目の“ポートの初期化”のところ、
	P0CFP = ( P0IOC & 0x8F ) | 0x00; // ポートとして扱う
	P1CFP = ( P1IOC & 0xEB ) | 0x00; // ポートとして扱う
は間違いで、正しくは
	P0CFP = ( P0CFP & 0x8F ) | 0x00; // ポートとして扱う
	P1CFP = ( P1CFP & 0xEB ) | 0x00; // ポートとして扱う
ですね。
付属CD-ROMに入っているソースコードと実行ファイルにも同じ間違いがあるようで、実行後にP/ECEの動作が不安定になる場合があります。
@endverbatim
	P/ECE研究室（nsawa氏）2004/05/09 20:54 より
	http://www.piece-me.org/
	
	@author zurachu
*/

/// 入力
#define T_IN (0)

/// 出力
#define T_OUT (1)

/**
	拡張端子の入出力を初期化.
	それぞれ T_IN または T_OUT を指定してください。
	@param p1 ポート１（拡張端子１）
	@param p2 ポート２（拡張端子４）
	@param p3 ポート３（拡張端子６）
	@param p4 ポート４（拡張端子７）
	@param p5 ポート５（拡張端子８）
*/
void Terminal_Init( unsigned char p1
				  , unsigned char p2
				  , unsigned char p3
				  , unsigned char p4
				  , unsigned char p5 );

/**
	拡張端子にデータを出力.
	@param port 出力先ポート（１～５）
	@param data データ（０／１）
*/
void Terminal_Set( int port, unsigned char data );

/**
	拡張端子からデータを入力.
	@param port 入力元ポート（１～５）
	@return データ（０／１）
*/
int Terminal_Get( int port );

#endif // !defined( ZURAPCE_TERMINAL_H )

