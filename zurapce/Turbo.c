#include "Turbo.h"

/** @file
	48 MHz 動作関連の実装.
*/

// 24MHz/48MHz切替等設定I/Oポート
#define	port_P0x	(*(volatile unsigned char*)0x402D1)

#define	reg_A6_4C	(*(volatile unsigned short int*)0x4812A)
#define	reg_A10_9C	(*(volatile unsigned short int*)0x48126)

// 16bitタイマn 制御レジスタ
#define reg_TC0		(*(volatile unsigned short int*)0x48186)
#define reg_TC1		(*(volatile unsigned short int*)0x4818E)
#define reg_TC4		(*(volatile unsigned short int*)0x481A6)

// 16bitタイマ0 コンペアデータB 設定レジスタ (1msタイマ) デフォルト=5999
#define reg_CR0B	(*(volatile unsigned short int*)0x48182)

// 16bitタイマx クロックコントロールレジスタ
//---------------------------------------------
// * bit 2:0が分周比
//   7: f/4096  6: f/1024  5: f/256   4: f/64 
//   3: f/16    2: f/4     1: f/2     0: f/1 
//---------------------------------------------
#define reg_CC1	(*(volatile unsigned char*)0x40148)
#define reg_CC4	(*(volatile unsigned char*)0x4014B)

// * 参考 *
// PIECE/sysdev/pcekn/snd.c    PWMC      750
// PIECE/sysdev/pcekn/timer.c  TMCOUNTER 6000

static void stop_timer( void )
{
	reg_TC0 &= ~4;
	reg_TC1 &= ~4;
	reg_TC4 &= ~4;
}

static void start_timer( void )
{
	reg_TC0 |= 4;
	reg_TC1 |= 4;
	reg_TC4 |= 4;
}

void Turbo_Init( void )
{	// ベースクロック/分周比/コンペアデータ
	stop_timer();	// カウント一時停止

	port_P0x &= ~0x80;	// ベースクロックを 48 MHz に
	reg_A6_4C  = (reg_A6_4C  & ~0x7) | 1;	// SRAM アクセス = 1 ウェイト
	reg_A10_9C = (reg_A10_9C & ~0x7) | 1;	// フラッシュ ROM アクセス = 1 ウェイト

	reg_CR0B = 11999;				// 48 MHz / 4 / 12000 =  1 kHz ( 1 ms カウンタ )
	reg_CC1 = (reg_CC1 & ~0x7) | 1;	// 48 MHz / 2 / 750   = 32 kHz ( PWM 音源用 )
	reg_CC4 = (reg_CC4 & ~0x7) | 1;	// 48 MHz / 2 / 4     =  6 MHz ( USB 用クロック )

	start_timer();	// カウント開始
}

void Turbo_Exit( void )
{
	stop_timer();	// カウント一時停止

	reg_A6_4C  = (reg_A6_4C  & ~0x7) | 2;	// SRAM アクセス = 2 ウェイト
	reg_A10_9C = (reg_A10_9C & ~0x7) | 2;	// フラッシュ ROM アクセス = 2 ウェイト

	port_P0x |= 0x80;	// ベースクロックを24MHzに.
	reg_CR0B = 5999;				// 24 MHz / 4 / 6000 =  1 kHz ( 1 ms カウンタ )
	reg_CC1 = (reg_CC1 & ~0x7) | 0;	// 24 MHz / 1 / 750  = 32 kHz ( PWM 音源用 )
	reg_CC4 = (reg_CC4 & ~0x7) | 0;	// 24 MHz / 1 / 4    =  6 MHz ( USB 用クロック )

	start_timer();	// カウント開始
}

