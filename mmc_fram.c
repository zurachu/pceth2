#include <s1c33cpu.h>
#include <string.h>
#include <stdio.h>
#include <piece.h>
#include "mmc_api.h"

#define PDWAIT	(*(volatile unsigned char*)0x1000000)	/* パワーダウンのためのウェイト領域 */
#define P0D	(*(volatile unsigned char*)0x00402D1)	/* P0入出力兼用ポートデータレジスタ */

/* クロックLo送信後のウェイト(1回必要、0回ではエラー) */
#define WAIT1 do {	\
	*pdwait = 0;	\
} while(0)

/* クロックHi送信後のウェイト(5回必要、4回ではエラー) */
#define WAIT5 do {	\
	*pdwait = 0;	\
	*pdwait = 0;	\
	*pdwait = 0;	\
	*pdwait = 0;	\
	*pdwait = 0;	\
} while(0)


//---------------------------------------------------------------------------

int
mmcWaitStartBit(void)
{
	volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	//const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;
	//
	int i;

	/* スタートビット(0)が来るまで待つ。 */
	i = MMC_RECV_TIMEOUT;
	for(;;) {

		/* クロックをLowに。(P06をLowに) */
		*p0d = lo1;
		WAIT1; /* ウェイト1回以上必要 */

		/* クロックをHighに。(P06をHighに) */
		*p0d = hi1;
		WAIT5; /* ウェイト5回以上必要 */

		/* データを取得(P04からデータを取得)。 */
		if(!(*p0d & 0x10)) {
			break; /* スタートビット(0)を検出した */
		}

		/* タイムアウトは0を返す。 */
		if(!--i) {
			return 0;
		}
	}

	return 1;
}

//---------------------------------------------------------------------------

int
mmcWaitBusy(void)
{
	volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	//const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;
	//
	int i;

	/* スタートビット(0)が来るまで待つ。 */
	i = MMC_RECV_TIMEOUT;
	for(;;) {

		/* クロックをLowに。(P06をLowに) */
		*p0d = lo1;
		WAIT1; /* ウェイト1回以上必要 */

		/* クロックをHighに。(P06をHighに) */
		*p0d = hi1;
		WAIT5; /* ウェイト5回以上必要 */

		/* データを取得(P04からデータを取得)。 */
		if(*p0d & 0x10) {
			break; /* ビジー完了(1)を検出した */

		}

		/* タイムアウトは0を返す。 */
		if(!--i) {
			return 0;
		}
	}

	return 1;
}

//---------------------------------------------------------------------------

unsigned char
mmcByteRecv(void)
{
	volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	//const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;
	//
	int v = 0;

	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x80;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x40;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x20;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x10;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x08;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x04;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x02;
	*p0d = lo1; WAIT1; *p0d = hi1; WAIT5; if(*p0d & 0x10) v |= 0x01;

	return v;
}

//---------------------------------------------------------------------------

void
mmcByteSend(int data)
{
	volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;

	/* データ連続転送以外でのバイト送信時やダミークロック送信時は、ウェイトが必要みたいです。 */
	if(data & 0x80) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x40) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x20) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x10) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x08) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x04) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x02) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
	if(data & 0x01) { *p0d = lo1; WAIT1; *p0d = hi1; WAIT5; } else { *p0d = lo0; WAIT1; *p0d = hi0; WAIT5; }
}

//---------------------------------------------------------------------------

int
mmcRecvData(unsigned char* buf, int len)
{
	volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	//const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;
	//
	int v;
	int i;

	/* 0バイト受信要求は不正なリクエストです。 */
	if(len <= 0) {
		return 0;
	}

	/* スタートビット(0)が来るまで待つ。 */
	i = MMC_RECV_TIMEOUT;
	for(;;) {

		/* クロックをLowに。(P06をLowに) */
		*p0d = lo1;
		WAIT1; /* ウェイト1回以上必要 */

		/* クロックをHighに。(P06をHighに) */
		*p0d = hi1;
		WAIT5; /* ウェイト5回以上必要 */

		/* データを取得(P04からデータを取得)。 */
		if(!(*p0d & 0x10)) {
			break; /* スタートビット(0)を検出した */
		}

		/* タイムアウトは0を返す。 */
		if(!--i) {
			return 0;
		}
	}

	/* R1 or R2レスポンス受信か？ */
	if(len <= 2) {
		/* 1ビット受信済みに。 */
		v = 0;
		goto R1_R2;
	}

	/* 続きのデータを受信。 */
	do {
		/* 1バイト=8ビットのデータを受信します。
		 * なぜかここだけはクロックLow、Hiの後のウェイト無しでも正しい値が受信できます。
		 * いったんデータ転送が始まったら、MMC内部の複雑な処理が無くなるからでしょうか？
		 */
		v = 0;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x80;
R1_R2:		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x40;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x20;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x10;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x08;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x04;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x02;
		*p0d = lo1; *p0d = hi1; if(*p0d & 0x10) v |= 0x01;
		*buf++ = v;

	} while(--len);

	return 1;
}

//---------------------------------------------------------------------------

int
mmcSendData(const unsigned char *paSendBuff,int iSendLen)
{
	//volatile unsigned char* const pdwait = &PDWAIT;
	volatile unsigned char* const p0d = &P0D;
	//
	const int lo0 = *p0d & ~0x60;
	const int lo1 = lo0 | 0x20;
	const int hi0 = lo0 | 0x40;
	const int hi1 = lo0 | 0x60;
	//
	int data;

	if(iSendLen <= 0) {
		return 0;
	}

	do {
		data = *paSendBuff++;

		/* データ連続転送中はウェイト無しで大丈夫みたいです。
		 * その代わり、ときどきBusyクリアが遅くなるみたいです。
		 * たぶんそのとき、フラッシュメモリの書き換えが行われているのだと思います。
		 * タイムアウトは少し長めに取ってください。
		 */
		if(data & 0x80) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x40) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x20) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x10) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x08) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x04) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x02) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }
		if(data & 0x01) { *p0d = lo1; *p0d = hi1; } else { *p0d = lo0; *p0d = hi0; }

	} while(--iSendLen);

	return 1;
}
//---------------------------------------------------------------------------
void mmcSendDataCore(unsigned char *paData,int iDataLen)
{

	//データの送信コア
	
	//ダミークロックを送らないとMMC側で色々処理を
	//してくれなくなるので注意
	int i;
	
	//8クロック分のダミー送信
	mmcByteSend(0xFF);
	
	//指定のバイト数データを送信
	for(i = 0;i < iDataLen;++i)
	{
		//１バイト送信
		mmcByteSend(*paData++);
	}

	//8クロック分のダミー送信
	mmcByteSend(0xFF);

}
//---------------------------------------------------------------------------
unsigned short mmcSendCommandAndRecvResponse(unsigned char ucCMD,unsigned long ulArg)
{

	//コマンド送信＆レスポンス受信
	//CMD13だけR2レスポンス(2bytes)
	unsigned short usRes = 0xFFFF;		//レスポンス
	unsigned char ucWork;				//一時記憶
	unsigned char ucaCommand[6];		//コマンドデータ
	//int i;

	//コマンドデータの作成

	//コマンド部セット
	ucaCommand[0] = (0x40 | ucCMD);
	
	//引数部セット
	//for(i = 4;i > 0;--i)
	//{
	//	ucaCommand[i] = (unsigned char)(ulArg & 0xFF);
	//	ulArg >>= 8;
	//}
	//2005/02/09 Change by Madoka 少しでも高速化させるため、ベタで書きます
	{	
		ucaCommand[4] = (unsigned char)(ulArg & 0xFF);
		ulArg >>= 8;
		ucaCommand[3] = (unsigned char)(ulArg & 0xFF);
		ulArg >>= 8;
		ucaCommand[2] = (unsigned char)(ulArg & 0xFF);
		ulArg >>= 8;
		ucaCommand[1] = (unsigned char)(ulArg       );
	}	

	//CRC部セット
	ucaCommand[5] = 0xFF;	//ダミー

	//コマンド送信
	mmcSendDataCore(ucaCommand,6);

	//レスポンスを受信
	if(mmcRecvData(&ucWork,1) == 1)
	{
		//レスポンスを取得
		usRes = ucWork;

		//送ったのはCMD13か？
		if(ucCMD == 13)
		{
			//さっきのを上位にずらす
			usRes <<= 8;

			//残りの1バイトを受信
			if(mmcRecvData(&ucWork,1) == 1)
			{
				
				usRes |= (unsigned short)ucWork;

			}
			else usRes = 0xFFFF;	//失敗
		}
	}

	return usRes;

}
//---------------------------------------------------------------------------
