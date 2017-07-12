/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/

#include <piece.h>
#include <muslib.h>
#include <string.h>
#include "../libfpk.h"

// 仮想画面
unsigned char g_vScreen[128 * 88];
// ダーティフラグ
unsigned char g_fDraw;

// par から展開した pmd データが入る
unsigned char *g_pSeq = NULL;
// par ファイルハンドル
HFPK g_hFpk = NULL;
// 再生中のファイルエントリと、選択中のファイルエントリ
FPKENTRY g_fpkPlaying, g_fpkSelection;
// 選択中 par ファイルインデックスと、ホールドフラグ
int g_nMusicIndex, g_fHold;

// 今聞いている曲名を表示
void ShowNowPlaying();
// 安全かつ直ちに曲を止める
void musStopImmediately();
// 今選択中の曲を表示
void PrintSelection();
// 未使用：自力で pmd からタイトルを取ってくる
void ReadTitle(unsigned char* pSeq, char* szTitle, int nTitleSize,
                                    char* szTitle2, int nTitle2Size);
// par 用 PlayMusic()
void fpkPlayMusic(int nIndex);

// 意味は VB とか BASIC と同じ :)
void Cls()
{
	memset(g_vScreen, 0, 128 * 88);
	pceFontSetPos(0, 0);
	g_fDraw = 1;
}

// 意味は VB と同じ :)
void Refresh()
{
	if(g_fDraw)
	{
		pceLCDTrans();
		g_fDraw = 0;
	}
}

void pceAppInit()
{
	pceLCDDispStop();
	pceLCDSetBuffer(g_vScreen);
	pceAppSetProcPeriod(50);

	InitMusic();
	Cls();

	g_nMusicIndex = 0;
	g_fHold = 0;

	if((g_hFpk = fpkOpenArchive("mypmds.par")))
	{
		// par の最初のファイルをとってくる
		fpkGetFileInfoN(g_hFpk, g_nMusicIndex, &g_fpkPlaying);
		g_fpkSelection = g_fpkPlaying;

		fpkPlayMusic(g_nMusicIndex);

		pceLCDDispStart();
	}
}

void pceAppProc(int cnt)
{
	// par を開き損なっていたら強制終了
	if(!g_hFpk) pceAppReqExit(-1);

	// START ボタン
	if(pcePadGet() & TRG_C)
	{
		if(!g_fHold)
			pceLCDDispStop();
		else
			pceLCDDispStart();
		g_fHold ^= 1;
	}
	// ホールド状態でなければ
	if(!g_fHold)
	{
		if(pcePadGet() & TRG_A)
		{
			musStopImmediately();
			fpkPlayMusic(g_nMusicIndex);
		}
		if(pcePadGet() & TRG_B)
		{
			musStopImmediately();
			Cls();
			PrintSelection();
		}
		if(pcePadGet() & TRG_LF)
		{
			if(g_nMusicIndex > 0)	g_nMusicIndex--;
			fpkGetFileInfoN(g_hFpk, g_nMusicIndex, &g_fpkSelection);
			PrintSelection();
		}
		if(pcePadGet() & TRG_RI)
		{
			if(g_nMusicIndex < g_hFpk->fpkHeader.dwFilesAmount - 1)	g_nMusicIndex++;
			fpkGetFileInfoN(g_hFpk, g_nMusicIndex, &g_fpkSelection);
			PrintSelection();
		}
	}

	pceAppActiveResponse(MusicCheck() ? AAR_NOACTIVE : AAR_ACTIVE);

	Refresh();
}

void pceAppExit()
{
	musStopImmediately();
	fpkCloseArchive(g_hFpk);
}

void PrintSelection()
{
	pceFontSetPos(0, 0);	pceFontPrintf("seek: %-16s", g_fpkSelection.szFileName);
	g_fDraw = 1;
}

void musStopImmediately()
{
	// まずドライバを止める
	StopMusic();
	// 再生待ちバッファがなくなるのを待つ
	while(pceWaveCheckBuffs(music_wch));
	// カーネルに停止要求を出す
	pceWaveAbort(music_wch);

	// シーケンスの解放
	if(g_pSeq)
	{
		pceHeapFree(g_pSeq);
		g_pSeq = NULL;
	}
}

void ShowNowPlaying()
{
	Cls();

	pceLCDLine(3, 0, 10, 127, 10);
	pceFontSetPos(0, 12); pceFontPrintf("playing: %-16s", g_fpkPlaying.szFileName);
	pceLCDLine(3, 0, 22, 127, 22);
	pceFontSetPos(0, 24); pceFontPrintf(title);
	pceLCDLine(3, 0, 46, 127, 46);
	pceFontSetPos(0, 48); pceFontPrintf(title2);

	PrintSelection();

	g_fDraw = 1;
}

// par 用 PlayMusic()
void fpkPlayMusic(int nIndex)
{
	musStopImmediately();
	fpkGetFileInfoN(g_hFpk, nIndex, &g_fpkPlaying);
	if(strcmp(strrchr(g_fpkPlaying.szFileName, '.'), ".pmd") == 0)
	{
		g_pSeq = fpkExtractToBuffer(g_hFpk, &g_fpkPlaying);
		PlayMusic(g_pSeq);
		ShowNowPlaying();
	}
}

#define AS_WORD(p) ((WORD)*((WORD*)p))

// 未使用：自力で pmd からタイトルを取ってくる
void ReadTitle(unsigned char* pSeq, char* szTitle, int nTitleSize,
                                    char* szTitle2, int nTitle2Size)
{
	BYTE* p = pSeq;

	// db 0
	if(!*p) p++;

  // partn X
	p += *p++ << 1;

	p += 2;

	if(AS_WORD(p))
		strncpy(szTitle, pSeq + AS_WORD(p), nTitleSize);
	p += 2;
	if(AS_WORD(p))
		strncpy(szTitle2, pSeq + AS_WORD(p), nTitle2Size);
}
