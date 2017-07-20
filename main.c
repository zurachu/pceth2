/*
 *	pceth2
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/02/16	好き好きタマお姉ちゃんVer.
 *	2005/03/11	画像は外部ファイル（par形式）から読み込むように
 *	2005/04/09	スクリプト読み込み達成
 *	2005/04/20	特殊文字の処理済み、ハート追加
 *	2005/04/21	禁則処理（、。）追加、w3桁に
 *	2005/04/23	画像データポインタをヒープ解放後に確実にNULLに
 *	2005/04/25	<S>、<W>に対応、\k後でない\nを無視
 *				w3桁にちゃんと対応していなかったのを修正
 *				名前置換に*nnk1を追加
 *	2005/04/27	pceth2_grp.cに分離
 *				ラベルジャンプ（条件なし）を追加
 *	2005/04/30	フラグロード、ラベルブランチ、スクリプトジャンプを追加
 *	2005/05/01	フラグセーブを追加
 *				禁則処理に」』を追加
 *				pceth2_sys.cに分離
 *	2005/05/07	pceth2_snd.cを追加
 *	2005/05/08	pceth2_msg.cに分離
 *	2005/06/11	カレンダーモードの時はBボタンでVBuffが消えないように修正
 *	2005/06/12	B＋上下左右でコントラスト、音量調節可能に
 *	2005/06/13	デバッグ用ビルド追加
 *	2005/06/15	一行の文字数の違いによる改行の補正
 *	2005/06/25	名前置換処理をコンバータ側に移動
 *	2005/06/30	コントラスト、音量調節の操作を変更（ウィンドウ消してる状態で上下左右）
 *	2005/07/19	calFlag廃止、pgxname[GRP_C]で判断するように
 *				スクリプト中のカレンダーモードに対応
 *				BG表示命令で桜を日付に合わせないケース（回想など）に対応
 *				 *
 *	TODO		スクリプトの完全解析
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"
#include "usbcapt.h"
#include "muslib2.h"

#include "ufe/ufe.h" /*{{2005/06/09 Naoyuki Sawa}}*/
#include "mmc_api.h" //2005/06/11 Added by Madoka

#include "common.h"
#include "pceth2_arc.h"
#include "pceth2_sys.h"
#include "pceth2_msg.h"
#include "pceth2_sel.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_cal.h"
#include "pceth2_sav.h"

BOOL file_load = FALSE;		// ファイルを開けたかどうか

BOOL debug_mode = FALSE;
int speed, wait, msgView;
static int bButtonMenuTime = 0;

static PrecisionTimer s_frame_timer;
static unsigned long s_frame_us, s_proc_us;

static void pceth2_initGraphicAndSound();
static int  pceth2_readScript(SCRIPT_DATA *s);
static void pceth2_waitKey();
static void pceth2_startDebugMenu();

//=============================================================================
//=============================================================================

/*
 *	初期化
 */
#define ARCHIVE_FILE_NAME	"pceth2.par"	// アーカイブファイル名
#define DEBUG_FILE_NAME		"999999999.scp"	// デバッグメニュースクリプト
#define PROC_PERIOD	33	// Proc/msec

void pceAppInit(void)
{	
	FramObject_Init();

	if(pcePadGetDirect() & PAD_C) {
		debug_mode = TRUE;
	}

	/*{{2005/06/09 Naoyuki Sawa*/
	if(ufe_setup() != 0)	// UFE初期化
	{
		//2005/06/11 Added by Madoka
		if(mmcInit(MMC_FILESIZE_MAX) != 1) {	// MMC初期化
			return;
		}
	}
	/*}}2005/06/09 Naoyuki Sawa*/

	usbCaptureInit();	// pceCaps初期化
	if(!Ldirect_Init())
	{
		return;
	}
	Ldirect_VBuffView(TRUE);
	pceLCDDispStop();

	FontProxy_Hook_Set();
	FontExtend_Hook_GetAdrs();	// 特殊フォント追加pceFontGetAdrsをフック
	FontFuchi_SetType(0);
	FontFuchi_SetRange(MSG_X_MIN, MSG_Y_MIN, MSG_X_MAX, MSG_Y_MAX);
	FontFuchi_SetTxColor(0);
	FontFuchi_SetBdColor(3);
	loadInst();			// ドラム音色分離キット初期化
	InitMusic();		// 音楽ライブラリ初期化


	pceAppSetProcPeriod(PROC_PERIOD);

	if (pceth2_readGlobalSaveData()) {

		// 実行前のコントラスト、音量を保存
		Configure_Init();

		// アーカイブ読み込み
		file_load = fpk_InitHandle(ARCHIVE_FILE_NAME);
		if (file_load) {
			if(debug_mode) {
				pceth2_Init();
				pceth2_startDebugMenu();
			} else {
				pceth2_TitleInit();
			}
		}
	}

	msgView = 1;
	speed = 0;

	pceLCDDispStart();
	PrecisionTimer_Construct(&s_frame_timer);
}

/*
 *	メイン
 */
void pceAppProc(int cnt)
{
	PrecisionTimer proc_timer;
	PrecisionTimer_Construct(&proc_timer);

	/*{{2005/06/09 Naoyuki Sawa*/
//	if(!hFpk) { //初期化失敗?
	if (!file_load) {	// 2005/07/23変更
		pceAppReqExit(0);
		return;
	}
	/*}}2005/06/09 Naoyuki Sawa*/

	switch (play.gameMode)
	{
        case GM_TITLE:		// タイトル画面
			pceth2_Title();
			break;
		case GM_SAVE:	// セーブロードメニュー
			pceth2_SaveMenu();
			break;
		case GM_EVSCRIPT:	// EV_～スクリプト読み込み
			while (pceth2_readScript(&play.evData));
			break;
		case GM_SCRIPT:		// スクリプト読み込み
			while (pceth2_readScript(&play.scData));
			break;
		case GM_SELECT:
			pceth2_Select();
			if (pcePadGet() & PAD_C) { pceth2_SaveInit(); }
			break;
		case GM_MAPSELECT:	// マップ選択
			pceth2_MapSelect();
			if (pcePadGet() & PAD_C) { pceth2_SaveInit(); }
			break;
		case GM_CALENDER:
			pceth2_calenderDrawCircle();
			break;
		case GM_KEYWAIT:	// キー待ち
			pceth2_waitKey();
			if (pcePadGet() & PAD_C && !pceth2_isCalenderMode()) { pceth2_SaveInit(); }
			break;
		case GM_TIMEWAIT:	// 時間待ち
			if (wait-- <= 0 || (pcePadGet() & PAD_RI)) {
				play.gameMode = GM_SCRIPT;
			}
			break;
		case GM_MAPCLOCK:	// マップ前時計（時間待ち）
			if (wait-- <= 0 || (pcePadGet() & PAD_RI)) {
				pceth2_initMapSelect();
			}
			break;
		case GM_SLIDECHR:	// 立ち絵スライド
			pceth2_slideChara();
			break;
	}

	if (pcePadGet() & PAD_D) {
		if(debug_mode) {
			if(!strncmp(play.scData.name, DEBUG_FILE_NAME, 6)) { // デバッグメニュースクリプト中
				pceAppReqExit(0);
			} else {
				pceth2_startDebugMenu();
			}
		} else {
			if (play.gameMode == GM_TITLE) {
				pceAppReqExit(0);
			} else {
				pceth2_TitleInit();
			}
		}
	}

	if(debug_mode) {
		pceLCDPaint(0, 0, 82, DISP_X, 6);
		pceFontSetType(2);
		pceFontSetPos(0, 82);
		pceFontSetTxColor(3);
		pceFontSetBkColor(FC_SPRITE);
		pceFontPrintf("%6lu/%6luus FREE:%8d", s_proc_us, s_frame_us, pceHeapGetMaxFreeSize());
		Ldirect_Update();
	}

	Ldirect_Trans();

	s_frame_us = PrecisionTimer_Count(&s_frame_timer);
	s_proc_us = PrecisionTimer_Count(&proc_timer);
}

/*
 *	終了
 */
void pceAppExit(void)
{
	StopMusic();
	pceWaveStop(0);

	/*** 読み込んだファイルを明示的に解放してないが問題なし？ ***/

	// グローバルデータ（フラグ、コントラスト、音量）を保存
	pceth2_writeGlobalSaveData();

	// 実行前のコントラスト、音量に戻す
	Configure_Exit();

	fpk_ReleaseHandle();
	FontExtend_Unhook_GetAdrs();	// pceFontGetAdrsを元に戻す
	FontProxy_Unhook_Set();
	Ldirect_Exit();
	usbCaptureRelease();	// pceCaps解放

	//2005/06/11 Added by Madoka
	mmcExit();

	/*{{2005/06/09 Naoyuki Sawa*/
	ufe_stop();
	/*}}2005/06/09 Naoyuki Sawa*/
}

//2005/06/11 Added by Madoka
/*
 *	システム通知
 */
int pceAppNotify(int type, int param)
{	
	
	//MMC対応カーネルVer.1.27以降での処理
	//カーネル側でのMMC初期化を無効にする
	//こうしないと、大きいファイルが扱えないため
	if(type == APPNF_INITMMC)
	{
		return APPNR_REJECT;
	}

	return APPNR_IGNORE;	//デフォルトの処理
}

//=============================================================================
//	
//=============================================================================

/*
 *	はじめから
 */
void pceth2_Init()
{
	memset(play, 0, sizeof(SAVE_DATA));

	MONTH	= START_MONTH;	// 月
	DAY		= START_DAY;	// 日
	TIME	= EV_MORNING;	// 時間
	// クリアフラグをグローバルと同期する
	memcpy(&play.flag[80], &global.flag, GLOBAL_FLAG_NUM * sizeof(unsigned short));

	memset(reg, 0, REG_NUM);	// レジスタ

	pceth2_initGraphicAndSound();

	pceth2_loadEVScript(&play.evData);

//	play.gameMode = GM_EVSCRIPT;
}

void pceth2_initGraphicAndSound()
{
	int i;

	pceth2_setPageTop();
	pceth2_clearMessage();

	msgView = 1;
	speed = 0;

	for (i = 0; i <= GRP_NUM; i++) {
		pceth2_clearGraphic(i);
	}
	pceth2_DrawGraphic();
	BG_TIME = BG_WEATHER = '0';	// 背景画像ファイル名修飾子

	Stop_PieceMML();

	Ldirect_Update();
}

/*
 *	キー待ち
 */
void pceth2_waitKey()
{
	if (msgView)	// メッセージ表示状態
	{
		if (pcePadGet() & (TRG_A | PAD_RI)) {	// スクリプトを進める
			if (pceth2_isPageTop()) {
				pceth2_clearMessage();
				Ldirect_Update();
			}
			if (pceth2_isCalenderMode()) {	// カレンダーモード時
				pceth2_clearGraphic(GRP_C);	// カレンダー画像消去
			}
			play.gameMode = GM_SCRIPT;
		} else if (pcePadGet() & TRG_B) {
			if (!pceth2_isCalenderMode()) {	// カレンダーの時は消せない
				pceth2_drawBButtonMenu();
			}
		}
	}
	else			// メッセージ非表示状態
	{
		pceth2_bButtonMenu();
	}
}

void pceth2_bButtonMenu()
{
	if(bButtonMenuTime > 0) {
		if(--bButtonMenuTime == 0) {
			Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
		}
	}
	if (pcePadGet() & (TRG_A | TRG_B)) {
		pceth2_comeBackMessage();
	}
	// ＋上下左右でコントラスト、音量の調節
	if (pcePadGet() & TRG_LF) {
		if(global.bright > 0) {
			pceLCDSetBright(--global.bright);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_RI) {
		if(global.bright < 63) {
			pceLCDSetBright(++global.bright);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_DN) {
		if(global.masteratt < 127) {
			pceWaveSetMasterAtt(++global.masteratt);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_UP) {
		if(global.masteratt > 0) {
			pceWaveSetMasterAtt(--global.masteratt);
		}
		pceth2_drawBButtonMenu();
	}
}

void pceth2_drawBButtonMenu()
{
	msgView = 0;
	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
	pceLCDPaint(0, 0, 0, DISP_X, 6);
	pceFontSetType(2);
	pceFontSetPos(0, 0);
	pceFontSetTxColor(3);
	pceFontSetBkColor(FC_SPRITE);
	pceFontPrintf("<>BRIGHT:%2d ^VOL:%3d", global.bright, 127 - global.masteratt);
	pceFontPut(48, 1, 'v');
	Ldirect_Update();
	bButtonMenuTime = 3 * 1000 / PROC_PERIOD;
}

/*
 *	スクリプトを読む
 *	return	1の間pceAppProc()から繰り返して呼び出される
 */
int pceth2_readScript(SCRIPT_DATA *s)
{
	/* 命令解析テーブル */
	static const struct {
		char com;
		int  (*func)(SCRIPT_DATA *);
	} com_table[] =	   {{'B',  pceth2_loadBG},		// 背景画像（B6.pgx＝11文字＋立ち絵消去フラグ1文字）
						{'G',  pceth2_setBGOption},	// 背景画像のオプション指定
						{'V',  pceth2_loadBG},		// イベント画像（V6.pgx＝11文字＋立ち絵消去フラグ1文字）
						{'C',  pceth2_loadChara},	// 立ち絵画像（C8.pgx＝13文字＋位置1文字＋更新時期フラグ1文字）
						{'c',  pceth2_clearChara},	// 立ち絵消去（c3文字＋更新時期フラグ1文字）
						{'S',  pceth2_loadSE},		// SE再生（SE_4.ppd＝11文字＋リピートフラグ1文字）
						{'M',  pceth2_loadBGM},		// BGM演奏（M2.pmd＝7文字）【win側未実装】
						{'w',  pceth2_wait},		// 時間ウェイト（w3文字）
						{'m',  pceth2_addMapItem},	// マップ選択肢を追加（9.scp＝13文字＋場所2文字＋チップキャラ2文字）
						{'J',  pceth2_jumpScript},	// スクリプトジャンプ（9.scp＝13文字）
						{'j',  pceth2_jumpLabel},	// ラベルジャンプ
						{'b',  pceth2_branchLabel},	// 条件付きラベルジャンプ
						{'l',  pceth2_loadFlag},	// フラグをレジスタにロード
						{'s',  pceth2_saveFlag},	// フラグを書き換え
						{'=', pceth2_setReg},		// レジスタに値をセット
						{'+', pceth2_incReg},		// レジスタをインクリメント
						{'-', pceth2_decReg},		// レジスタをデクリメント
						{'q',  pceth2_addSelItem},	// 選択肢を追加
						{'Q',  pceth2_initSelect},	// 選択
						{'@',  pceth2_skipLabel},	// ラベル（読み飛ばし）
						{'<',  pceth2_procControl},	// メッセージ制御
						{'\\', pceth2_procEscape},	// エスケープシーケンス処理
						{'D', pceth2_calenderInitEx},	// スクリプトからカレンダーモードに移行
						{'z', pceth2_goEpilogue},	// エピローグへ
						{'Z', pceth2_backTitle},	// タイトルに戻る
	};
	int i;

	// 最後まで読んだら終了
	if (s->p >= s->size) {
		if(debug_mode) {	// デバッグモードの場合デバッグメニューに戻る
			pceth2_startDebugMenu();
		} else {
			switch(play.gameMode)
			{
				case GM_EVSCRIPT:
					JUMP = 0;	// 2005/06/20 まるしすさん案：1e, 4, 1が来たらgotoの初期化
					if (TIME == EV_MAP_SELECT) {
						pceth2_initMapClock();
						TIME++;
					} else if (TIME > EV_NIGHT) {	// 一日終了
						play.lmAmount = 0;	// マップ選択肢があるがマップ選択に行かなかった場合、ここで初期化
						TIME = EV_MORNING;
						DAY++;
						pceth2_calenderInit();	// カレンダー
					} else {
						pceth2_loadEVScript();	// 次のEVスクリプトを読む
					}
	//				if (play.evData.size == 0) {	// 読めなかったら終了
	//					pceAppReqExit(0);
	//				}
					break;
				case GM_SCRIPT:
					pceth2_closeScript(&play.scData);
					play.gameMode = GM_EVSCRIPT;
					break;
			}
		}
		return 0;
	}


	// 命令解析
	for (i = 0; i < array_size(com_table); i++) {
		if (*(s->data + s->p) == com_table[i].com) {
			return com_table[i].func(s);
		}
	}

	// 残りは画面表示文字だけのはずですよ
	if (pceth2_jpnHyphenation(s->data + s->p + 2) || pceth2_lineFeed(s->data + s->p)) {
		pceth2_putCR();
		if (pceth2_isPageTop()) {
			play.gameMode = GM_KEYWAIT;
			goto UPDATE;
		}
	}
	// 連続空白は一つしか表示しない（これで手動センタリングを回避できる？）
	if (strncmp(play.msg + play.msglen - 2, "　", 2) || strncmp(s->data + s->p, "　", 2)) {
		pceth2_putKanji(s->data + s->p);
	}
	s->p += 2;
	if (pceth2_isPageTop()) {
		play.gameMode = GM_KEYWAIT;
		goto UPDATE;
	}

	if (pcePadGet() & PAD_RI) {	// →を押していればreadScriptを再呼び出し（スキップ表示）
		return 1;
	}

UPDATE:
	Ldirect_Update();
	return 0;
}

void pceth2_startDebugMenu()
{
	pceth2_initGraphicAndSound();
	pceth2_loadScript(&play.scData, DEBUG_FILE_NAME);
	play.gameMode = GM_SCRIPT;
}
