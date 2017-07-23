/*
 *	pceth2 - SE、BGM関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/07	開発開始
 *	2005/05/27	Win側pceth2bin2に対応（スクリプト形式を多少変更
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"
#include "muslib2.h"

#include "common.h"
#include "pceth2_snd.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

extern SAVE_DATA play;

//=============================================================================
//	PCEWAVEINFOを作成、再生、リピート再生、解放
//=============================================================================
/*
typedef struct tagPCEWAVEINFO {
	volatile unsigned char stat;					// 0  ステータス
	unsigned char type;								// 1  データ形式
	unsigned short resv;							// 2  予約
	const void *pData;								// 4  データへのポインタ
	unsigned long len;								// 8  データの長さ(サンプル数)
	struct tagPCEWAVEINFO *next;					// 12 次へのポインタ
	void (*pfEndProc)( struct tagPCEWAVEINFO *);	// 16 終了時コールバック←これを使ってリピート再生、解放を行います。
} PCEWAVEINFO;
*/

static PCEWAVEINFO	pwav;
static BYTE			*ppd;

/*
 *	リピート再生用コールバック
 *
 *	*pWav	コールバック呼び出し元PCEWACEINFOのポインタ
 */
static void Play_PieceWave(PCEWAVEINFO *pWav)
{
	pceWaveDataOut(SND_CH, pWav);
}

/*
 *	解放用コールバック
 *
 *	*pWav	コールバック呼び出し元PCEWACEINFOのポインタ
 */
static void Stop_PieceWave(PCEWAVEINFO *pWav)
{
	pceWaveAbort(SND_CH);
	pceHeapFree(ppd);
	ppd = NULL;
}

/*
 *	PCEWAVEINFO作成拡張（pfEndProcでリピート再生）
 *
 *	*pWav	PCEWAVEDATA構造体のポインタ
 *	*data	メモリ上のppdファイルデータ
 *	rep		リピートフラグ（0=なし／0以外＝リピート）
 */
static void Get_PieceWaveEx(PCEWAVEINFO *pWav, BYTE *data, const int rep)
{
	PceWaveInfo_Construct(pWav, data);
	if (rep) {	// リピート
		pWav->pfEndProc = Play_PieceWave;
	} else {	// 再生終了
		pWav->pfEndProc = Stop_PieceWave;
	}
}

//=============================================================================
//	SE再生（ファイルがなければ停止）
//=============================================================================

/*
 *	SEを読み込む	SE_0000.ppd,0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
#define FNAMELEN_SE		11

int pceth2_loadSE(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_SE + 1];
	int rep;

	Stop_PieceWave(&pwav);	// 前の再生が有る無し問わず止める

	// ファイル名をバッファにコピー
	pceth2_strcpy(buf, s, FNAMELEN_SE);
	s->p++;	// ,
	rep = (int)(*(s->data + s->p++) - '0');	// リピートフラグ

	ppd = fpk_getEntryData(buf, NULL, NULL);
	if (ppd != NULL) {
		Get_PieceWaveEx(&pwav, ppd, rep);
		Play_PieceWave(&pwav);
	}

	return 1;
}

//=============================================================================
//	pmd再生、停止
//=============================================================================

// ヒープのフラグメンテーションを回避するため、固定で確保
// このサイズを超えることは無いと思うが、超えたら増やすこと
// （v1.04 時点、最大は M35.pmd の 3,998）
static BYTE	pmd[4096];

/*
 *	BGM再生
 *
 *	*fName	pmdファイル名
 */
void Play_PieceMML(const char *fName)
{
	if (strcmp(play.pmdname, fName) != 0) {
		Stop_PieceMML();
		strcpy(play.pmdname, fName);
		if (fpk_getEntryData(play.pmdname, NULL, pmd)) {
			PlayMusic(pmd);
		} else {
			*play.pmdname = '\0';
		}
	}
}

/*
 *	BGM停止
 */
void Stop_PieceMML()
{
	*play.pmdname = '\0';
	StopMusic();
	pceWaveAbort(BGM_CH);
}

//=============================================================================
//	BGM再生（ファイルがなければ停止）
//=============================================================================

/*
 *	BGMを再生	M00.pmd
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
#define FNAMELEN_BGM	7

int pceth2_loadBGM(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_BGM + 1];
	pceth2_strcpy(buf, s, FNAMELEN_BGM);
    Play_PieceMML(buf);

	return 1;
}
